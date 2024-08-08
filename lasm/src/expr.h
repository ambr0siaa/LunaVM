#ifndef EXPR_H_
#define EXPR_H_

#include "lexer.h"
#include "consts.h"

#define EMPTY_EXPR (Expr) { .t = EXPR_EMPTY }

LUNA_API Expr expr_str(Token tk);
LUNA_API Expr expr_value(Token tk);
LUNA_API Expr expr_from_binOp(BinaryOp *b);
LUNA_API Expr expr_from_const(String_View name, Const_Table *constT);

LUNA_API BinaryOp *binOp_new(Arena *a);
LUNA_API BinOp_Type binOp_type(Lexer *L);

LUNA_API Expr binOp_operand(Arena *a, Lexer *L);
LUNA_API Expr binOp_eval(BinOp_Type type, Expr lhs, Expr rhs);

LUNA_API Expr parse_binOp(Arena *a, Lexer *L);
LUNA_API Expr parse_subBinOp(Arena *a, Lexer *L);

LUNA_API Expr eval_binOp_expr(Expr src, Const_Table *constT);
LUNA_API Expr eval_expr(Expr src, Const_Table *constT);
LUNA_API Expr parse_expr(Arena *a, Lexer *L);

#endif // EXPR_H_
