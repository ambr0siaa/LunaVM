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
    EXPR_LIT_INT = 0,
    EXPR_LIT_FLT,
    EXPR_LIT_STR,
    EXPR_BINARY_OP
} Expr_Type;

typedef union {
    double as_flt;
    int64_t as_int;
    String_View as_str;
    BinaryOp *as_binop;
} Expr_Value;

typedef struct {
    Expr_Type t;
    Expr_Value v;
} Expr;

// For future expresions
typedef enum {
    BINOP_PLUS = 0,
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

Expr eval_expr(Expr *src, Const_Table *constT);
Expr parse_expr(Arena *a, Lexer *L);

#endif // EXPR_H_