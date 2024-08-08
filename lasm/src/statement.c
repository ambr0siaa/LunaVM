#include "statement.h"

LasmState *lasmstate_new(Arena *a)
{
    LasmState *ls = arena_alloc(a, sizeof(LasmState));
    ls->head = NULL;
    ls->tail = NULL;
    return ls;
}

LasmBlock *lasmblock_new(Arena *a, Statement s)
{
    LasmBlock *lb = arena_alloc(a, sizeof(LasmBlock));
    lb->next = NULL;
    lb->s = s;
    return lb;
}

void lasmstate_push(LasmState *ls, LasmBlock *lb)
{
    if (!ls->head && !ls->tail) {
        ls->head = lb;
        ls->tail = lb;
    } else {
        ls->tail->next = lb;
        ls->tail = lb;
    }
}

int try_register(String_View sv)
{
    if (sv.data[0] == 'r' || sv.data[0] == 'f') {
        if (isdigit(sv.data[1])) {
            return 1;
        } else {
            return 0;
        }
    } else if (sv_cmp(sv, sv_from_cstr("acc"))) {
        return 1;
    } else if (sv_cmp(sv, sv_from_cstr("accf"))) {
        return 1;
    } else {
        return 0;
    }
}

int parse_register(String_View sv)
{   
    char *acc = reg_as_cstr(ACC);
    if (sv_cmp(sv, sv_from_cstr(acc)))
        return ACC;

    char *accf = reg_as_cstr(ACCF);
    if (sv_cmp(sv, sv_from_cstr(accf)))
        return ACCF;

    int n = sv.data[1] - '0';

    if (sv.data[0] == 'r' && isdigit(sv.data[1])) {
        return n;

    } else if (sv.data[0] == 'f' && isdigit(sv.data[1])) {
        n = n + ACC + 1;
        return n;

    } else {
        return -1;
    }
}

StateInst inststat(Arena *a, Lexer *L, Hash_Table *instT)
{
    StateInst s = {0};
    Token tk = token_yield(L, TK_TEXT);

    String_View inst_name = tk.txt;
    char *key = sv_to_cstr(inst_name);
    int inst = inst_table_get(instT, key);

    if (inst < 0) {
        pr_error(SYNTAX_ERR, tk, "%s not an instruction", key);
    }

    free(key);
    s.inst = inst;

    tk = token_next(L);
    switch (tk.type) {
        case TK_NONE: {
            s.kind = KIND_NONE;
            break;
        }
        case TK_TEXT: {
            if (try_register(tk.txt)) {
                s.dst = parse_register(tk.txt);

                tk = token_next(L);
                if (tk.type != TK_COMMA) {
                    s.kind = KIND_REG;
                    break;
                }

                tk = token_next(L);
                if (tk.type == TK_NUMBER || tk.type == TK_AMPERSAND || tk.type == TK_OPEN_PAREN) {
                    token_back(L, 1);
                    s.kind = KIND_REG_VAL;
                    s.src = parse_expr(a, L);

                } else if (tk.type == TK_TEXT) {
                    s.kind = KIND_REG_REG;
                    s.src.t = EXPR_LIT_INT;
                    int reg = parse_register(tk.txt);

                    if (reg < 0) {
                        pr_error(SYNTAX_ERR, tk, "argument `"SV_Fmt"` not a register", SV_Args(tk.txt));
                    }

                    s.src.v.as_int = reg;

                } else if (tk.type == TK_DOLLAR) {
                    s.kind = KIND_VAL;
                    s.src = parse_expr(a, L);

                } else {
                    pr_error(SYNTAX_ERR, tk, "invalid argument `"SV_Fmt"` for instruction", SV_Args(tk.txt));
                }
            } else {
                s.kind = KIND_VAL;
                s.src.t = EXPR_LIT_STR;
                s.src.v.as_str = tk.txt;
            }
            break;
        }
        case TK_NUMBER:
        case TK_AMPERSAND:
        case TK_OPEN_PAREN: {
            token_back(L, 1);
            s.kind = KIND_VAL;
            s.src = parse_expr(a, L);
            break;
        }
        default: {
            pr_error(SYNTAX_ERR, tk, "invalid type `%s` when paring instruction `%u`",
                    tk_type_as_cstr(tk.type), inst_as_cstr(s.inst));
        }
    }

    return s;
}

StateConst conststat(Arena *a, Lexer *L)
{
    StateConst s = {0};
    String_View name = token_yield(L, TK_TEXT).txt;
    Expr value = parse_expr(a, L);

    s.name = name;
    s.value = value;

    return s;
}

StateLable labelstat(Lexer *L)
{
    StateLable s = {0};
    Token tk = token_yield(L, TK_TEXT);
    token_yield(L, TK_COLON);
    s.name = tk.txt;
    return s;
}

StateEntry entrystat(Lexer *L)
{
    token_yield(L, TK_ENTRY); // skip TK_ENTRY
    return (StateEntry){ .name = labelstat(L).name };
}
