#ifndef PARSER_H_
#define PARSER_H_

//TODO: implemenet parser

#include "./lexer.h"

typedef struct ast_node {
    Token token;
    struct ast_node *left_operand; 
    struct ast_node *right_operand; 
} Ast_Node;

typedef struct {
    Ast_Node *root;
    size_t count;
} Ast;

// This macro make need indent, when printing ast
#define TAB(iter) ({                    \
    for (int j = 0; j < (iter); ++j) {  \
        printf(" ");                    \
    }                                   \
    (iter)++;                           \
})

void print_ast(Ast *ast);
void print_node(Ast_Node *node);
void print_ast_root(Ast_Node *node);

#define BINARY_OP(dst, operator, op1, op2, type)                                        \
    do {                                                                                \
        if (type == 'f')                                                                \
            (dst)->token.val.f64 = (op1)->token.val.f64 operator (op2)->token.val.f64;  \
        else if (type == 'i')                                                           \
            (dst)->token.val.i64 = (op1)->token.val.i64 operator (op2)->token.val.i64;  \
    } while(0)              

void eval(Ast *ast);
void parse_value(Ast *ast, Lexer *lex);
void ast_clean(Ast_Node *node, size_t *node_count);
void ast_push_subtree(Ast *ast, Ast_Node *subtree);
void subtree_node_count(Ast_Node *subtree, size_t *count);

Ast_Node *ast_node_create(Token tk);
Ast_Node *resolve_ast(Ast_Node *node);
Ast_Node *parse_expr(Token tk, Lexer *lex);
Ast_Node *parse_term(Token tk, Lexer *lex);

#endif // PARSER_H_