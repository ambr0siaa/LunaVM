#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define PROGRAM_INIT_CAPACITY 1024
#define STACK_CAPACITY        16384

// RT   - return register
// RTF  - return float register
// ACC  - accumulator
// ACCF - accumulator for float
// RC   - registers count
typedef enum {
    R0 = 0, R1, R2, R3, R4, R5, R6, R7, R8, ACC,
    F0, F1, F2, F3, F4, F5, F6, F7, F8, ACCF,
    RT, RTF, RC,
} Register;

#define MOVS_OPTION '$'

typedef enum {
    INST_MOV = 0,
    INST_MOVI,      // Move integer to register
    INST_MOVF,      // Move float to register
    INST_MOVS,      // Move value from stack relativly; `$` is option to get args for call from previous stack frame

    INST_HLT,
    INST_DBR,       // DeBug Register

    // Arefmetic insts
    // For integer registers
    INST_ADDI,
    INST_SUBI,
    INST_DIVI,
    INST_MULI,

    // For float registers
    INST_ADDF,
    INST_SUBF,
    INST_DIVF,
    INST_MULF,

    // For values with registers
    INST_ADDV,
    INST_SUBV,
    INST_MULV,
    INST_DIVV,

    INST_PUSH_VAL,
    INST_PUSH_REG,

    INST_POP,
    INST_CALL,
    INST_RET,

    // TODO: INST_INC, INST_DEC
    // INST_AND, INST_OR, INST_NOT, INST_XOR, INST_RSR, INST_RSL

    INST_JMP,
    INST_JNZ,
    INST_JZ,
    INST_CMP,

    INST_VLAD,
    IC          // IC -> inst count
} Inst;

// Abstract representation of all types that can use vm
typedef union {
    Inst inst;
    double f64;
    int64_t i64;
    uint64_t u64;
    Register reg;
} Object;

// Register from enum (without RT and RTF) + registers from CPU (ip, sp, fp, zero_flag)
#define STACK_FRAME_SIZE 24

// kernel of virtual machine
typedef struct {
    int64_t regs[RC];
    double regsf[RC];

    Object *program;
    uint64_t program_capacity;
    uint64_t program_size;
    uint64_t ip;

    Object stack[STACK_CAPACITY];
    uint64_t stack_size;
    uint64_t sp;
    uint64_t fp;

    int zero_flag : 1;
    int halt : 1;
} CPU;

#define OBJ_INST(type) (Object) { .inst = (type) }
#define OBJ_FLOAT(val) (Object) { .f64 = (val) }
#define OBJ_UINT(val)  (Object) { .u64 = (val) }
#define OBJ_REG(r)     (Object) { .reg = (r) }
#define OBJ_INT(val)   (Object) { .i64 = (val) }

extern void debug_regs(CPU *c);
extern void debug_stack(CPU *c);

extern void cpu_execute_inst(CPU *c);
extern void cpu_execute_program(CPU *c, int debug, int limit, int stk);

extern void load_program_to_file(CPU *c, const char *file_path);
extern void load_program_from_file(CPU *c, const char *file_path);
extern void load_program_to_cpu(CPU *c, Object *program, size_t program_size);

extern char *reg_as_cstr(uint64_t operand);
extern char *inst_as_cstr(Inst inst);

extern int inst_has_2_regs(Inst inst);
extern int inst_has_no_ops(Inst inst);
extern int inst_has_1_op(Inst inst);

extern void cpu_clean_program(CPU *c);

#endif // CPU_H_
