#ifndef PARSER_H_
#define PARSER_H_

#include "statement.h"
#include "linizer.h"
#include "consts.h"

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

#define label_append(a, ll, label) arena_da_append(a, ll, label)
LUNA_API Label label_search(Label_List *ll, String_View name);

LUNA_API Object expr_value_as_obj(Expr expr, Const_Table *constT);

LUNA_API void parse_state_inst(StateInst s, Lasm *L);
LUNA_API void parse_state_const(StateConst s, Lasm *L);
LUNA_API void parse_state_entry(StateEntry s, Lasm *L);
LUNA_API void parse_state_label(String_View name, Lasm *L);

LUNA_API int parse_register(String_View sv);
LUNA_API Statement parse_line(Arena *a, Line *line, Hash_Table *instT);

LUNA_API void lasm_push_obj(Lasm *L, Object obj);

LUNA_API void parser_secondary(LasmState *ls, Lasm *L);
LUNA_API LasmState *parser_primary(Lasm *L, Linizer *linizer);

#endif // PARSER_H_
