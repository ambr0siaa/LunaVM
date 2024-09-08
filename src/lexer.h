#ifndef LEXER_H_
#define LEXER_H_

#include "common.h"
#include "table.h"
#include "sv.h"

#define LEX_STATUS_OK 1
#define LEX_STATUS_ERR 0
#define LEX_STATUS_EMPTY 2

#define lex_staterr(L) ((L)->status == LEX_STATUS_ERR)
#define lex_statempty(L) ((L)->status == LEX_STATUS_EMPTY)

typedef enum {
    /* Empty */
    TK_NONE = 0,

    /* Common */
    TK_TEXT, TK_NUMBER, TK_STRING, TK_COMMA, TK_DCOLON, 
    TK_COLON, TK_INVOKE, TK_OPEN_PAREN, TK_CLOSE_PAREN,
    TK_OPEN_BRACKET, TK_CLOSE_BRACKET, TK_POINTER,
    TK_DIVERT, TK_DEFER,

    /* Keywords */
    TK_I8, TK_U8, TK_CHAR, TK_I16, TK_U16, TK_I32,
    TK_U32, TK_F32, TK_I64, TK_U64, TK_F64, TK_END,
    TK_LABEL, TK_DEFINE, TK_MODULE, TK_ENTRY, TK_IMPORT,

    AMOUNT_OF_TOKENS 
} Token_Type;

typedef struct {
    Token_Type type;
    String_View text;
    Location loc;
} Token;

#define TOKEN_NONE (Token) {0}

typedef struct {
    int status;
    Table keywords;
    String_View src;
    size_t linenumber;
    char *linestart;
    const char *file;
} Lexer;

LUNA_API Lexer lexer_new(const char *file_path, String_View src);
LUNA_API void lexer_clean(Lexer *lex);

LUNA_API Token lexer_next(Lexer *lex);
LUNA_API Token lexer_peek(Lexer *lex);
LUNA_API Token lexer_yield(Lexer *lex, Token_Type t);

LUNA_API void token_dump(Token tk);
LUNA_API void lexer_dump(Lexer lex);

#endif /* LEXER_H_ */
