#ifndef LINIZER_H_
#define LINIZER_H_

#include "lexer.h"
#include "../../common/ht.h"

typedef enum {
    LINE_INST,
    LINE_LABEL,
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

#define LNZ_DEBUG_TRUE 1
#define LNZ_DEBUG_FALSE 0

void print_lnz(Linizer *lnz);
void line_clean(Linizer *lnz);
void line_push(Linizer *lnz, Line line);

int try_inst(Hash_Table *ht, Token tk);
Linizer linizer(Lexer *lex, Hash_Table *ht, int ht_debug, int lnz_debug);

#endif // LINIZER_H_