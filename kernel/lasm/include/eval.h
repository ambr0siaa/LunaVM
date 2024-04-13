#ifndef EVAL_H_
#define EVAL_H_

#include "lexer.h"
#include "consts.h"

typedef struct eval_node {
    Token token;
    struct eval_node *left_operand; 
    struct eval_node *right_operand; 
} Eval_Node;

typedef struct {
    Eval_Node *root;
    size_t count;
} Eval;

// This macro make need indent, when printing Eval
#define TAB(iter) ({                    \
    for (int j = 0; j < (iter); ++j) {  \
        printf(" ");                    \
    }                                   \
    (iter)++;                           \
})

void print_eval(Eval *Eval);
void print_node(Eval_Node *node);
void print_eval_root(Eval_Node *node);

#define BINARY_OP(dst, operator, op1, op2, type)                                        \
    do {                                                                                \
        if (type == 'f')                                                                \
            (dst)->token.val.f64 = (op1)->token.val.f64 operator (op2)->token.val.f64;  \
        else if (type == 'i')                                                           \
            (dst)->token.val.i64 = (op1)->token.val.i64 operator (op2)->token.val.i64;  \
    } while(0)              

void eval_tree(Eval *eval);
void parse_arefmetic_expr(Eval *eval, Lexer *lex);
Token parse_constant_expr(Token tk, Const_Table *ct);
void eval_clean(Eval_Node *node, size_t *node_count);
void eval_push_subtree(Eval *eval, Eval_Node *subtree);
void subtree_node_count(Eval_Node *subtree, size_t *count);

Eval_Node *eval_node_create(Token tk);
Eval_Node *resolve_eval(Eval_Node *node);
Eval_Node *parse_expr(Token tk, Lexer *lex);
Eval_Node *parse_term(Token tk, Lexer *lex);

#endif // EVAL_H_