#ifndef EXPR_H_
#define EXPR_H_

#ifndef LEXER_H_
#include "lexer.h"
#endif

#ifndef CONSTS_H_
#include "consts.h"
#endif

typedef struct BinaryOp BinaryOp;

typedef enum {
    EXPR_EMPTY = 0,
    EXPR_LIT_INT,
    EXPR_LIT_FLT,
    EXPR_LIT_STR,
    EXPR_BINARY_OP
} Expr_Type;

typedef union {
    double as_flt;
    int64_t as_int;
    String_View as_str;
    BinaryOp *as_binOp;
} Expr_Value;

typedef struct {
    Expr_Type t;
    Expr_Value v;
} Expr;

#define EMPTY_EXPR (Expr) { .t = EXPR_EMPTY }

// For future expresions
typedef enum {
    BINOP_EMPTY = 0,
    BINOP_PLUS,
    BINOP_MINUS,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_GT,
    BINOP_GE,
    BINOP_LE,
    BINOP_EQ
} BinOp_Type;

struct BinaryOp {
    BinOp_Type type;
    Expr left;
    Expr right;
};

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

Expr expr_str(Token tk);
Expr expr_value(Token tk);
Expr expr_from_binOp(BinaryOp *b);
Expr expr_from_const(String_View name, Const_Table *constT);

BinaryOp *binOp_new(Arena *a);
BinOp_Type binOp_type(Lexer *L);

Expr binOp_operand(Arena *a, Lexer *L);
Expr binOp_eval(BinOp_Type type, Expr lhs, Expr rhs);

Expr parse_binOp(Arena *a, Lexer *L);
Expr parse_subBinOp(Arena *a, Lexer *L);

Expr eval_binOp_expr(Expr src, Const_Table *constT);
Expr eval_expr(Expr src, Const_Table *constT);
Expr parse_expr(Arena *a, Lexer *L);

#endif // EXPR_H_