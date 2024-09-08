#include "lexer.h"

#define KEYS_COUNT 17

static const char *key_words[KEYS_COUNT] = {
    "i8", "u8", "char", "i16", "u16", "i32",
    "u32", "f32", "i64", "u64", "f64", "end",
    "label", "define", "module", "entry", "import"
};

#define lex_relocation(L)                   \
    do {                                    \
        (L)->linestart = (L)->src.data + 1; \
        (L)->linenumber += 1;               \
    } while(0)

#define token_location(L, tk)                                           \
    do {                                                                \
        (tk)->loc.row = (L)->linenumber;                                \
        (tk)->loc.col = (size_t)((L)->src.data - (L)->linestart) + 1;   \
    } while(0)

LUNA_FUNC void lexer_init_keys(Table *keywords)
{
    for (size_t i = TK_I8; i < AMOUNT_OF_TOKENS; ++i) {
        char *k = (char*)key_words[i - TK_I8];
        Table_Item item;
        item.value = i;
        item.key = sv_from_parts(k, strlen(k)),
        item.hash = hash_string(item.key.data, item.key.count);
        table_insert(keywords, item, Table);
    }
}

Lexer lexer_new(const char *file_path, String_View src)
{
    Lexer L = {0};
    L.src = src;
    L.linenumber = 1;
    L.status = LEX_STATUS_OK;
    L.linestart = src.data;
    L.file = file_path;
    table_init(&L.keywords, KEYS_COUNT * 2);
    lexer_init_keys(&L.keywords);
    return L; 
}

void lexer_clean(Lexer *lex)
{
    lex->linenumber = 0;
    lex->file = NULL;
    lex->linestart = NULL;
    table_clean(&lex->keywords);
}

LUNA_FUNC void lexer_comments(Lexer *L)
{
    size_t i = 0;
    sv_cut_left(&L->src, 1);
    while (i < L->src.count && L->src.data[i] != '\n') ++i;
    sv_cut_left(&L->src, i + 1);
    L->linestart = L->src.data;
    L->linenumber += 1;
}

LUNA_FUNC String_View lexer_string(String_View *src)
{
    size_t i = 0;
    sv_cut_left(src, 1);
    while (src->data[i] != '"' && i < src->count) ++i;
    String_View result = sv_from_parts(src->data, i);
    sv_cut_left(src, i + 1);
    return result;
}

LUNA_FUNC void lexer_search_key(Table *keywords, Token *tk)
{
    Table_Item item = {0};
    item.hash = hash_string(tk->text.data, tk->text.count);
    table_get(keywords, item);
    if (item.value != 0) {
        tk->type = item.value;
    } else {
        tk->type = TK_TEXT;
    }
}

LUNA_FUNC void lexer_space(Lexer *L)
{
    while (L->src.data && isspace(*L->src.data)) {
        if (*L->src.data == '\n') lex_relocation(L);
        sv_cut_left(&L->src, 1);
    }
}

LUNA_FUNC String_View lexer_number(Lexer *L)
{
    size_t i = 0;
    String_View result = {0};
    if (L->src.data[0] == '0' && L->src.data[1] == 'x') {
        i = 2;
        while (i < L->src.count && !isspace(L->src.data[i])) {
            if (isdigit(L->src.data[i])) ++i;
            else {
                if (L->src.data[i] >= 'a' && L->src.data[i] <= 'f') ++i;
                else {
                    L->status = LEX_STATUS_ERR;
                    break;
                }
            }
        }
    } else {
        while (i < L->src.count
              && (isdigit(L->src.data[i])
              || L->src.data[i] == '.')) ++i;
    }
    result = sv_from_parts(L->src.data, i);
    sv_cut_left(&L->src, i);
    return result;
}

LUNA_FUNC int lexer_character(Lexer *L, Token *tk)
{
    int status = 1;
    size_t shift = 1;
    if (isdigit(L->src.data[0])) {
        tk->type = TK_NUMBER;
        tk->text = lexer_number(L);
        if (lex_staterr(L)) {
            tk->type = TK_NONE;
            tk->loc.col += tk->text.count;
            luna_excp(EXCP_LEXICAL, &tk->loc,
                      "Character `%c` not a hexadecimal value",
                      L->src.data[0]);
            status = -1;
        }
        return status;
    }
    switch (L->src.data[0]) {
        case '?': tk->type = TK_DEFER;     break;
        case ',': tk->type = TK_COMMA;     break;
        case '%': tk->type = TK_INVOKE;    break;
        case '*': tk->type = TK_POINTER;     break;
        case '(': tk->type = TK_OPEN_PAREN;  break;
        case ')': tk->type = TK_CLOSE_PAREN;   break;
        case '[': tk->type = TK_OPEN_BRACKET;  break;
        case ']': tk->type = TK_CLOSE_BRACKET; break;
        case '-': {
            if (L->src.data[1] != '>') {
                luna_excp(EXCP_LEXICAL, &tk->loc, "After `-` expected `>`");
                status = -1;
            } else {
                tk->type = TK_DIVERT;
                shift = 2;
            }
            break;
        } case ':': {
            if (L->src.data[1] == ':') {
                tk->type = TK_DCOLON;
                shift = 2;
            } else {
                tk->type = TK_COLON;
            }
            break;
        } case ';': {
            lexer_comments(L);
            token_location(L, tk);
            status = lexer_character(L, tk);
            shift = 0;
            break;
        } case '"': {
            tk->type = TK_STRING;
            tk->text = lexer_string(&L->src);
            shift = 0;
            break;
        } case '\0': {
            status = -1;
            shift = 0;
            break;
        } case '\n': {
            lex_relocation(L);
            sv_cut_left(&L->src, 1);
            token_location(L, tk);
            status = lexer_character(L, tk);
            shift = 0;
            break;
        } case ' ': {
            lexer_space(L);
            token_location(L, tk);
            status = lexer_character(L, tk);
            shift = 0;
            break;
        } default: {
            status = shift = 0;
            break;
        }
    }
    if (shift != 0) {
        tk->text = sv_from_parts(L->src.data, shift);
        sv_cut_left(&L->src, shift);
    }
    return status;
}

Token lexer_next(Lexer *L)
{
    Token tk = TOKEN_NONE;
    if (L->src.count == 0) {
        L->status = LEX_STATUS_EMPTY;
        goto defer;
    }
    tk.loc.file = L->file; 
    token_location(L, &tk);
    if (!lexer_character(L, &tk)) {
        tk.text = sv_cut_txt(&L->src);
        if (tk.text.count == 0) {
            luna_excp(EXCP_LEXICAL, &tk.loc, "Unknown character `%c`", L->src.data[0]);
            goto defer;
        }
        lexer_search_key(&L->keywords, &tk);
    }
defer:
    return tk;
}

Token lexer_peek(Lexer *L)
{
    Lexer copy = *L;
    Token tk = lexer_next(L);
    int s = L->status;
    *L = copy;
    L->status = s;
    return tk;
}

Token lexer_yield(Lexer *L, Token_Type t)
{
    Token tk = lexer_next(L);
    if (tk.type != t) {
        L->status = LEX_STATUS_ERR;
        luna_excp(EXCP_LEXICAL, &tk.loc,
                  "Expected %u, but provided %u",
                  t, tk.type);
    }
    return tk;
}

void token_dump(Token tk)
{
    if (tk.type == TK_NONE) return;
    printf("[row: %zu, col: %zu] ", tk.loc.row, tk.loc.col);
    switch (tk.type) {
        case TK_NUMBER: {
            luna_report("number ['"SV_Fmt"']", SV_Args(tk.text));
            break;
        } case TK_TEXT: {
            luna_report("text ['"SV_Fmt"']", SV_Args(tk.text));
            break;
        } case TK_STRING: {
            luna_report("string ['"SV_Fmt"']", SV_Args(tk.text));
            break;
        } case TK_POINTER: case TK_DCOLON: case TK_DIVERT: case TK_COLON:
          case TK_COMMA: case TK_INVOKE: case TK_OPEN_PAREN: case TK_DEFER:
          case TK_CLOSE_PAREN: case TK_OPEN_BRACKET: case TK_CLOSE_BRACKET: {
            luna_report("character ['"SV_Fmt"']", SV_Args(tk.text));
            break;
        } case TK_I8: case TK_U8: case TK_CHAR: case TK_I16: case TK_U16:
          case TK_I32: case TK_U32: case TK_F32: case TK_I64: case TK_U64:
          case TK_F64:  case TK_END: case TK_LABEL: case TK_DEFINE: case TK_MODULE:
          case TK_ENTRY: case TK_IMPORT: {
            luna_report("keyword ['"SV_Fmt"']", SV_Args(tk.text));
            break;
        } default: {
            luna_assert(0 && "Unreachable token type");
        }
    }
}

void lexer_dump(Lexer L)
{
    while (!lex_statempty(&L)) {
        Token tk = lexer_next(&L);
        if (tk.type == TK_NONE) break;
        token_dump(tk);
    }
}

