#ifndef LINIZER_H_
#define LINIZER_H_

#ifndef LEXER_H_
#include "lexer.h"
#endif

typedef enum {
    LINE_INST = 0,
    LINE_LABEL,
    LINE_CONST,
    LINE_ENTRY
} Line_Type;

typedef struct line {
    Lexer item;
    uint32_t line;
    Line_Type type;
    uint32_t location;
    struct line *next;
} Line;

typedef struct {
    Line *head;
    Line *tail;
} Linizer;

void print_linizer(Linizer *linizer);
void linizer_push(Linizer *linizer, Line *line);
Line *line_create(Arena *a, Lexer L, Line_Type type);

int try_inst(Hash_Table *ht, Token tk);
void linize_line_inst(Arena *a, Lexer *dst, Lexer *src);
void linize_line_lable(Arena *a, Lexer *dst, Lexer *src);
void linize_line_const(Arena *a, Lexer *dst, Lexer *src);

Linizer lexical_analyze(Arena *global, String_View src, Hash_Table *instT);

#endif // LINIZER_H_