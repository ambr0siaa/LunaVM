#ifndef CPU_H_
#define CPU_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define EXECUTION_LIMIT 256
#define PROGRAM_INIT_CAPACITY 1024
#define ARRAY_SIZE(arr) sizeof((arr)) / sizeof((arr)[0])

typedef enum {
    R0 = 0, R1, R2, R3, R4, R5, R6, R7, R8, ACC,
    F0, F1, F2, F3, F4, F5, F6, F7, F8, ACCF,
    RC 
} Register;
 
typedef enum {
    INST_MOV = 0,
    INST_MOVI,
    INST_MOVF,

    INST_HLT,
    INST_DBR,   // DeBug Register

    INST_ADDI,
    INST_SUBI,
    INST_DIVI,
    INST_MULI,

    INST_ADDF,
    INST_SUBF,
    INST_DIVF,
    INST_MULF,

    INST_JMP,
    INST_JNZ,
    INST_JZ,
    INST_CMP,
    IC          // IC -> inst count
} Inst;

typedef union {
    Inst inst;
    double f64;
    int64_t i64;
    uint64_t u64;
    Register reg;
} Object;

typedef struct {
    int64_t regs[RC];
    double regfs[RC];
    // TODO: add stack

    Object *program;
    uint64_t program_capacity;
    uint64_t program_size;
    uint64_t ip;

    int zero_flag : 1;
    int halt : 1;
} CPU;

#define OBJ_INST(type)   (Object) { .inst = (type) }
#define OBJ_FLOAT(val)   (Object) { .f64 = (val) }
#define OBJ_UINT(val)    (Object) { .u64 = (val) }
#define OBJ_REG(r)       (Object) { .reg = (r) }
#define OBJ_INT(val)     (Object) { .i64 = (val) }

extern void debug_regs(CPU *c);
extern void cpu_execute_inst(CPU *c);
extern void cpu_execute_program(CPU *c, int debug, int limit);

extern void load_program_to_file(CPU *c, const char *file_path);
extern void load_program_from_file(CPU *c, const char *file_path);
extern void load_program_to_cpu(CPU *c, Object *program, size_t program_size);

extern char *reg_as_cstr(uint64_t operand);
extern char *inst_as_cstr(Inst inst);

extern int inst_has_2_regs(Inst inst);
extern int inst_has_no_ops(Inst inst);
extern int inst_has_1_op(Inst inst);

#endif // CPU_H_