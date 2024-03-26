#include "../include/linizer.h"

void line_push(Linizer *lnz, Line line) { da_append(lnz, line); }

int try_inst(Hash_Table *ht, Token tk) 
{
    int result = 0;
    if (tk.type == TYPE_TEXT) {
        char *str = sv_to_cstr(tk.txt);
        int inst = ht_get_inst(ht, str);

        if (inst == -1) result = 0;
        else result = 1;
        
        free(str);
    }
    return result;
}

void line_clean(Linizer *lnz)
{
    for (size_t i = 0; i < lnz->count; ++i) {
        lex_clean(&lnz->items[i].item);
    }
    da_clean(lnz);
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
            default:
                fprintf(stderr, "Error: unknown type int linizer: `%u`", lnz->items[i].type);
                exit(1);
        }
        printf(" : line [%zu]", i);
        print_lex(&(lnz->items[i].item), LEX_PRINT_MODE_FALSE);
        printf("\n");
    }
}

Linizer linizer(Lexer *lex, Hash_Table *ht, int ht_debug, int lnz_debug)
{
    Linizer lnz = {0};

    if (ht->capacity == 0) {
        inst_ht_init(ht, ht_debug);
    }

    while (1) {
        Token tk = token_next(lex);
        if (tk.type == TYPE_NONE) break;
        
        Lexer sub_lex = {0};
        Line line = {0}; 

        if (try_inst(ht, tk)) {
            do {
                Token_Type next = token_peek(lex);
                if (next == TYPE_COLON) break;
                lex_push(&sub_lex, tk);
                tk = token_next(lex);
                if (tk.type == TYPE_NONE) break;
            } while (!try_inst(ht, tk));

            if (tk.type != TYPE_NONE) {
                token_back(lex, 1);
            }

            line.item = sub_lex;
            line.type = LINE_INST;

        } else {
            Token t = token_next(lex);
            if (t.type == TYPE_COLON) {
                lex_push(&sub_lex, tk);
                lex_push(&sub_lex, t);

                line.item = sub_lex;
                line.type = LINE_LABEL;

            } else if (t.type == TYPE_NONE) {
                break;
            } else {
                fprintf(stderr, "Error: linizer expected label or inst\n");
                exit(1);
            }
        }
        line_push(&lnz, line);
    }

    if (lnz_debug) print_lnz(&lnz);
    return lnz;
}