#ifndef STATEMENT_H_
#define STATEMENT_H_

#include "expr.h"

typedef struct LasmBlock LasmBlock;

struct LasmBlock {
    Statement s;
    LasmBlock *next;
};

typedef struct {
    LasmBlock *head;
    LasmBlock *tail;
} LasmState;

LUNA_API StateEntry entrystat(Lexer *L);
LUNA_API StateLable labelstat(Lexer *L);
LUNA_API StateConst conststat(Arena *a, Lexer *L);
LUNA_API StateInst inststat(Arena *a, Lexer *L, Hash_Table *instT);

LUNA_API LasmState *lasmstate_new(Arena *a);
LUNA_API LasmBlock *lasmblock_new(Arena *a, Statement s);
LUNA_API void lasmstate_push(LasmState *ls, LasmBlock *lb);

LUNA_API int try_register(String_View sv);
LUNA_API int parse_register(String_View sv);

#endif // STATEMENT_H_
