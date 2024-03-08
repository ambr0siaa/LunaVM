//TODO: Impelment parser for luna's assembler

#include "../include/parser.h"

void print_node(Ast_Node *node)
{
    print_token(node->token);
}

void print_ast_root(Ast_Node *node)
{
    static int i;

    TAB(i);
    print_node(node);

    if (node->right_operand != NULL) {
        TAB(i);
        printf("right: ");
        print_ast_root(node->right_operand);
        i--;
    }

    if (node->left_operand != NULL) {
        TAB(i);
        printf("left: ");
        print_ast_root(node->left_operand);
        i--;
    }

    i--;
}          

void print_ast(Ast *ast)
{
    printf("\n------------------- Abstract Syntax Tree -------------------\n\n");
    print_ast_root(ast->root);
    printf("\n------------------------------------------------------------\n\n");
}

Ast_Node *resolve_ast(Ast_Node *node)
{   
    if (node->left_operand != NULL && node->right_operand != NULL) {
        if (node->right_operand->token.type == TYPE_OPERATOR ||
            node->left_operand->token.type == TYPE_OPERATOR ) {
                node->left_operand = resolve_ast(node->left_operand);
                node->right_operand = resolve_ast(node->right_operand);
        }
            
        if (node->token.type == TYPE_OPERATOR) {
            char type;
            Ast_Node *new_node = ast_node_create((Token) {.type = TYPE_VALUE });
            if (node->left_operand->token.val.type == VAL_FLOAT) {
                type = 'f'; 
                new_node->token.val.type = VAL_FLOAT;
            } else {
                type = 'i'; 
                new_node->token.val.type = VAL_INT;
            } 

            switch (node->token.op) {
                case '+': BINARY_OP(new_node, +, node->left_operand, node->right_operand, type); break;
                case '*': BINARY_OP(new_node, *, node->left_operand, node->right_operand, type); break;
                case '-': BINARY_OP(new_node, -, node->left_operand, node->right_operand, type); break;
                case '/': BINARY_OP(new_node, /, node->left_operand, node->right_operand, type); break;
                default: {
                    fprintf(stderr, "Error, unknown operator `%c`\n", node->token.op);
                    exit(1);
                }
            }

            free(node->left_operand);
            free(node->right_operand);
            free(node);
            return new_node;
        }
    }
    return node;
}

// Get ast and calculate final number
void eval(Ast *ast)
{
    ast->root = resolve_ast(ast->root);
    ast->count = 1; 
}

void ast_clean(Ast_Node *node, size_t *node_count)
{
    if (*node_count == 1) free(node);
    else {
        if (node->token.type == TYPE_OPERATOR) {
            if (node->left_operand->token.type == TYPE_VALUE && 
                node->right_operand->token.type == TYPE_VALUE) {
                free(node->right_operand);
                free(node->left_operand);
                *node_count -= 2; 
            }
        } else if (node->token.type == TYPE_VALUE) {
            return;
        } else {
            ast_clean(node->left_operand, node_count);
            ast_clean(node->right_operand, node_count);
        }
    }
}

Ast_Node *ast_node_create(Token tk)
{
    Ast_Node *node = malloc(sizeof(Ast_Node));
    node->token = tk;
    node->left_operand = NULL;
    node->right_operand = NULL;
    return node;
}

void ast_push_subtree(Ast *ast, Ast_Node *subtree)
{
    if (ast->root == NULL) {
        ast->root = subtree;
    } else {
        if (subtree->right_operand != NULL) {
            subtree->left_operand = ast->root;
            ast->root = subtree;
        } else {
            subtree->right_operand = ast->root;
            ast->root = subtree;
        }
    }
}

void subtree_node_count(Ast_Node *subtree, size_t *count) 
{
    if (subtree == NULL) return;
    subtree_node_count(subtree->left_operand, count);
    *count += 1;
    subtree_node_count(subtree->right_operand, count);
}

/*
*  Grammar:
*
*   E - expresion
*   T - term
*   V - value
*   
*   * - mean that can be one or more T, V or E
*   
*   E: T { + | -  T }*
*   T: V { * | /  V }*
*   V: INT | FLOAT
*/


Ast_Node *parse_expr(Token tk, Lexer *lex)
{
    if (tk.type == TYPE_VALUE) {
        Ast_Node *val1;
        Ast_Node *opr;
        Ast_Node *val2;

        val1 = parse_term(tk, lex);
        Token_Type type;

        do {
            type = token_peek(lex);
            if (type == TYPE_OPERATOR) {
                Token _op = token_next(lex);
                if (_op.op == '+' || _op.op == '-') {
                    opr = ast_node_create(_op);

                    Token v2 = token_next(lex);
                    
                    if (v2.type == TYPE_OPEN_BRACKET) {
                        Token t1 = token_next(lex);
                        val2 = parse_expr(t1, lex);

                        do {
                            Token opr_tk = token_next(lex);
                            if (opr_tk.type == TYPE_OPERATOR) {
                                if (opr_tk.op == '*' || opr_tk.op == '/') {
                                    Ast_Node *opr_node = ast_node_create(opr_tk);
                                    Ast_Node *subval;

                                    Token tok = token_next(lex);
                                    if (tok.type == TYPE_VALUE) {
                                        subval = parse_term(tok, lex);

                                    } else if (tok.type == TYPE_OPEN_BRACKET) {
                                        tok = token_next(lex);
                                        subval = parse_expr(tok, lex);
                                    }

                                    opr_node->left_operand = val2;
                                    opr_node->right_operand = subval;
                                    val2 = opr_node;

                                } else {
                                    lex->tp -= 1;
                                    break;
                                }
                            } else {
                                lex->tp -= 1;
                                break;
                            }
                        } while(1);

                    } else if (v2.type == TYPE_VALUE) {
                        val2 = parse_term(v2, lex);

                    } else {
                        fprintf(stderr, "Error: in function `parse_expr` unknown condition\n");
                        exit(1);
                    }

                    if (val2 == NULL) exit(1);
                    
                    opr->left_operand = val1;
                    opr->right_operand = val2;
                    val1 = opr;
                }
            } else if (type == TYPE_OPEN_BRACKET) {
                Token t1 = token_next(lex);
                Ast_Node *subtree = parse_expr(t1, lex);
                
                if (subtree == NULL) exit(1);
                return subtree;

            } else if (type == TYPE_NONE) {
                return val1;

            } else if (type == TYPE_CLOSE_BRACKET) {
                token_next(lex);
                return val1;

            } else {
                fprintf(stderr, "Error: in `parse_expr` unknown token type `%u`\n", type);
                print_token(lex->items[lex->tp]);
                printf("tp: %zu\n", lex->tp);
                printf("count: %zu\n", lex->count);
                exit(1);
            }
        } while(1); 

        return val1;

    } else if (tk.type == TYPE_OPEN_BRACKET) {
        Token_Type type;
        Token t1 = token_next(lex);
        Ast_Node *subtree = parse_expr(t1, lex);

        do {
            type = token_peek(lex);
            if (type == TYPE_OPERATOR) {
                Ast_Node *val;
                Token opr_tk = token_next(lex);
                Ast_Node *opr_node = ast_node_create(opr_tk);

                Token tk2 = token_next(lex);
                if (tk2.type == TYPE_OPEN_BRACKET) {
                    Token tk3 = token_next(lex);
                    val = parse_expr(tk3, lex);
                    
                    if (val == NULL) exit(1);

                    Token t2 = token_next(lex);
                    if (t2.type == TYPE_OPERATOR) {
                        Ast_Node *subval;
                        Ast_Node *op_node = ast_node_create(t2);

                        Token t3 = token_next(lex);
                        if (t3.type == TYPE_OPEN_BRACKET) {
                            Token t4 = token_next(lex);
                            subval = parse_expr(t4, lex);

                            if (subval == NULL) exit(1);

                        } else if (t3.type == TYPE_VALUE) {
                            subval = parse_term(t3, lex);
                        }

                        op_node->left_operand = val;
                        op_node->right_operand = subval;
                        val = op_node;

                    } else {
                        lex->tp -= 1;
                    }

                } else if (tk2.type == TYPE_VALUE) {
                    val = parse_term(tk2, lex);
                }

                opr_node->left_operand = subtree;
                opr_node->right_operand = val;
                subtree = opr_node;

            } else if (type == TYPE_CLOSE_BRACKET) {
                token_next(lex);
            } else {
                break;
            }
        } while (1);

        return subtree;
        
    } else {
        fprintf(stderr, "Error: unknown type in function `parse_expr` in 1st condition\n");
        return NULL;
    }
}

Ast_Node *parse_term(Token tk, Lexer *lex)
{   
    Ast_Node *val1 = ast_node_create(tk);  

    do {
        Token_Type tk_type = token_peek(lex);
        if (tk_type == TYPE_NONE) break;

        if (tk_type == TYPE_OPERATOR) {
            Ast_Node *val2;
            Token opr_tk = token_next(lex);

            if (opr_tk.op == '*' || opr_tk.op == '/') {
                Ast_Node *opr_node = ast_node_create(opr_tk);
                Token t1 = token_next(lex);

                if (t1.type == TYPE_VALUE) {
                    val2 = ast_node_create(t1);

                } else if (t1.type == TYPE_OPEN_BRACKET) {
                    Token tok = token_next(lex); 
                    val2 = parse_expr(tok, lex);
            
                } else if (t1.type == TYPE_NONE) {
                    fprintf(stderr, "Error: expected second operand\n");
                    exit(1);
                }

                if (val2 == NULL) exit(1);

                opr_node->left_operand = val1;
                opr_node->right_operand = val2;
                val1 = opr_node;

            } else {
                lex->tp -= 1;
                break;
            }
        } else {
            break;
        }
    } while (1);

    return val1;
}

void parse_value(Ast *ast, Lexer *lex)
{
    while (1) {
        Token tk = token_next(lex);
        if (tk.type == TYPE_NONE) break;
        
        size_t count = 0;
        if (tk.type == TYPE_VALUE) {
            Token_Type type = token_peek(lex);
            if (type == TYPE_OPERATOR) {
                Token t1 = token_next(lex);
                if (t1.op == '+' || t1.op == '-') {
                    lex->tp -= 1;

                    Ast_Node *subtree = parse_expr(tk, lex);
                    if (subtree == NULL) exit(1);

                    subtree_node_count(subtree, &count);
                    ast_push_subtree(ast, subtree);

                } else if (t1.op == '*' || t1.op == '/') {
                    lex->tp -= 1;
                    Ast_Node *val = parse_term(tk, lex);

                    subtree_node_count(val, &count);
                    ast_push_subtree(ast, val);
                }
            }

        } else if (tk.type == TYPE_OPERATOR) {
            Ast_Node *val;
            Ast_Node *opr = ast_node_create(tk);
            Token tok = token_next(lex);

            if (tok.type == TYPE_VALUE) {
                val = parse_term(tok, lex);
            
            } else if (tok.type == TYPE_OPEN_BRACKET) {
                Token t = token_next(lex);
                val = parse_expr(t, lex);    
                if (val == NULL) exit(1);

            } else {
                fprintf(stderr, "Error: in function `parser` unknown condition\n");
                exit(1);
            }

            count += 1; // its `opr`
            subtree_node_count(val, &count);

            opr->right_operand = val;
            ast_push_subtree(ast, opr);

        } else if (tk.type == TYPE_OPEN_BRACKET) {
            Token tok = token_next(lex);
            Ast_Node *subtree = parse_expr(tok, lex);
            if (subtree == NULL) exit(1);

            subtree_node_count(subtree, &count);
            ast_push_subtree(ast, subtree);

        } else if (tk.type == TYPE_CLOSE_BRACKET) {
            token_next(lex);
        }

        ast->count += count;
    }
}