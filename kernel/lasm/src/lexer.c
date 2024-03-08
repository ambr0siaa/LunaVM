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

        return VALUE_FLOAT(d);

    } else {
        int64_t value = sv_to_int(sv);
        return VALUE_INT(value);
    }
}

void lex_clean(Lexer *lex) { da_clean(lex); }
void lex_push(Lexer *lex, Token tk) { da_append(lex, tk); }

Lexer lexer(String_View src_sv)
{
    Lexer lex = {0};
    String_View src = sv_trim(src_sv);
    const String_View special = sv_from_cstr("+-*/%():&*%$!#,");
    
    while (src.count != 0) {
        Token tk;
        if (isdigit(src.data[0])) {
            String_View value = sv_cut_value(&src);
            tk.val = tokenise_value(value);
            tk.type = TYPE_VALUE;
            sv_cut_space_left(&src);

        } else if (char_in_sv(special, src.data[0])){
            switch(src.data[0]) {
                case '(': tk.type = TYPE_OPEN_BRACKET;  break;
                case ')': tk.type = TYPE_CLOSE_BRACKET; break;
                case ':': tk.type = TYPE_COLON; break;
                case ',': tk.type = TYPE_COMMA; break;

                case '/': tk.type = TYPE_OPERATOR;   break;
                case '%': tk.type = TYPE_OPERATOR;   break;
                case '+': tk.type = TYPE_OPERATOR;  break;
                case '*': tk.type = TYPE_OPERATOR;  break;
                case '-': tk.type = TYPE_OPERATOR; break;


                default:
                    fprintf(stderr, "Error: unknown operator `%c`\n", src.data[0]);
                    exit(1);
            }

            tk.op = src.data[0];
            sv_cut_left(&src, 1);
            sv_cut_space_left(&src);

        } else if (isalpha(src.data[0])){
            String_View txt = sv_cut_alpha(&src);
            tk.txt = txt;
            tk.type = TYPE_TEXT;
            sv_cut_space_left(&src);

        } else {
            fprintf(stderr, "Error: cannot tokenize `%c`\n", src.data[0]);
            exit(1);
        }
        lex_push(&lex, tk);
    }

    return lex;
}

Token token_next(Lexer *lex)
{
    if (lex->tp >= lex->count) {
        return (Token) { .type = TYPE_NONE };
    } else {
        Token tk = lex->items[lex->tp];
        lex->tp += 1;
        return tk;
    }
}

Token_Type token_peek(Lexer *lex)
{
    if (lex->tp >= lex->count) {
        return TYPE_NONE;
    } else {
        Token_Type type = lex->items[lex->tp].type;
        return type;
    }
}

void print_token(Token tk)
{
    switch (tk.type) {
        case TYPE_VALUE: {
            if (tk.val.type == VAL_FLOAT) {
                printf("float: `%lf`\n", tk.val.f64);
            } else {
                printf("int: `%ld`\n", tk.val.i64);
            }
            break;
        }
        case TYPE_COLON:
        case TYPE_COMMA:
        case TYPE_OPERATOR: {
            printf("opr: `%c`\n", tk.op);
            break;
        }
        case TYPE_OPEN_BRACKET: {
            printf("open bracket: `%c`\n", tk.op);
            break;
        }
        case TYPE_CLOSE_BRACKET: {
            printf("close bracket: `%c`\n", tk.op);
            break;
        }
        case TYPE_TEXT: {
            printf("txt: `"SV_Fmt"`\n", SV_Args(tk.txt));
            break;
        }
        case TYPE_NONE:
        default: 
            fprintf(stderr, "Error: unknown type `%u`\n", tk.type);
            exit(1);
    }
}

void print_lex(Lexer *lex)
{
    printf("\n-------------- LEXER --------------\n\n");
    for (size_t i = 0; i < lex->count; ++i) {
        print_token(lex->items[i]);
    }
    printf("\n-----------------------------------\n\n");
}