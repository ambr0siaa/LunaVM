#ifndef LEXER_H_
#define LEXER_H_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "../common/sv.h"
#include "../common/ht.h"

typedef struct Token Token;

#include "error.h"

#define LEXER_KEYS_COUNT 2

extern const char *lexer_key_words[LEXER_KEYS_COUNT];
extern Hash_Table lexer_keys;

typedef enum {
    TK_OPERATOR = 0,
    TK_NUMBER,
    TK_OPEN_BRACKET,
    TK_CLOSE_BRACKET,
    TK_OPEN_PAREN,
    TK_CLOSE_PAREN,
    TK_OPEN_CURLY,
    TK_CLOSE_CURLY,
    TK_SEMICOLON,
    TK_AMPERSAND,
    TK_EQ,
    TK_DOLLAR,
    TK_COLON,
    TK_STRING,
    TK_DOT,
    TK_TEXT,
    TK_COMMA,
    TK_CONST,
    TK_ENTRY,
    TK_NONE
} Token_Type;

struct Token {
    String_View txt;
    Token_Type type;
    uint32_t location;
    uint32_t line;
};

#define TOKEN_NONE (Token) { .type = TK_NONE, .location = 0, .line = 0 }

typedef struct {
    Token *items;
    size_t capacity;
    size_t count;
    int64_t tp;      // Token Pointer
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

#define lexer_append(arena, L, tk) da_append(arena, L, tk);
void print_token(Token tk);

void print_lex(Lexer *lex, int mode);

Token token_next(Lexer *lex);
Token_Type token_peek(Lexer *lex);
void token_back(Lexer *lex, int shift);
Token token_yield(Lexer *lex, Token_Type type);
Token token_get(Lexer *lex, int shift, int skip);

const char *tk_type_as_cstr(Token_Type type);

String_View lexer_cut_string(String_View *src);

int lexer(String_View *src, Token *tk, size_t *location);

size_t lexer_key_get(Hash_Table *ht, const char *key);
void lexer_keys_init(Arena *a, Hash_Table *ht, size_t capacity);

#endif // LEXER_H_