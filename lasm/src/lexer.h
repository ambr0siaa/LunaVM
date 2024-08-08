#ifndef LEXER_H_
#define LEXER_H_

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>

#include "../../common/sv.h"
#include "../../common/ht.h"
#include "../../common/types.h"

#include "error.h"

#define LEXER_KEYS_COUNT 2

extern const char *lexer_key_words[LEXER_KEYS_COUNT];
extern Hash_Table lexer_keys;

#define TOKEN_NONE (Token) { .type = TK_NONE, .location = 0, .line = 0 }

typedef struct {
    Token *items;
    size_t capacity;
    size_t count;
    int64_t tp;      // Token Pointer
} Lexer;

#define lexer_append(a, L, tk) arena_da_append(a, L, tk);

LUNA_API void print_token(Token tk);
LUNA_API void print_lex(Lexer *lex, int mode);

LUNA_API void token_back(Lexer *lex, int shift);
LUNA_API Token token_yield(Lexer *lex, Token_Type type);
LUNA_API Token token_get(Lexer *lex, int shift, int skip);
LUNA_API Token token_next(Lexer *lex);
LUNA_API Token_Type token_peek(Lexer *lex);

LUNA_API const char *tk_type_as_cstr(Token_Type type);

LUNA_API String_View lexer_cut_string(String_View *src);

LUNA_API int lexer(String_View *src, Token *tk, size_t *location);

LUNA_API size_t lexer_key_get(Hash_Table *ht, const char *key);
LUNA_API void lexer_keys_init(Arena *a, Hash_Table *ht, size_t capacity);

#endif // LEXER_H_
