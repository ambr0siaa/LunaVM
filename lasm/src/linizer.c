#include "linizer.h"

Line *line_create(Arena *a, Lexer L, Line_Type type)
{
    Line *line = arena_alloc(a, sizeof(Line));
    line->item = L;
    line->type = type;
    line->next = NULL;
    line->line = (*L.items).line;
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
            case LINE_INST:  printf("INST");  break;
            case LINE_LABEL: printf("LABEL"); break;
            case LINE_CONST: printf("CONST"); break;
            case LINE_ENTRY: printf("ENTRY"); break;
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

Linizer lexical_analyze(Arena *global, String_View src, Hash_Table *instT)
{
    Lexer root = {0};
    Arena local = {0};
    Linizer linizer = {0};

    // first pass
    {
        if (lexer_keys.capacity == 0)
            lexer_keys_init(&local, &lexer_keys, LEXER_KEYS_COUNT + 1);

        size_t line_num = 1;
        size_t line_ptr = 0;

        while (src.count > 0) {
            String_View line = sv_div_by_delim(&src, '\n');
            if (err_global.defined) arena_da_append(global, &err_global, line);
            sv_cut_space_left(&line);

            while (line.count > 0) {
                Token tk = {0};
                if (err_global.defined) tk.line = line_ptr;
                if (lexer(&line, &tk, &line_num))
                    lexer_append(&local, &root, tk);
            }

            line_ptr = line_num++;
        }
    }

    // second pass
    {
        for(;;) {
            Lexer lex = {0};
            size_t type = 0;

            Token tk = token_get(&root, 0, 0);
            if (tk.type == TK_NONE) break;

            if (try_inst(instT, tk)) {
                type = LINE_INST;
                linize_line_inst(global, &lex, &root);
            } else {
                switch (tk.type) {
                    case TK_CONST: {
                        type = LINE_CONST;
                        linize_line_const(global, &lex, &root);
                        break;
                    }
                    case TK_ENTRY: {
                        tk = token_next(&root);
                        lexer_append(global, &lex, tk);
                        type = LINE_ENTRY;
                        linize_line_lable(global, &lex, &root);
                        break;
                    }
                    default: {
                        type = LINE_LABEL;
                        linize_line_lable(global, &lex, &root);
                        break;
                    }
                }
            }

            Line *line = line_create(global, lex, type);
            linizer_push(&linizer, line);
        }
    }

    arena_free(&local);
    return linizer;
}
