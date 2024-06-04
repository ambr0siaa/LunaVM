#include "expr.h"

Expr parse_expr(Arena *a, Lexer *L)
{
    // TODO: Evaluate harder expresions
    (void) a;
    Expr expr = {0};
    Token tk = token_next(L);
    switch (tk.type) {
        case TK_AMPERSAND: {
            tk = token_next(L);
            expr.t = EXPR_LIT_STR;
            expr.v.as_str = tk.txt;
            break;
        }
        case TK_NUMBER: {
            if (sv_is_float(tk.txt)) {
                expr.t = EXPR_LIT_FLT;
                expr.v.as_flt = sv_to_flt(tk.txt);
            } else {
                expr.t = EXPR_LIT_INT;
                expr.v.as_int = sv_to_int(tk.txt);
            }
            break;
        }
        default:
            pr_error(ERROR, "Case `%s` not implemented for now",
                     tk_type_as_cstr(tk.type));
    }
    return expr;
}

Expr eval_expr(Expr *src, Const_Table *constT)
{
    Expr expr = {0};
    if (src->t == EXPR_LIT_STR) {
        Const_Statement *cnst = ct_get(constT, src->v.as_str);
        if (cnst == NULL) {
            pr_error(SYNTAX_ERR, TOKEN_NONE,
                     "cannot get constant by name `"SV_Fmt"`",
                     SV_Args(src->v.as_str));
        }
        if (cnst->type == CONST_TYPE_FLOAT) {
            expr.t = EXPR_LIT_FLT;
            expr.v.as_flt = cnst->value.as_f64;
        } else if (cnst->type == CONST_TYPE_INT) {
            expr.t = EXPR_LIT_INT;
            expr.v.as_flt = cnst->value.as_i64;
        } else {
            pr_error(SYNTAX_ERR, TOKEN_NONE, "invalid type `%u` for constant value", cnst->type);
        }
    } else {
        expr = *src;
    }
    return expr;
}