#include "parser.h"

void lasm_push_obj(Lasm *L, Object obj)
{
    if (L->program_capacity == 0) {
        L->program_capacity = INIT_CAPACITY;
        L->program = arena_alloc(L->a, sizeof(*L->program) * L->program_capacity);
    }

    if (L->program_size + 1 > L->program_capacity) {
        size_t old_size = L->program_capacity * sizeof(*L->program);
        L->program_capacity *= 2;
        L->program = arena_realloc(L->a, L->program, old_size, L->program_capacity * sizeof(*L->program));
    }

    L->program[L->program_size++] = obj;
}

Statement parse_line(Arena *a, Line *line, Hash_Table *instT)
{
    Statement s = {0};
    s.line = line->line;
    s.location = line->location;
    switch (line->type) {
        case LINE_LABEL: s.t = STATE_LABLE; s.v.as_lable = labelstat(&line->item);          break;
        case LINE_ENTRY: s.t = STATE_ENTRY; s.v.as_entry = entrystat(&line->item);          break;
        case LINE_CONST: s.t = STATE_CONST; s.v.as_const = conststat(a, &line->item);       break;
        case LINE_INST:  s.t = STATE_INST;  s.v.as_inst  = inststat(a, &line->item, instT); break;
        default:
            pr_error(ERROR, "unknown line type `%u`", line->type);
    }
    return s;
}

LasmState *parser_primary(Lasm *L, Linizer *linizer)
{
    LasmState *ls = lasmstate_new(L->a);
    for (Line *line = linizer->head; line != NULL; line = line->next) {
        Statement s = parse_line(L->a, line, &L->instT);
        LasmBlock *lb = lasmblock_new(L->a, s);
        lasmstate_push(ls, lb);
    }
    return ls;
}

Label label_search(Label_List *ll, String_View name)
{
    for (size_t i = 0; i < ll->count; ++i) {
        Label label = ll->items[i];
        if (sv_cmp(label.name, name)) {
            return label;
        }
    }
    return (Label) {
        .addr = -1
    };
}

void parse_state_label(String_View name, Lasm *L)
{
    Label label = {
        .addr = L->program_size,
        .name = name
    };
    label_append(L->a, &L->jmps, label);
}

Object expr_value_as_obj(Expr expr, Const_Table *constT)
{
    Object obj = {0};
    Expr val = eval_expr(expr, constT);
    if (val.t == EXPR_LIT_FLT)
        obj = OBJ_FLOAT(val.v.as_flt);
    else if (val.t == EXPR_LIT_INT)
        obj = OBJ_INT(val.v.as_int);
    else /* TODO: Print better error info */
        pr_error(SYNTAX_ERR,
                 TOKEN_NONE,
                 "invalid type `%u` for pushing value",
                 val.t);
    return obj;
}

void parse_state_inst(StateInst s, Lasm *L)
{
    Inst inst = inst_defs[s.inst][s.kind];

    lasm_push_obj(L, OBJ_INST(inst));
    if (s.kind != KIND_NONE && s.kind != KIND_VAL) {
        lasm_push_obj(L, OBJ_REG(s.dst));
        switch (s.kind) {
            case KIND_REG:
                break;
            case KIND_REG_REG:
                Object reg = OBJ_REG(s.src.v.as_int);
                lasm_push_obj(L, reg);
                break;
            case KIND_REG_VAL:
                Object obj = expr_value_as_obj(s.src, &L->constT);
                lasm_push_obj(L, obj);
                break;
            default:
                pr_error(ERROR, "unknown case `%u` in `parse_state_inst`",
                         s.kind);
        }
    } else if (s.kind == KIND_VAL) {
        if (inst == INST_MOVS)
            lasm_push_obj(L, OBJ_REG(s.dst));

        Object obj = {0};
        if (inst_isjump(inst)) {
            if (s.src.t == EXPR_LIT_STR) {
                Label label = label_search(&L->jmps, s.src.v.as_str);
                if (label.addr == (Inst_Addr)(-1)) {
                    label.addr = L->program_size;
                    label_append(L->a, &L->defered_jmps, label);
                }
                obj = OBJ_UINT(label.addr);
            } else goto as_value;
        } else as_value: obj = expr_value_as_obj(s.src, &L->constT);
        lasm_push_obj(L, obj);
    }
}

void parse_state_entry(StateEntry s, Lasm *L)
{
    L->entry = L->program_size;
    parse_state_label(s.name, L);
}

void parse_state_const(StateConst s, Lasm *L)
{
    Expr value = eval_expr(s.value, &L->constT);
    size_t type = value.t == EXPR_LIT_FLT ? CONST_TYPE_FLOAT : CONST_TYPE_INT;
    Const_Value val = type == CONST_TYPE_INT ?
                      (Const_Value) { .as_i64 = value.v.as_int } :
                      (Const_Value) { .as_f64 = value.v.as_flt };
    Const_Statement *cnst = cnst_state_create(L->a, s.name, type, val);
    ct_insert(L->a, &L->constT, cnst);
}

void parser_secondary(LasmState *ls, Lasm *L)
{
    // first pass
    for (LasmBlock *b = ls->head; b != NULL; b = b->next) {
        Statement s = b->s;
        switch (s.t) {
            case STATE_INST:  parse_state_inst(s.v.as_inst, L);        break;
            case STATE_ENTRY: parse_state_entry(s.v.as_entry, L);      break;
            case STATE_CONST: parse_state_const(s.v.as_const, L);      break;
            case STATE_LABLE: parse_state_label(s.v.as_lable.name, L); break;
            default:
                pr_error(ERROR, "unknown case `%u` in parse_secondary", s.t);
        }
    }

    // second pass
    for (size_t i = 0; i < L->defered_jmps.count; ++i) {
        Label dlabel = L->defered_jmps.items[i];
        Label label = label_search(&L->jmps, dlabel.name);
        if (label.addr != (uint64_t)(-1))
            L->program[dlabel.addr] = OBJ_UINT(label.addr);
    }
}