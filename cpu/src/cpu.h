#ifndef CPU_H_
#define CPU_H_

// TODO: separate instructions stuff to diffrent file 

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "arena.h"

#define ARRAY_SIZE(arr) sizeof((arr)) / sizeof((arr)[0])

#define PROGRAM_INIT_CAPACITY 1024
#define STACK_CAPACITY 16384

// RC   - registers count
// ACC  - accumulator for int
// ACCF - accumulator for float
typedef enum {
    R0 = 0, R1, R2, R3, R4, R5, R6, R7, R8, ACC,
    F0, F1, F2, F3, F4, F5, F6, F7, F8, ACCF,
    RC,
} Register;

typedef enum {
    // inst mov header
    INST_MOV = 0,

    // Kinds:
    INST_MOV_RR,
    INST_MOV_RV,
    INST_MOVS,   // Move value from stack relativly; `$` is option to get args for call from previous stack frame

    INST_HLT,
    INST_DBR, // DeBug Register

    // Arefmetic insts headers
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,

    // Kinds:
    //  For register with registers
    INST_ADD_RR,
    INST_SUB_RR,
    INST_DIV_RR,
    INST_MUL_RR,

    //  For values with registers
    INST_ADD_RV,
    INST_SUB_RV,
    INST_MUL_RV,
    INST_DIV_RV,

    // inst push header
    INST_PUSH,

    // Kinds:
    INST_PUSH_V,
    INST_PUSH_R,

    // inst pop header
    INST_POP,
    
    // Kinds:
    INST_POP_R,
    INST_POP_N,

    INST_CALL,

    // inst ret header
    INST_RET,

    // Kinds:
    INST_RET_N,
    INST_RET_RR,
    INST_RET_RV,

    // INST_INC, 
    // INST_DEC,

    // binwise instruction headers    
    INST_AND,
    INST_OR,
    INST_NOT,
    INST_XOR,
    INST_SHR,
    INST_SHL,

    // Kinds:
    //  For register with register 
    INST_AND_RR,
    INST_OR_RR,
    INST_XOR_RR,
    INST_SHR_RR,
    INST_SHL_RR,

    // For value with registers
    INST_AND_RV,
    INST_OR_RV,
    INST_XOR_RV,
    INST_SHR_RV,
    INST_SHL_RV,

    INST_JMP,
    INST_JNZ,
    INST_JZ,

    // TODO: INST_CMP_REG_REG, INST_CMP_REG_VAL
    INST_CMP,

    // TODO: INST_GRT, INST_GET, INST_LST, INST_LET

    INST_VLAD,
    IC          // IC -> inst count
} Inst;

typedef enum {
    KIND_REG_REG= 0,
    KIND_REG_VAL,
    KIND_REG,
    KIND_VAL,
    KIND_NONE
} Inst_Kind;

// Abstract representation of all types that can use vm
typedef union {
    Inst inst;
    double f64;
    int64_t i64;
    uint64_t u64;
    Register reg;
} Object;

// Count of regs in array on cpu
#define CPU_REGS 10

// Register from enum (without RT and RTF) + registers from CPU (ip, sp, fp, zero_flag)
#define STACK_FRAME_SIZE 24

// kernel of virtual machine
typedef struct {
    int64_t regs[CPU_REGS];
    double regsf[CPU_REGS];

    Object *program;
    uint64_t program_capacity;
    uint64_t program_size;
    uint64_t ip;

    Object stack[STACK_CAPACITY];
    uint64_t stack_size;
    uint64_t sp;
    uint64_t fp;

    uint8_t zero_flag;
    uint8_t halt;
} CPU;


#if defined(__GNUC__) || defined(__clang__)
#  define PACKED __attribute__((packed))
#else
#  warning "Packed attributes for struct is not implemented for this compiler. This may result in a program working incorrectly. Feel free to fix that and submit a Pull Request to https://github.com/tsoding/bng"
#  define PACKED
#endif

#define LUNA_MAGIC 0x4e4c

typedef struct {
    size_t magic;
    size_t entry;
    size_t program_size;
} PACKED Luna_File_Meta;

#define OBJ_INST(type)   (Object) { .inst = (type) }
#define OBJ_FLOAT(val)   (Object) { .f64 = (val) }
#define OBJ_UINT(val)    (Object) { .u64 = (val) }
#define OBJ_INT(val)     (Object) { .i64 = (val) }
#define OBJ_REG(r)       (Object) { .reg = (r) }

#define AREFMETIC_OP(c, type, op1, op2, operator, dst) \
    do {                                               \
        (c)->regs##type [dst] = (op1) operator (op2);  \
        (c)->ip += 1;                                  \
    } while (0)

#define IP_INC_TRUE 1
#define IP_INC_FLASE 0

#define CPU_OP(c, place, op, index, on) \
    do {                                \
        (c)->place [(index)] = op;      \
        if (on) (c)->ip += 1;           \
    } while(0)

int inst_has_1_op(Inst inst);
int inst_has_2_regs(Inst inst);
int inst_has_no_ops(Inst inst);

char *inst_as_cstr(Inst inst);
char *reg_as_cstr(uint64_t operand);
char *luna_shift_args(int *argc, char ***argv);

void cpu_inst_return(CPU *c);
void cpu_set_entry_ip(CPU *c);
Object cpu_fetch(CPU *const c);

void debug_regs(CPU *const c);
void debug_stack(CPU *const c);

void cpu_execute_inst(CPU *const c);
void cpu_clean_program(CPU *const c);
void cpu_execute_program(CPU *const c, int debug, int limit, int stk);

void load_program_from_file(Arena *arena, CPU *c, const char *file_path);
void load_program_to_cpu(CPU *c, Object *program, size_t program_size);

#endif // CPU_H_