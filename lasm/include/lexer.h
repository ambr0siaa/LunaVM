#ifndef LEXER_H_
#define LEXER_H_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "./sv.h"

#ifndef ARENA_H_
#include "./arena.h"
#endif // ARENA_H_

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
    TK_OPERATOR = 0,
    TK_VALUE,
    TK_OPEN_S_BRACKET,
    TK_CLOSE_S_BRACKET,
    TK_OPEN_BRACKET,
    TK_CLOSE_BRACKET,
    TK_OPEN_CURLY,
    TK_CLOSE_CURLY,
    TK_SEMICOLON,
    TK_AMPERSAND,
    TK_EQ,
    TK_DOLLAR,
    TK_COLON,
    TK_CONST,
    TK_ENTRY,
    TK_DOT,
    TK_TEXT,
    TK_COMMA,
    TK_NONE
} Token_Type;

typedef struct {
    Token_Type type;
    union {
        Value val;
        char op;
        String_View txt;
    };
    uint64_t location;
} Token;

typedef struct {
    int64_t tp;         // Token Pointer
    size_t count;
    uint8_t debug_info;
    size_t capacity;
    Token *items;
} Lexer;

#define INIT_CAPACITY 8

// macro for append item to dynamic array
#define da_append(arena, da, new_item)                                                                         \
    do {                                                                                                       \
        if ((da)->capacity == 0) {                                                                             \
            (da)->count = 0;                                                                                   \
            (da)->capacity = INIT_CAPACITY;                                                                    \
            (da)->items = arena_alloc(arena, sizeof(*(da)->items) * (da)->capacity);                           \
        }                                                                                                      \
                                                                                                               \
        if ((da)->count + 1 >= (da)->capacity) {                                                               \
            size_t old_size = (da)->capacity * sizeof(*(da)->items);                                           \
            (da)->capacity *= 2;                                                                               \
            (da)->items = arena_realloc(arena, (da)->items, old_size, (da)->capacity * sizeof(*(da)->items));  \
            assert((da)->items != NULL);                                                                       \
        }                                                                                                      \
        (da)->items[(da)->count++] = (new_item);                                                               \
    } while(0)

void print_token(Token tk);

// prints at header of lexer `LEXER` if true
#define LEX_PRINT_MODE_TRUE 1
#define LEX_PRINT_MODE_FALSE 0

void print_lex(Lexer *lex, int mode);
void lex_append(Arena *arena, Lexer *lex, Token tk);

Token token_next(Lexer *lex);
Token token_yield(Lexer *lex, Token_Type type);
Token_Type token_peek(Lexer *lex);
void token_back(Lexer *lex, int shift);
Token token_get(Lexer *lex, int shift, int skip);
int tokenizer(String_View *src, Token *tk, size_t *location);

#define SKIP_TRUE 1
#define SKIP_FALSE 0

Value tokenise_value(String_View sv);
Lexer lexer(Arena *arena, String_View src_sv);

#endif // LEXER_H_