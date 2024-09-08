#include "parser.h"
#include "arena.h"
#include "common.h"
#include "lexer.h"

static const LObject_Type keywords_to_objtypes[] = {
    OBJ_I8, OBJ_U8, OBJ_CH, OBJ_I16,
    OBJ_U16, OBJ_I32, OBJ_U32, OBJ_F32,
    OBJ_I64, OBJ_U64, OBJ_F64
};

void lmap_insert(Label_Map *map, Label l)
{
    l.idx = ++map->index;
    l.hash = hash_string(l.name.data, l.name.count);
    table_insert(map, l, Label_Map);
}

Label lmap_search(Label_Map *map, String_View name)
{
    Label l = {0};
    l.hash = hash_string(name.data, name.count);
    table_get(map, l);
    return l;
}

LUNA_FUNC Expr *expr_init(Arena *a)
{
    Expr *e = arena_alloc(a, sizeof(Expr));
    e->t = EXPR_NONE;
    return e;
}

LUNA_FUNC void opcode_search(Table *opcodes, IOpcode *dst, String_View name)
{
    Table_Item item = {0};
    item.hash = hash_string(name.data, name.count);
    table_get(opcodes, item);
    *dst = item.value;
}

LUNA_FUNC void parse_register(Expr *e, Token *tk, u8 *status)
{
    if (tk->text.data[0] != 'r') {
        luna_excp(EXCP_PARSING, &tk->loc, "text "SV_Fmt" not a register", SV_Args(tk->text));
        *status = LUNA_STATUS_ERR;
        return;
    }
    sv_cut_left(&tk->text, 1);
    int num = sv_to_int(tk->text);
    if (num > 9) {
        luna_excp(EXCP_PARSING, &tk->loc, "No register with number %d", num);
        *status = LUNA_STATUS_ERR;
        return;
    }
    e->v.u = num;
}

LUNA_FUNC size_t parse_typesize(Lexer *lex, u8 *status)
{
    Token tk = lexer_next(lex);
    if (tk.type >= TK_I8 && tk.type <= TK_F64) {
        size_t type_size = type_sizes[tk.type - TK_I8];
        lexer_yield(lex, TK_COLON); /* Skip colon */
        tk = lexer_yield(lex, TK_NUMBER);
        if (sv_is_float(tk.text)) { /* Check for unsigned long */
            luna_excp(EXCP_PARSING, &tk.loc, "Cannot use float for type size");
            goto defer;
        }
        lexer_yield(lex, TK_CLOSE_BRACKET); /* Skip `]` */
        return sv_to_ulong(tk.text) * type_size;
    } else {
        luna_excp(EXCP_PARSING, &tk.loc, "Token "SV_Fmt" not a object type", SV_Args(tk.text));
    }
defer:
    *status = LUNA_STATUS_ERR;
    return 0;
}

LUNA_FUNC Expr *parse_expr(Arena *a, Luna *L, Lexer *lex)
{
    Expr *e = expr_init(a);
    Token tk = lexer_next(lex);
    switch (tk.type) {
        case TK_TEXT: {
            e->t = EXPR_REG; /* Must be a register */
            parse_register(e, &tk, &L->status);
            break;
        } case TK_STRING: {
            e->t = EXPR_STR;
            e->v.s = tk.text;
            break;
        } case TK_NUMBER: {
            if (sv_is_float(tk.text)) {
                e->t = EXPR_FLT;
                e->v.f = sv_to_flt(tk.text);
            } else {
                e->t = EXPR_INT;
                e->v.i = sv_to_long(tk.text);
            }
            break;
        } case TK_INVOKE: {
            e->t = EXPR_UINT;
            tk = lexer_yield(lex, TK_TEXT);
            Label l = lmap_search(&L->lmap, tk.text);
            if (l.idx != 0) {
                e->v = l.v;
            } else {
                struct backpatch b;
                b.name = tk.text;
                b.addr = L->ps;
                e->v.u = b.addr;
                arena_da_append(a, L->bp, b);
                L->status = LUNA_STATUS_DFR;
            }
            break;
        } case TK_OPEN_BRACKET: {
            e->t = EXPR_UINT;
            e->v.u = parse_typesize(lex, &L->status);
            break;
        } default: {
            luna_excp(EXCP_PARSING, &tk.loc, "Token `"SV_Fmt"` not expresion", SV_Args(tk.text));
            L->status = LUNA_STATUS_ERR;
        }
    }
    return e;
}

LUNA_FUNC void parse_instruction(Arena *a, Luna *L, Lexer *lex)
{
    Stmt_Inst s = {0};
    Token tk = lexer_next(lex); /* Intruction opcode */
    size_t line = tk.loc.row; /* Save current line of instruction */
    opcode_search(&L->opcodes, &s.opcode, tk.text);
    if (!s.opcode) { /* Token must be an opcode */
        luna_excp(EXCP_PARSING, &L->s->loc, "Unknown opcode '"SV_Fmt"'", SV_Args(tk.text));
        L->status = LUNA_STATUS_ERR;
        return;
    }
    tk = lexer_peek(lex);
    if (tk.loc.row != line) {
        /* Instruction without arguments */
        defer_status(LUNA_STATUS_OK);
    } else if (tk.type == TK_INVOKE) {
        /* Instructions without mode */
        s.mode = OBJ_U64; /* It needs for writing data */
        Expr *e = parse_expr(a, L, lex);
        if (luna_staterr(L)) return; /* If cannot parse expr */
        stmtarg_append(a, &s.args, e);
        defer_status(LUNA_STATUS_OK);
    } else if (tk.type < TK_I8 || tk.type > TK_F64) {
        /* Assert case when all higher cases was missed */
        luna_excp(EXCP_PARSING, &L->s->loc, "Unknown mode '"SV_Fmt"'", SV_Args(tk.text));
        L->status = LUNA_STATUS_ERR;
        return;
    }
    lexer_next(lex); /* Skip type*/
    s.mode = keywords_to_objtypes[tk.type - TK_I8];
    Expr *arg = parse_expr(a, L, lex);
    if (luna_staterr(L)) return; /* If cannot parse expr */
    stmtarg_append(a, &s.args, arg);
    if (lexer_peek(lex).type == TK_COMMA) {
        lexer_next(lex); /* Skip comma */
        Expr *arg = parse_expr(a, L, lex);
        if (luna_staterr(L)) return; /* Same thing */
        stmtarg_append(a, &s.args, arg);
    }
defer:
    L->s->v.inst = s;
}

LUNA_FUNC void parse_label(Arena *a, Luna *L, Lexer *lex)
{
    (void)a;
    Stmt_Label s = {0};
    lexer_next(lex); /* Skip label */
    if (lexer_peek(lex).type == TK_ENTRY) {
        lexer_next(lex); /* Skip entry */
        L->entry = L->ps;
    }
    Token tk = lexer_yield(lex, TK_TEXT);
    if (lex_staterr(lex)) { /* Not a text */
        defer_status(LUNA_STATUS_ERR);
    }
    s.l.loc = tk.loc; /* For debug information */
    s.l.name = tk.text;
    if (lexer_peek(lex).type == TK_COLON) {
        lexer_next(lex); /* Skip colon */
        s.l.v.u = L->ps; /* Address of next instruction */
        s.l.mode = LABEL_MODE_ADDR;
        defer_status(LUNA_STATUS_OK);
    } else {
        /* TODO: parse none address case */
        luna_assert(0 && "TODO: cannot parser this label case");
    }
defer:
    if (!luna_staterr(L)) {
        lmap_insert(&L->lmap, s.l);
        L->s->v.label = s;
    }
    return;
}

/* Parsing sequens of tokens */
void parse_statement(Arena *a, Luna *L, Lexer *lex)
{
    L->s->t = STMT_NONE;
    Token tk = lexer_peek(lex);
    L->s->loc = tk.loc;
    if (tk.type == TK_NONE) {
        lexer_next(lex);
        return;
    }
    switch (tk.type) {
        case TK_TEXT: {
            L->s->t = STMT_INST;
            parse_instruction(a, L, lex);
            break;
        } case TK_LABEL: {
            L->s->t = STMT_LABEL;
            parse_label(a, L, lex);
            break;
        } default: {
            luna_excp(EXCP_PARSING, &tk.loc, "TODO: not implemented parsing of token %u", tk.type);
            L->status = LUNA_STATUS_ERR;
            break;
        }
    }
}

#define iarg_type(s, num) (s).args.items[(num)]->t == EXPR_REG ? IARG_REG : IARG_VAL

void luna_write(Luna *L, LObject *o)
{
    size_t offset = type_sizes[o->t];
    if (L->ps + offset >= L->pc) {
        if (L->pc == 0) L->pc = LUNA_CODE_INIT_CAP;
        while (L->ps + offset >= L->pc) L->pc *= 2;
        L->code = realloc(L->code, L->pc);
    } 
    memcpy(L->code + L->ps, &o->v.as_u64, offset);
    L->ps += offset;
}

LUNA_FUNC LObject luna_object(Expr *e, u8 mode)
{
    LObject obj = {0};
    if (e->t == EXPR_REG) {
        obj.t = OBJ_U8;
        obj.v.as_u8 = (u8)e->v.u;
    } else {
        obj.t = mode;
        switch(obj.t) {
            case OBJ_I8:  obj.v.as_i8  = e->v.i; break;
            case OBJ_U8:  obj.v.as_u8  = e->v.u; break;
            case OBJ_I16: obj.v.as_i16 = e->v.i; break;
            case OBJ_U16: obj.v.as_u16 = e->v.u; break;
            case OBJ_I32: obj.v.as_i32 = e->v.i; break;
            case OBJ_U32: obj.v.as_u32 = e->v.u; break;
            case OBJ_F32: obj.v.as_f32 = e->v.f; break;
            case OBJ_I64: obj.v.as_i64 = e->v.i; break;
            case OBJ_U64: obj.v.as_u64 = e->v.u; break;
            case OBJ_F64: obj.v.as_f64 = e->v.f; break;
            default:      obj.t = OBJ_EMPTY; break;
        }
    }
    return obj;
}

LUNA_FUNC void statinst(Arena *a, Luna *L)
{
    (void)a;
    size_t argc = 0;
    LObject o[2] = {0};
    Instruction inst = 0;
    Stmt_Inst s = L->s->v.inst;
    inst = ipush_opcode(inst, s.opcode);
    inst = ipush_mode(inst, s.mode);
    while (argc < s.args.count) {
        o[argc] = luna_object(s.args.items[argc], s.mode);
        inst = ipush_arg(inst, iarg_type(s, argc), argc);
        argc++;
    }
    luna_write(L, &OBJECT_U16(inst));
    for (size_t i = 0; i < argc; ++i) {
        luna_write(L, &o[i]);
    }
}

LUNA_FUNC void statlabel(Arena *a, Luna *L)
{
    (void)a;
    Stmt_Label s = L->s->v.label;
    if (s.l.mode != LABEL_MODE_ADDR) {
        luna_assert(0 && "TODO: none addr mode not implemented");
    }
}

void luna_backpatching(Arena *a, Luna *L)
{
    (void)a;
    for (size_t i = 0; i < L->bp->count; ++i) {
        struct backpatch b = L->bp->items[i];
        Label l = lmap_search(&L->lmap, b.name);
        if (l.idx == 0 && l.mode != LABEL_MODE_ADDR) {
            /* Assert case when label is not exist and not an address */
            luna_excp(EXCP_TRANSLATE, &l.loc, "Cannot find/use label with name '"SV_Fmt"' for backpatching", SV_Args(b.name));
            L->status = LUNA_STATUS_ERR;
            return;
        }
        /* 
         * Shift by 2 bytes for rewriting data of jump-instruction
         * Next magic `8` number is size of address 
         */
        memcpy(L->code + b.addr + 2, &l.v.u, sizeof(u64));
    }
}
 
/* Modifie state of Luna by executing statement */
void luna_translate_stmt(Arena *a, Luna *L, Lexer *lex)
{
    parse_statement(a, L, lex);
    lexer_peek(lex); /* Check for empty */
    if (luna_staterr(L)) return;
    else if (lex_statempty(lex)) {
        L->status = LUNA_STATUS_OK;
        return;
    }
    switch (L->s->t) {
        case STMT_INST: {
            statinst(a, L);
            break;
        } case STMT_LABEL: {
            statlabel(a, L);
            break;
        } default: {
            luna_excp(EXCP_TRANSLATE, &L->s->loc, "Unknown statement %u to translating", L->s->t);
            L->status = LUNA_STATUS_ERR;
        }
    }
}

void expr_print(Expr *e)
{
    printf("    Arg ");
    switch(e->t) {
        case EXPR_INT: {
            luna_report("%li", e->v.i);
            break;
        } case EXPR_UINT: {
            luna_report("%lu", e->v.u);
            break;
        } case EXPR_REG: {
            luna_report("r%u", e->v.u);
            break;
        } default: {
            luna_report(" Unknown expr type %u", e->t);
            break;
        }
    }
}

void statement_print(Statement *s)
{
    if (s->t == STMT_NONE) return;
    luna_report("Statement(%zu,%zu)", s->loc.row, s->loc.col);
    switch (s->t) {
        case STMT_INST: {
            Stmt_Inst *inst = &s->v.inst;
            luna_report(" Instruction");
            luna_report("   Mode %zu", inst->mode);
            luna_report("   IOpcode %s", inst_names[inst->opcode]);
            for (size_t i = 0; i < inst->args.count; ++i)
                expr_print(inst->args.items[i]);
            break;
        } case STMT_LABEL: {
            Stmt_Label *l =  &s->v.label;
            luna_report(" Label");
            luna_report("   Name "SV_Fmt, SV_Args(l->l.name));
            luna_report("   Addr %zu", l->l.v.u);
            break;
        } default: {
            luna_report("  Unknown statement %zu", s->t);
            exit(27);
        }
    }
}
