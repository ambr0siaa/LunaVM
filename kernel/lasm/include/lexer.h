#ifndef LEXER_H_
#define LEXER_H_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../../common/sv.h"

typedef enum {
    VAL_FLOAT = 0,
    VAL_INT
} Value_Type;

typedef struct {
    Value_Type type;
    union {
        int64_t i64;
        double f64;
    };
} Value;

#define VALUE_INT(val) (Value) { .type = VAL_INT, .i64 = (val) }
#define VALUE_FLOAT(val) (Value) { .type = VAL_FLOAT, .f64 = (val) }

typedef enum {
    TYPE_OPERATOR = 0,
    TYPE_VALUE,
    TYPE_OPEN_BRACKET,
    TYPE_CLOSE_BRACKET,
    TYPE_SEMICOLON,
    TYPE_DOLLAR,
    TYPE_COLON,
    TYPE_TEXT,
    TYPE_COMMA,
    TYPE_NONE
} Token_Type;

typedef struct {
    Token_Type type;
    union {
        Value val;
        char op;
        String_View txt;
    };
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
    int64_t tp;         // Token Pointer
} Lexer;

#define INIT_CAPACITY 8
#define LEX_DEBUG_TRUE 1
#define LEX_DEBUG_FALSE 0
#define LEX_DEBUG_TXTS_TRUE 1
#define LEX_DEBUG_TXTS_FALSE 0

// macro for append item to dynamic array
#define da_append(da, new_item)                                                         \
    do {                                                                                \
        if ((da)->count + 1 >= (da)->capacity) {                                        \
            (da)->capacity = (da)->capacity > 0 ? (da)->capacity * 2 : INIT_CAPACITY;   \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items));  \
            assert((da)->items != NULL);                                                \
        }                                                                               \
        (da)->items[(da)->count++] = (new_item);                                        \
    } while(0)

#define da_clean(da)        \
    do {                    \
        free((da)->items);  \
        (da)->items = NULL; \
        (da)->count = 0;    \
        (da)->capacity = 0; \
    } while(0)

void print_token(Token tk);

// prints at header of lexer `LEXER` if true
#define LEX_PRINT_MODE_TRUE 1
#define LEX_PRINT_MODE_FALSE 0

void print_lex(Lexer *lex, int mode);
void lex_clean(Lexer *lex);
void lex_push(Lexer *lex, Token tk);

Token token_next(Lexer *lex);
Token_Type token_peek(Lexer *lex);
void token_back(Lexer *lex, int shift);

#define SKIP_TRUE 1
#define SKIP_FALSE 0
Token token_get(Lexer *lex, int shift, int skip);

Value tokenise_value(String_View sv);
Lexer lexer(String_View src_sv, int db_txt);

#endif // LEXER_H_