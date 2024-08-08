#include "expr.h"

Expr expr_from_const(String_View name, Const_Table *constT)
{
    Expr e = {0};
    Const_Statement *cnst = ct_get(constT, name);
    if (cnst == NULL) { /* TODO: make errors more informative */
        pr_error(SYNTAX_ERR, TOKEN_NONE,
                    "cannot get constant by name `"SV_Fmt"`",
                    SV_Args(name));
    }

    switch (cnst->type) {
        case CONST_TYPE_FLOAT:
            e.t = EXPR_LIT_FLT;
            e.v.as_flt = cnst->value.as_f64;
            break;
        case CONST_TYPE_INT:
            e.t = EXPR_LIT_INT;
            e.v.as_int = cnst->value.as_i64;
            break;
        case CONST_TYPE_UINT:
            e.t = EXPR_LIT_INT;
            e.v.as_int = cnst->value.as_u64;
            break;
        default:
            pr_error(SYNTAX_ERR, TOKEN_NONE,
                     "invalid type `%u` for constant value",
                     cnst->type);
    }

    return e;
}

Expr expr_from_binOp(BinaryOp *b)
{
    return (Expr) {
        .t = EXPR_BINARY_OP,
        .v.as_binOp = b
    };
}

Expr expr_value(Token tk)
{
    Expr e = {0};
    if (sv_is_float(tk.txt)) {
        e.t = EXPR_LIT_FLT;
        e.v.as_flt = sv_to_flt(tk.txt);
    } else {
        e.t = EXPR_LIT_INT;
        e.v.as_int = sv_to_int(tk.txt);
    }
    return e;
}

Expr expr_str(Token tk)
{
    return (Expr) {
        .t = EXPR_LIT_STR,
        .v.as_str = tk.txt
    };
}

BinaryOp *binOp_new(Arena *a)
{
    BinaryOp *b = arena_alloc(a, sizeof(BinaryOp));
    b->left = b->right = EMPTY_EXPR;
    b->type = BINOP_EMPTY;
    return b;
}

BinOp_Type binOp_type(Lexer *L)
{
    BinOp_Type type;
    Token tk = token_next(L);
    if (tk.type != TK_OPERATOR)
        return BINOP_EMPTY;

    switch (tk.txt.data[0]) {
        case '*': type = BINOP_MUL;   break;
        case '/': type = BINOP_DIV;   break;
        case '+': type = BINOP_PLUS;  break;
        case '-': type = BINOP_MINUS; break;
        default: pr_error(SYNTAX_ERR, tk,
                          "Unknown operator `%c`",
                          tk.txt.data[0]);
    }

    return type;
}

Expr binOp_operand(Arena *a, Lexer *L)
{
    Expr e = {0};
    Token tk = token_next(L);

    switch (tk.type) {
        case TK_NUMBER:     e = expr_value(tk);       break;
        case TK_OPEN_PAREN: e = parse_subBinOp(a, L); break;
        case TK_AMPERSAND: {
            tk = token_yield(L, TK_TEXT);
            e = expr_str(tk);
            break;
        }
        default: {
            pr_error(SYNTAX_ERR, tk,
                     "invalid opernad: type `%s`",
                     tk_type_as_cstr(tk.type));
        }
    }

    return e;
}

#define BINOP_EVAL(dst, op, lhs, rhs) \
    do { \
        if ((lhs).t == EXPR_LIT_FLT && \
            (rhs).t == EXPR_LIT_FLT) { \
                *(dst) = (Expr) { \
                    .t = EXPR_LIT_FLT, \
                    .v.as_flt = (lhs).v.as_flt op (rhs).v.as_flt \
                }; \
        } else if ((lhs).t == EXPR_LIT_INT && \
                   (rhs).t == EXPR_LIT_INT) { \
                *(dst) = (Expr) { \
                    .t = EXPR_LIT_INT, \
                    .v.as_int = (lhs).v.as_int op (rhs).v.as_int \
                }; \
        } else { /* TODO: Makes errors more informative*/ \
            pr_error(SYNTAX_ERR, TOKEN_NONE, /* TODO: instead of real types print them as string */ \
                     "operands have diffrent type." \
                     "left: `%u`, right `%u`.", \
                     (lhs).t, (rhs).t); \
        } \
    } while (0)

Expr binOp_eval(BinOp_Type type, Expr lhs, Expr rhs)
{
    Expr e = {0};
    switch (type) {
        case BINOP_MUL:   BINOP_EVAL(&e, *, lhs, rhs); break;
        case BINOP_DIV:   BINOP_EVAL(&e, /, lhs, rhs); break;
        case BINOP_PLUS:  BINOP_EVAL(&e, +, lhs, rhs); break;
        case BINOP_MINUS: BINOP_EVAL(&e, -, lhs, rhs); break;
        default: { /* TODO: Makes errors more informative and intead of printing real type print string */
            pr_error(SYNTAX_ERR, TOKEN_NONE,
                     "cannot evaluate operator `%u`",
                     type);
        }
    }
    return e;
}

Expr parse_binOp(Arena *a, Lexer *L)
{
    Expr e = {0};
    BinOp_Type type;
    Expr lhs = binOp_operand(a, L);
    Token_Type next = token_peek(L);
    if (next == TK_NONE ||
        next == TK_CLOSE_PAREN) {
            e = lhs;
            goto defer;
    }

    BinaryOp *b = binOp_new(a);

    b->left = lhs;
    b->type = binOp_type(L);
    b->right = binOp_operand(a, L);

    for(;;) {
        type = binOp_type(L);
        if (type == BINOP_DIV ||
            type == BINOP_MUL) {
                BinaryOp *sub = binOp_new(a);
                sub->right = binOp_operand(a, L);
                sub->left = b->right;
                sub->type = type;
                b->right = expr_from_binOp(sub);
        } else {
            token_back(L, 1);
            break;
        }
    }

    e = expr_from_binOp(b);
    defer: return e;
}

Expr parse_subBinOp(Arena *a, Lexer *L)
{
    Expr e = parse_expr(a, L);
    token_yield(L, TK_CLOSE_PAREN);
    return e;
}

Expr parse_expr(Arena *a, Lexer *L)
{
    Expr e = parse_binOp(a, L);

    for (;;) {
        Token_Type type = token_peek(L);
        if (type == TK_CLOSE_PAREN ||
            type == TK_NONE) break;

        BinaryOp *b = binOp_new(a);
        b->type = binOp_type(L);
        b->right = parse_binOp(a, L);
        b->left = e;
        e = expr_from_binOp(b);
    }

    return e;
}

Expr eval_binOp_expr(Expr src, Const_Table *constT)
{
    if (src.t != EXPR_BINARY_OP) {
        if (src.t == EXPR_LIT_STR)
            return expr_from_const(src.v.as_str, constT);
        return src;
    }

    Expr lhs = eval_binOp_expr(src.v.as_binOp->left, constT);
    Expr rhs = eval_binOp_expr(src.v.as_binOp->right, constT);

    return binOp_eval(src.v.as_binOp->type, lhs, rhs);
}

Expr eval_expr(Expr src, Const_Table *constT)
{
    Expr e = {0};
    if (src.t == EXPR_LIT_FLT ||
        src.t == EXPR_LIT_INT)        e = src;
    else if (src.t == EXPR_LIT_STR)   e = expr_from_const(src.v.as_str, constT);
    else if (src.t == EXPR_BINARY_OP) e = eval_binOp_expr(src, constT);
    else pr_error(SYNTAX_ERR, TOKEN_NONE,
                  "cannot evaluate type `%u`",
                  src.t);
    return e;
}
