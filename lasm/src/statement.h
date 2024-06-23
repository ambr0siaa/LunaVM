#ifndef STATEMENT_H_
#define STATEMENT_H_

#ifndef CPU_H_
#include "../../luna/src/luna.h"
#endif

#include "expr.h"

typedef struct {
    Inst inst;
    Expr src;
    Register dst;
    Inst_Kind kind;
} StateInst;

typedef struct {
    String_View name;
} StateLable;

typedef struct {
    String_View name;
    Expr value;
} StateConst;

typedef struct {
    String_View name;
} StateEntry;

typedef enum {
    STATE_INST = 0,
    STATE_LABLE,
    STATE_CONST,
    STATE_ENTRY
} State_Type;

typedef union {
    StateInst as_inst;
    StateLable as_lable;
    StateConst as_const;
    StateEntry as_entry;
} State_Value;

typedef struct {
    State_Type t;
    State_Value v;
    uint32_t location;
    uint32_t line;
} Statement;

typedef struct LasmBlock LasmBlock;

struct LasmBlock {
    Statement s;
    LasmBlock *next;
};

typedef struct {
    LasmBlock *head;
    LasmBlock *tail;
} LasmState;


StateEntry entrystat(Lexer *L);
StateLable labelstat(Lexer *L);
StateConst conststat(Arena *a, Lexer *L);
StateInst inststat(Arena *a, Lexer *L, Hash_Table *instT);

LasmState *lasmstate_new(Arena *a);
LasmBlock *lasmblock_new(Arena *a, Statement s);
void lasmstate_push(LasmState *ls, LasmBlock *lb);

int try_register(String_View sv);
int parse_register(String_View sv);

#endif // STATEMENT_H_
