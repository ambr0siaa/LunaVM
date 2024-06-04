#ifndef PARSER_H_
#define PARSER_H_

#include "statement.h"
#include "linizer.h"
#include "consts.h"

typedef uint64_t Inst_Addr;

typedef struct {
    String_View name;
    Inst_Addr addr;
} Label;

// TODO: make hash table intead of dynamic array
typedef struct {
    size_t count;
    size_t capacity;
    Label *items;
} Label_List;

#define label_append(a, ll, label) da_append(a, ll, label)
Label label_search(Label_List *ll, String_View name);

typedef struct {
    size_t entry;
    String_View src;
    Label_List jmps;
    Hash_Table instT; /*instruction table*/
    Const_Table constT; /*table of constants*/
    Label_List defered_jmps;
    size_t program_capacity;
    size_t program_size;
    Object *program;
    const char *input_file;
    const char *output_file;
    Arena *a;
} Lasm;

Object expr_value_as_obj(Expr *expr, Const_Table *constT);

void parse_state_inst(StateInst s, Lasm *L);
void parse_state_const(StateConst s, Lasm *L);
void parse_state_entry(StateEntry s, Lasm *L);
void parse_state_label(String_View name, Lasm *L);

int parse_register(String_View sv);
Statement parse_line(Arena *a, Line *line, Hash_Table *instT);

void lasm_push_obj(Lasm *L, Object obj);

void parser_secondary(LasmState *ls, Lasm *L);
LasmState *parser_primary(Lasm *L, Linizer *linizer);

#endif // PARSER_H_