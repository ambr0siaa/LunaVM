#ifndef PARSER_H_
#define PARSER_H_

#include "common.h"
#include "lexer.h"
#include "arena.h"
#include "table.h"
#include "sv.h"

#define stmtarg_append(a, da, item) arena_da_append(a, da, item)

#define LABEL_MODE_ADDR    0xc
#define LABEL_MODE_PTR     0xd
#define LABEL_MODE_FUNCALL 0xe
#define LABEL_MODE_MODULE  0xf

#define LUNA_SECTION_DATA  0
#define LUNA_SECTION_BSS   1
#define LUNA_SECTION_STACK 2
#define LUNA_SECTION_HEAP  3

#define defer_status(s) do { L->status = (s); goto defer; } while(0)
#define LUNA_CODE_INIT_CAP 128

LUNA_API void luna_write(Luna *L, LObject *o);
LUNA_API void parse_statement(Arena *a, Luna *L, Lexer *lex);
LUNA_API void luna_translate_stmt(Arena *a, Luna *L, Lexer *lex);
LUNA_API void luna_backpatching(Arena *a, Luna *L);

LUNA_API void statement_print(Statement *s);
LUNA_API void expr_print(Expr *e);

#endif /* PARSER_H_ */
