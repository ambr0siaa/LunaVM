#include "../include/eval.h"

void print_node(Eval_Node *node)
{
    print_token(node->token);
}

void print_eval_root(Eval_Node *node)
{
    static int i;

    TAB(i);
    print_node(node);

    if (node->right_operand != NULL) {
        TAB(i);
        printf("right: ");
        print_eval_root(node->right_operand);
        i--;
    }

    if (node->left_operand != NULL) {
        TAB(i);
        printf("left: ");
        print_eval_root(node->left_operand);
        i--;
    }

    i--;
}          

void print_eval(Eval *eval)
{
    printf("\n------------------- Eval -------------------\n\n");
    print_eval_root(eval->root);
    printf("\n------------------------------------------------------------\n\n");
}

Eval_Node *resolve_eval(Eval_Node *node)
{   
    if (node->left_operand != NULL && node->right_operand != NULL) {
        if (node->right_operand->token.type == TYPE_OPERATOR ||
            node->left_operand->token.type == TYPE_OPERATOR ) {
                node->left_operand = resolve_eval(node->left_operand);
                node->right_operand = resolve_eval(node->right_operand);
        }
            
        if (node->token.type == TYPE_OPERATOR) {
            char type;
            Eval_Node *new_node = eval_node_create((Token) {.type = TYPE_VALUE });
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

// Get Eval and calculate final number
void eval_tree(Eval *eval)
{
    eval->root = resolve_eval(eval->root);
    eval->count = 1; 
}

void eval_clean(Eval_Node *node, size_t *node_count)
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
            eval_clean(node->left_operand, node_count);
            eval_clean(node->right_operand, node_count);
        }
    }
}

Eval_Node *eval_node_create(Token tk)
{
    Eval_Node *node = malloc(sizeof(Eval_Node));
    node->token = tk;
    node->left_operand = NULL;
    node->right_operand = NULL;
    return node;
}

void eval_push_subtree(Eval *eval, Eval_Node *subtree)
{
    if (eval->root == NULL) {
        eval->root = subtree;
    } else {
        if (subtree->right_operand != NULL) {
            subtree->left_operand = eval->root;
            eval->root = subtree;
        } else {
            subtree->right_operand = eval->root;
            eval->root = subtree;
        }
    }
}

void subtree_node_count(Eval_Node *subtree, size_t *count) 
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

Eval_Node *parse_expr(Token tk, Lexer *lex)
{
    if (tk.type == TYPE_VALUE) {
        Eval_Node *val1;
        Eval_Node *opr;
        Eval_Node *val2;

        val1 = parse_term(tk, lex);
        Token_Type type;

        do {
            type = token_peek(lex);
            if (type == TYPE_OPERATOR) {
                Token _op = token_next(lex);
                if (_op.op == '+' || _op.op == '-') {
                    opr = eval_node_create(_op);

                    Token v2 = token_next(lex);
                    
                    if (v2.type == TYPE_OPEN_BRACKET) {
                        Token t1 = token_next(lex);
                        val2 = parse_expr(t1, lex);

                        do {
                            Token opr_tk = token_next(lex);
                            if (opr_tk.type == TYPE_OPERATOR) {
                                if (opr_tk.op == '*' || opr_tk.op == '/') {
                                    Eval_Node *opr_node = eval_node_create(opr_tk);
                                    Eval_Node *subval;

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
                                    token_back(lex, 1);
                                    break;
                                }
                            } else {
                                token_back(lex, 1);
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
                Eval_Node *subtree = parse_expr(t1, lex);
                
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
                printf("tp: %li\n", lex->tp);
                printf("count: %zu\n", lex->count);
                exit(1);
            }
        } while(1); 

        return val1;

    } else if (tk.type == TYPE_OPEN_BRACKET) {
        Token_Type type;
        Token t1 = token_next(lex);
        Eval_Node *subtree = parse_expr(t1, lex);

        do {
            type = token_peek(lex);
            if (type == TYPE_OPERATOR) {
                Eval_Node *val;
                Token opr_tk = token_next(lex);
                Eval_Node *opr_node = eval_node_create(opr_tk);

                Token tk2 = token_next(lex);
                if (tk2.type == TYPE_OPEN_BRACKET) {
                    Token tk3 = token_next(lex);
                    val = parse_expr(tk3, lex);
                    
                    if (val == NULL) exit(1);

                    Token t2 = token_next(lex);
                    if (t2.type == TYPE_OPERATOR) {
                        Eval_Node *subval;
                        Eval_Node *op_node = eval_node_create(t2);

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
                        token_back(lex, 1);
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

Eval_Node *parse_term(Token tk, Lexer *lex)
{   
    Eval_Node *val1 = eval_node_create(tk);  

    do {
        Token_Type tk_type = token_peek(lex);
        if (tk_type == TYPE_NONE) break;

        if (tk_type == TYPE_OPERATOR) {
            Eval_Node *val2;
            Token opr_tk = token_next(lex);

            if (opr_tk.op == '*' || opr_tk.op == '/') {
                Eval_Node *opr_node = eval_node_create(opr_tk);
                Token t1 = token_next(lex);

                if (t1.type == TYPE_VALUE) {
                    val2 = eval_node_create(t1);

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
                token_back(lex, 1);
                break;
            }
        } else {
            break;
        }
    } while (1);

    return val1;
}

void parse_arefmetic_expr(Eval *eval, Lexer *lex)
{
    if ((size_t)lex->tp + 1 == lex->count) {
        eval->root = eval_node_create(lex->items[lex->tp]);
        return;
    }

    while (1) {
        Token tk = token_next(lex);
        if (tk.type == TYPE_NONE) break;
        
        size_t count = 0;
        if (tk.type == TYPE_VALUE) {
            Token_Type type = token_peek(lex);
            if (type == TYPE_OPERATOR) {
                Token t1 = token_next(lex);
                if (t1.op == '+' || t1.op == '-') {
                    token_back(lex, 1);

                    Eval_Node *subtree = parse_expr(tk, lex);
                    if (subtree == NULL) exit(1);

                    subtree_node_count(subtree, &count);
                    eval_push_subtree(eval, subtree);

                } else if (t1.op == '*' || t1.op == '/') {
                    token_back(lex, 1);
                    Eval_Node *val = parse_term(tk, lex);

                    subtree_node_count(val, &count);
                    eval_push_subtree(eval, val);
                }
            }

        } else if (tk.type == TYPE_OPERATOR) {
            Eval_Node *val;
            Eval_Node *opr = eval_node_create(tk);
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
            eval_push_subtree(eval, opr);

        } else if (tk.type == TYPE_OPEN_BRACKET) {
            Token tok = token_next(lex);
            Eval_Node *subtree = parse_expr(tok, lex);
            if (subtree == NULL) exit(1);

            subtree_node_count(subtree, &count);
            eval_push_subtree(eval, subtree);

        } else if (tk.type == TYPE_CLOSE_BRACKET) {
            token_next(lex);
        }
        eval->count += count;
    }
}