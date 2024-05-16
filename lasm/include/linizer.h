#ifndef LINIZER_H_
#define LINIZER_H_

#include "lexer.h"
#include "ht.h"

typedef enum {
    LINE_INST = 0,
    LINE_LABEL,
    LINE_CONSTANT,
    LINE_ENTRY_LABLE
} Line_Type;

typedef struct line {
    Lexer item;
    Line_Type type;
} Line;

typedef struct {
    Line *items;
    size_t count;
    size_t capacity;
} Linizer;

void print_lnz(Linizer *lnz);
void line_push(Arena *arena, Linizer *lnz, Line line);

int try_inst(Hash_Table *ht, Token tk);
void linize_line_lable(Arena *arena, Lexer *dst, Lexer *src);
void linize_line_constant(Arena *arena, Lexer *dst, Lexer *src);
void linize_line_inst(Arena *arena, Lexer *dst, Lexer *src, Hash_Table *ht, Token tk);
Linizer linizer(Arena *arena, Lexer *lex, Hash_Table *ht, int ht_debug, int lnz_debug);

#endif // LINIZER_H_