#ifndef ASM_H_
#define ASM_H_

#include "../cpu/cpu.h"
#include "../common/sv.h"
#include "../common/ht.h"

#define INIT_CAPACITY 32
typedef uint64_t Inst_Addr;

typedef struct {
    String_View name;
    Inst_Addr addr;
} Lable;

typedef struct {
    Lable *lables;
    size_t count;
    size_t capacity;
} Lable_List;

typedef struct {
    Lable_List current;
    Lable_List deferred;
} Program_Jumps;

extern String_View asm_load_file(const char *file_path);
extern Register parse_register(String_View sv);
extern Inst parse_inst(String_View inst_sv);

extern char *asm_shift_args(int *argc, char ***argv);
extern void asm_translate_source(CPU *c, Program_Jumps *PJ, String_View src);
extern void asm_cut_comments(String_View *line);

extern void ll_append(Lable_List *ll, Lable lable);
extern Lable ll_search_lable(Lable_List *ll, String_View name);

#endif // ASM_H_
