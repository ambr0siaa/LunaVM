#include "../include/linizer.h"

Line *line_create(Arena *a, Lexer L, Line_Type type)
{
    Line *line = arena_alloc(a, sizeof(Line));
    line->item = L;
    line->type = type;
    line->next = NULL;
    line->location = (*L.items).location;
    return line;
}

void linizer_push(Linizer *linizer, Line *line)
{
    if (!linizer->head && !linizer->tail) {
        linizer->head = line;
        linizer->tail = line;
    } else {
        linizer->tail->next = line;
        linizer->tail = line;
    }
}

int try_inst(Hash_Table *ht, Token tk) 
{
    int result = 0;
    char *str = sv_to_cstr(tk.txt);
    int inst = inst_table_get(ht, str);

    if (inst == -1) result = 0;
    else result = 1;

    free(str);
    return result;
}

void print_linizer(Linizer *linizer)
{
    printf("\n");
    Line *line = linizer->head;
    while (line != NULL) {
        switch (line->type) {
            case LINE_INST:        printf("INST");        break;
            case LINE_LABEL:       printf("LABEL");       break;
            case LINE_CONST:       printf("CONSTANT");    break;
            case LINE_ENTRY_LABLE: printf("ENTRY LABEL"); break;
            default:
                pr_error(ERROR, "unknown type for linizer: `%u`",
                         line->type);
        }
        printf(" : [%u]", line->location);
        print_lex(&line->item, 0);
        line = line->next;
    }
}

void linize_line_lable(Arena *a, Lexer *dst, Lexer *src)
{
    Token tk = token_yield(src, TK_TEXT);
    lexer_append(a, dst, tk);
    tk = token_yield(src, TK_COLON);
    lexer_append(a, dst, tk);
}

void linize_line_const(Arena *a, Lexer *dst, Lexer *src)
{
    size_t loc = token_yield(src, TK_CONST).location;

    Token tk = token_yield(src, TK_TEXT);
    lexer_append(a, dst, tk);

    token_yield(src, TK_EQ);

    while(1) {
        tk = token_next(src);

        if (loc != tk.location) {
            if (tk.type != TK_NONE)
                token_back(src, 1);
            break;
        }

        lexer_append(a, dst, tk);
        loc = tk.location;
    }
}

void linize_line_inst(Arena *a, Lexer *dst, Lexer *src)
{
    Token tk;
    size_t loc = 0;
    for (;;) {
        tk = token_next(src);
        if (loc != 0 && loc != tk.location) {
            if (tk.type != TK_NONE)
                token_back(src, 1);
            break;
        }
        lexer_append(a, dst, tk);
        loc = tk.location;
    }
}

Linizer linizer_init(Arena *a, Lexer *src, Hash_Table *ht)
{
    Linizer linizer = {0};

    for(;;) {
        Lexer dst = {0};
        size_t type = 0;

        Token tk = token_get(src, 0, 0);
        if (tk.type == TK_NONE) break;

        if (try_inst(ht, tk)) {
            type = LINE_INST;
            linize_line_inst(a, &dst, src);
        } else {
            switch (tk.type) {
                case TK_CONST: {
                    type = LINE_CONST;
                    linize_line_const(a, &dst, src);
                    break;
                }
                case TK_ENTRY: {
                    tk = token_next(src);
                    lexer_append(a, &dst, tk);
                    type = LINE_ENTRY_LABLE;
                    linize_line_lable(a, &dst, src);
                    break;
                }
                default: {
                    type = LINE_LABEL;
                    linize_line_lable(a, &dst, src);
                    break;
                }
            }
        }
        Line *line = line_create(a, dst, type);
        linizer_push(&linizer, line);
    }

    return linizer;
}