#ifndef PARSER_H_
#define PARSER_H_

#include <assert.h>

#include "eval.h"
#include "linizer.h"

#include "../../cpu/src/cpu.h"

typedef uint64_t Inst_Addr;

typedef struct {
    String_View name;
    Inst_Addr addr;
} Label;

typedef struct {
    Label *labels;
    size_t count;
    size_t capacity;
} Label_List;

typedef struct {
    Label_List current;
    Label_List deferred;
} Program_Jumps;

typedef struct {
    Object *items;
    size_t count;
    size_t capacity;
} Object_Block;

typedef struct {
    Object_Block *items;
    size_t count;
    size_t capacity;
} Block_Chain;

#define BLOCK_CHAIN_DEBUG_TRUE 1
#define BLOCK_CHAIN_DEBUG_FALSE 0

#define LINE_DEBUG_TRUE 1
#define LINE_DEBUG_FALSE 0

void objb_clean(Object_Block *objb);
void objb_to_cpu(CPU *c, Object_Block *objb);
void objb_push(Object_Block *objb, Object obj);

void block_chain_debug(Block_Chain *bc);
void block_chain_clean(Block_Chain *block_chain);
void block_chain_to_cpu(CPU *c, Block_Chain *block_chain);
void block_chain_push(Block_Chain *block_chain, Object_Block objb);

void parse_kind_reg(Inst_Addr *inst_pointer, Lexer *lex, Object_Block *objb);
void parse_kind_reg_reg(Inst_Addr *inst_pointer, Lexer *lex, Object_Block *objb);
void parse_kind_reg_val(Inst_Addr *inst_pointer, Lexer *lex, Object_Block *objb, Const_Table *vt);
void parse_kind_val(Inst inst, Inst_Addr *inst_pointer, Lexer *lex, Object_Block *objb, Program_Jumps *PJ, Const_Table *vt, size_t line_num);

Token parse_val(Lexer *lex);
int parse_register(String_View sv);
Inst parse_inst(Lexer *lex, Hash_Table *ht);
Token parse_constant_expr(Token tk, Const_Table *ct);

Const_Statement parse_line_constant(Lexer *lex);
void parse_line_label(String_View name, Program_Jumps *PJ, size_t inst_pointer);
Object_Block parse_line_inst(Line line, Hash_Table *ht, Program_Jumps *PJ, Const_Table *vt, size_t inst_counter, size_t *inst_pointer, int db_line, size_t line_num);

Block_Chain parse_linizer(Linizer *lnz, Program_Jumps *PJ, Hash_Table *ht, Const_Table *vt, int line_debug, int bc_debug);

int translate_inst(String_View inst_sv, Hash_Table *ht);
Inst convert_to_cpu_inst(Inst inst, Inst_Kind *inst_kind, Lexer *lex);

void ll_print(Label_List *ll);
void ll_append(Label_List *ll, Label label);
Label ll_search_label(Label_List *ll, String_View name);

#endif // PARSER_H_