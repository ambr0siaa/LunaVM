#ifndef LUNA_H_
#define LUNA_H_

// TODO: separate instructions stuff to diffrent file 
#include <stdlib.h>
#include <inttypes.h>

#include "../../common/arena.h"
#include "../../common/types.h"

#define LUNA_PROGRAM_INIT_CAPACITY 1024
#define LUNA_STACK_CAPACITY 16384

// Count of regs in array on luna
#define LUNA_REGS 10

// Register from enum (without RT and RTF) + registers from Luna (ip, sp, fp, zero_flag)
#define LUNA_STACK_FRAME_SIZE 24

// kernel of virtual machine
typedef struct {
    int64_t regs[LUNA_REGS];
    double regsf[LUNA_REGS];

    Object *program;
    uint64_t program_capacity;
    uint64_t program_size;
    uint64_t ip;

    Object stack[LUNA_STACK_CAPACITY];
    uint64_t stack_size;
    uint64_t sp;
    uint64_t fp;

    uint8_t zero_flag;
    uint8_t halt;
} Luna;

extern Inst inst_defs[IC][NUMBER_OF_KINDS];

LUNA_API int inst_has_1_op(Inst inst);
LUNA_API int inst_has_2_regs(Inst inst);
LUNA_API int inst_has_no_ops(Inst inst);
LUNA_API int inst_isjump(Inst inst);

LUNA_API char *inst_as_cstr(Inst inst);
LUNA_API char *reg_as_cstr(uint64_t operand);
LUNA_API char *luna_shift_args(int *argc, char ***argv);

LUNA_API void luna_inst_return(Luna *c);
LUNA_API Object luna_fetch(Luna *const c);

LUNA_API void debug_regs(Luna *const c);
LUNA_API void debug_stack(Luna *const c);

LUNA_API void luna_execute_inst(Luna *const c);
LUNA_API void luna_clean_program(Luna *const c);
LUNA_API void luna_execute_program(Luna *const c, int debug, int limit, int stk);

LUNA_API void load_program_from_file(Arena *arena, Luna *c, const char *file_path);
LUNA_API void load_program_to_luna(Luna *c, Object *program, size_t program_size);

#endif // LUNA_H_
