#include "../include/linizer.h"

void line_push(Arena *arena, Linizer *lnz, Line line) { da_append(arena, lnz, line); }

int try_inst(Hash_Table *ht, Token tk) 
{
    int result = 0;
    if (tk.type == TYPE_TEXT) {
        char *str = sv_to_cstr(tk.txt);
        int inst = inst_table_get(ht, str);

        if (inst == -1) result = 0;
        else result = 1;
        
        free(str);
    }
    return result;
}

void print_lnz(Linizer *lnz)
{
    printf("\n");
    for (size_t i = 0; i < lnz->count; ++i) {
        switch (lnz->items[i].type) {
            case LINE_INST:
                printf("INST");
                break;
            case LINE_LABEL:
                printf("LABEL");
                break;
            case LINE_CONSTANT:
                printf("CONSTANT");
                break;
            case LINE_ENTRY_LABLE:
                printf("ENTRY POINT");
                break;
            default:
                fprintf(stderr, "Error: unknown type int linizer: `%u`\n", lnz->items[i].type);
                exit(1);
        }
        printf(" : line [%zu]", i);
        print_lex(&(lnz->items[i].item), LEX_PRINT_MODE_FALSE);
        printf("\n");
    }
}

void linize_line_lable(Arena *arena, Lexer *dst, Lexer *src)
{
    Token name = token_get(src, -1, SKIP_FALSE);
    Token tk = token_next(src);
    if (tk.type == TYPE_COLON) {
        lex_push(arena, dst, name);
        lex_push(arena, dst, tk);
    } else {
        fprintf(stderr, "Error: linizer expected label or inst\n");
        exit(1);
    }
}

void linize_line_constant(Arena *arena, Lexer *dst, Lexer *src)
{
    Token tk = token_next(src);
    while (tk.type != TYPE_CLOSE_CURLY) {
        lex_push(arena, dst, tk);
        tk = token_next(src);
    }
    lex_push(arena, dst, tk);
}

void linize_line_inst(Arena *arena, Lexer *dst, Lexer *src, Hash_Table *ht, Token tk)
{
    do {
        Token_Type next = token_peek(src);
        if (next == TYPE_COLON) break;
        lex_push(arena, dst, tk);
        tk = token_next(src);
        if (tk.type == TYPE_NONE) break;
        if (tk.type == TYPE_DOT) break;
    } while (!try_inst(ht, tk));

    if (tk.type != TYPE_NONE) {
        token_back(src, 1);
    }
}

Linizer linizer(Arena *arena, Lexer *lex, Hash_Table *ht, int ht_debug, int lnz_debug)
{
    Linizer lnz = {0};

    if (ht->capacity == 0)
        inst_table_init(arena, ht, ht_debug);

    while (1) {
        Token tk = token_next(lex);
        if (tk.type == TYPE_NONE) break;
        
        Lexer sub_lex = {0};
        Line line = {0}; 

        if (try_inst(ht, tk)) {
            line.type = LINE_INST;
            linize_line_inst(arena, &sub_lex, lex, ht, tk);

        } else {
            if (tk.type == TYPE_DOT) {
                tk = token_next(lex);
                if (tk.type == TYPE_TEXT) {
                    if (sv_cmp(tk.txt, sv_from_cstr("entry"))) {
                        tk = token_next(lex);
                        line.type = LINE_ENTRY_LABLE;
                        linize_line_lable(arena, &sub_lex, lex);

                    } else if (sv_cmp(tk.txt, sv_from_cstr("constant"))) {
                        line.type = LINE_CONSTANT;
                        linize_line_constant(arena, &sub_lex, lex);
                    }
                }
            } else {
                line.type = LINE_LABEL;
                linize_line_lable(arena, &sub_lex, lex);
            }
        }

        line.item = sub_lex;
        line_push(arena, &lnz, line);
    }

    if (lnz_debug) print_lnz(&lnz);
    return lnz;
}