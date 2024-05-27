#include "../include/lexer.h"

Value tokenise_value(String_View sv)
{
    int is_float = sv_is_float(sv);

    if (is_float) {
        char *float_cstr = malloc(sizeof(char) * sv.count + 1);
        char *endptr = float_cstr; 
        memcpy(float_cstr, sv.data, sv.count);

        double d = strtod(float_cstr, &float_cstr);

        if (d == 0 && endptr == float_cstr) {
            fprintf(stderr, "Error: cannot parse `%s` to float64\n",float_cstr);
            exit(1);
        }
        
        free(endptr);
        return VALUE_FLOAT(d);

    } else {
        int64_t value = sv_to_int(sv);
        return VALUE_INT(value);
    }
}

void lex_append(Arena *arena, Lexer *lex, Token tk) { da_append(arena, lex, tk); }

int tokenizer(String_View *src, Token *tk, size_t *location)
{
    // TODO: remove special
    const String_View special = sv_from_cstr("+-*/():,.;$&{}[]=");

    if (isdigit(src->data[0])) {
        String_View value = sv_cut_value(src);
        tk->val = tokenise_value(value);
        tk->type = TK_VALUE;
        sv_cut_space_left(src);

    } else if (char_in_sv(special, src->data[0])){
        switch(src->data[0]) {
            case '=': tk->type = TK_EQ;              break;
            case '.': tk->type = TK_DOT;             break;
            case ',': tk->type = TK_COMMA;           break;
            case ':': tk->type = TK_COLON;           break;
            case '$': tk->type = TK_DOLLAR;          break;
            case '/': tk->type = TK_OPERATOR;        break;
            case '+': tk->type = TK_OPERATOR;        break;
            case '*': tk->type = TK_OPERATOR;        break;
            case '-': tk->type = TK_OPERATOR;        break;
            case '&': tk->type = TK_AMPERSAND;       break;
            case '{': tk->type = TK_OPEN_CURLY;      break;
            case '}': tk->type = TK_CLOSE_CURLY;     break;
            case '(': tk->type = TK_OPEN_BRACKET;    break;
            case ')': tk->type = TK_CLOSE_BRACKET;   break;
            case '[': tk->type = TK_OPEN_S_BRACKET;  break;
            case ']': tk->type = TK_CLOSE_S_BRACKET; break;
            case ';': {
                if (src->data[1] == ';') {
                    sv_div_by_delim(src, '\n');
                    return 0;
                } else
                    tk->type = TK_SEMICOLON;
                break;
            }
            default: {
                fprintf(stderr, "Error: unknown operator `%c`\n", src->data[0]);
                exit(1);
            }
        }

        tk->op = src->data[0];
        sv_cut_left(src, 1);
        sv_cut_space_left(src);

    } else if (isalpha(src->data[0])){
        // TODO: Better checking for key words and instructions (as HashTable)
        tk->txt = sv_cut_txt(src, special);

        if (sv_cmp(tk->txt, sv_from_cstr("const"))) {
            tk->type = TK_CONST;
        } else if (sv_cmp(tk->txt, sv_from_cstr("entry"))) {
            tk->type = TK_ENTRY;  
        } else
            tk->type = TK_TEXT;

        sv_cut_space_left(src);

    } else {
        if (src->data[0] == '\000') {
            sv_cut_left(src, 1);
            return 0;
        } else {
            fprintf(stderr, "Error: cannot tokenize `%c`\n", src->data[0]);
            fprintf(stderr, "Current src:\n `"SV_Fmt"`\n", SV_Args(*src));
            return -1;
        }
    }

    tk->location = *location;
    return 1;
}

Lexer lexer(Arena *arena, String_View src)
{
    Lexer lex = {0};
    size_t line_num = 1;
    while (src.count > 0) {
        String_View line = sv_div_by_delim(&src, '\n');
        sv_cut_space_left(&line);
        while (line.count > 0) {
            Token tk = {0};
            if (tokenizer(&line, &tk, &line_num))
                lex_append(arena, &lex, tk);
        }
        line_num++;
    }
    return lex;
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
    else printf("%zu: ", tk.location);

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
        case TK_OPEN_BRACKET:
        case TK_CLOSE_BRACKET:
        case TK_OPEN_S_BRACKET:
        case TK_CLOSE_S_BRACKET:
            printf("`%c`\n", tk.op);
            break;
        case TK_VALUE: {
            if (tk.val.type == VAL_FLOAT) {
                printf("`%lf`\n", tk.val.f64);
            } else {
                printf("`%"PRIi64"`\n", tk.val.i64);
            }
            break;
        }
        case TK_TEXT: {
            printf("txt: `"SV_Fmt"`\n", SV_Args(tk.txt));
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
    if (mode == 1) {
        printf("\n-------------- LEXER --------------\n\n");
    } else {
        printf("\n-----------------------------------\n\n");    
    }
    for (size_t i = 0; i < lex->count; ++i) {
        print_token(lex->items[i]);
    }
    printf("\n-----------------------------------\n\n");
}