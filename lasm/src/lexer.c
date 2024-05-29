#include "../include/lexer.h"

Hash_Table lexer_keys = {0};

const char *lexer_key_words[LEXER_KEYS_COUNT] = {
    "const", "entry"
};

void lexer_keys_init(Arena *a, Hash_Table *ht, size_t capacity)
{
    size_t tk = TK_CONST;
    ht_init(a, ht, capacity);
    for (size_t i = 0; i < LEXER_KEYS_COUNT; ++i) {
        size_t *value = arena_alloc(a, sizeof(size_t));
        *value = tk + i;
        ht_insert(a, ht, lexer_key_words[i], (void*)(value));
    }
}

size_t lexer_key_get(Hash_Table *ht, const char *key)
{
    size_t *dst;
    if (!ht_get(ht, key, (void**)(&dst)))
        return 0;
    return *dst;
}

String_View lexer_cut_string(String_View *src)
{
    if (src->data[0] == '"')
        sv_cut_left(src, 1);

    size_t i = 0;
    while (i < src->count && src->data[i] != '"') {
        i++;
    }

    String_View result = sv_from_parts(src->data, i);
    sv_cut_left(src, i + 1); // plus 1 for skipping quote

    return result;
}

int tokenizer(String_View *src, Token *tk, size_t *location)
{
    int shift = 1;
    tk->location = *location;

    if (isdigit(src->data[0])) {
        tk->txt = sv_cut_value(src);
        tk->type = TK_NUMBER;
        sv_cut_space_left(src);

    } else {
        switch(src->data[0]) {
            case '/':
            case '+':
            case '*':
            case '-': tk->type = TK_OPERATOR;      break;
            case '=': tk->type = TK_EQ;            break;
            case '.': tk->type = TK_DOT;           break;
            case ',': tk->type = TK_COMMA;         break;
            case ':': tk->type = TK_COLON;         break;
            case '$': tk->type = TK_DOLLAR;        break;
            case '&': tk->type = TK_AMPERSAND;     break;
            case '{': tk->type = TK_OPEN_CURLY;    break;
            case '}': tk->type = TK_CLOSE_CURLY;   break;
            case '(': tk->type = TK_OPEN_PAREN;    break;
            case ')': tk->type = TK_CLOSE_PAREN;   break;
            case '[': tk->type = TK_OPEN_BRACKET;  break;
            case ']': tk->type = TK_CLOSE_BRACKET; break;
            case '"': {
                shift = 0;
                tk->txt = lexer_cut_string(src);
                tk->type = TK_STRING;
                break;
            }
            case ';': {
                if (src->data[1] == ';') {
                    sv_div_by_delim(src, '\n');
                    return 0;
                } else tk->type = TK_SEMICOLON;
                break;
            }
            case '\000': {
                sv_cut_left(src, shift);
                return 0;
            }
            default: {
                shift = 0;
                tk->txt = sv_cut_txt(src);
                if (tk->txt.count == 0) {
                    tk->txt.count = 1;
                    pr_error(LEXICAL_ERR, *tk, "cannot tokenize `%c`", src->data[0]);
                    return -1;
                }

                char *key = sv_to_cstr(tk->txt);
                size_t type = lexer_key_get(&lexer_keys, key);
                if (!type) tk->type = TK_TEXT;
                else tk->type = type;
                free(key);

                sv_cut_space_left(src);
                break;
            }
        }

        if (shift != 0) tk->txt = sv_from_parts(src->data, 1);
        sv_cut_left(src, shift);
        sv_cut_space_left(src);
    }

    return 1;
}

void lexer_create(Arena *arena, String_View src, Lexer *L)
{
    Arena local = {0};
    if (lexer_keys.capacity == 0)
        lexer_keys_init(&local, &lexer_keys, LEXER_KEYS_COUNT);

    size_t line_num = 1;
    size_t line_ptr = 0;

    while (src.count > 0) {
        String_View line = sv_div_by_delim(&src, '\n');
        if (err_global.defined) da_append(arena, &err_global, line);
        sv_cut_space_left(&line);

        while (line.count > 0) {
            Token tk = {0};
            if (err_global.defined) tk.line = line_ptr;
            int status = tokenizer(&line, &tk, &line_num);
            if (status > 0) lexer_append(arena, L, tk);
        }

        line_ptr = line_num++;
    }

    arena_free(&local);
}

Token token_next(Lexer *lex)
{
    if ((size_t)lex->tp >= lex->count) {
        return (Token) { .type = TK_NONE };
    } else {
        Token tk = lex->items[lex->tp];
        lex->tp += 1;
        return tk;
    }
}

Token_Type token_peek(Lexer *lex)
{
    if ((size_t)lex->tp >= lex->count) {
        return TK_NONE;
    } else {
        Token_Type type = lex->items[lex->tp].type;
        return type;
    }
}

Token token_yield(Lexer *lex, Token_Type type)
{
    Token tk = token_next(lex);
    if (tk.type == TK_NONE) {
        fprintf(stderr, "Error: all tokens were yielded\n");
        exit(1);
    }

    if (tk.type != type) {
        fprintf(stderr, "Error: expected %d, but provided %d", type, tk.type);
        exit(1);
    }

    return tk;
}

void token_back(Lexer *lex, int shift)
{
    if (lex->tp - shift >= 0) {
        lex->tp -= shift;
    } else {
        fprintf(stderr, "Error: token pointer must be > 0!\n");
        exit(1);
    }
}

Token token_get(Lexer *lex, int shift, int skip)
{
    if ((size_t)lex->tp + shift >= lex->count) {
        return (Token) { .type = TK_NONE };
    } else {
        Token tk = lex->items[lex->tp + shift];
        if (skip) lex->tp += shift + 1;
        return tk;
    }
}

void print_token(Token tk)
{
    printf("line ");
    if (tk.location == 0) printf("unknown: ");
    else printf("%u: ", tk.location);

    switch (tk.type) {
        case TK_EQ:
        case TK_DOT:
        case TK_COLON:
        case TK_COMMA:
        case TK_DOLLAR:
        case TK_OPERATOR:
        case TK_AMPERSAND:
        case TK_SEMICOLON:
        case TK_OPEN_CURLY:
        case TK_CLOSE_CURLY:
        case TK_OPEN_PAREN:
        case TK_CLOSE_PAREN:
        case TK_OPEN_BRACKET:
        case TK_CLOSE_BRACKET:
            printf("`%c`\n", tk.txt.data[0]);
            break;
        case TK_NUMBER: {
            printf("number: `"SV_Fmt"`\n",
                   SV_Args(tk.txt));
            break;
        }
        case TK_TEXT: {
            printf("txt: `"SV_Fmt"`\n",
                   SV_Args(tk.txt));
            break;
        }
        case TK_STRING: {
            printf("string: `"SV_Fmt"`\n",
                   SV_Args(tk.txt));
            break;
        }
        case TK_CONST: {
            printf("constant\n");
            break;
        }
        case TK_ENTRY: {
            printf("entry\n");
            break;
        }
        case TK_NONE: {
            printf("End\n");
            break;
        }
        default: 
            fprintf(stderr, "Error: unknown type `%u`\n", tk.type);
            exit(1);
    }
}

void print_lex(Lexer *lex, int mode)
{
    printf("\n-----------------------------------\n\n");    
    for (size_t i = 0; i < lex->count; ++i) {
        Token tk = lex->items[i];
        print_token(tk);
        if (err_global.defined && mode)
            printf("reference: `"SV_Fmt"`\n",
                    SV_Args(err_line(tk.line)));
    }
    printf("\n-----------------------------------\n\n");
}