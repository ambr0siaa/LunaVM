#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include "sv.h"

#define LUNA_API extern
#define LUNA_MAGIC 0x4e4c

#define ARRAY_SIZE(arr) sizeof((arr)) / sizeof((arr)[0])

typedef enum {
    TK_OPERATOR = 0,
    TK_NUMBER,
    TK_OPEN_BRACKET,
    TK_CLOSE_BRACKET,
    TK_OPEN_PAREN,
    TK_CLOSE_PAREN,
    TK_OPEN_CURLY,
    TK_CLOSE_CURLY,
    TK_SEMICOLON,
    TK_AMPERSAND,
    TK_EQ,
    TK_DOLLAR,
    TK_COLON,
    TK_STRING,
    TK_DOT,
    TK_TEXT,
    TK_COMMA,
    TK_CONST,
    TK_ENTRY,
    TK_NONE
} Token_Type;

typedef struct {
    String_View txt;
    Token_Type type;
    uint32_t location;
    uint32_t line;
} Token;

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

    // DONT CHANGE THIS ORDER OF JUMP INSTS!!!
    INST_CALL,
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
    KIND_REG_REG = 0,
    KIND_REG_VAL,
    KIND_REG,
    KIND_VAL,
    KIND_NONE,
    NUMBER_OF_KINDS
} Inst_Kind;

typedef struct BinaryOp BinaryOp;

typedef enum {
    EXPR_EMPTY = 0,
    EXPR_LIT_INT,
    EXPR_LIT_FLT,
    EXPR_LIT_STR,
    EXPR_BINARY_OP
} Expr_Type;

typedef union {
    double as_flt;
    int64_t as_int;
    String_View as_str;
    BinaryOp *as_binOp;
} Expr_Value;

typedef struct {
    Expr_Type t;
    Expr_Value v;
} Expr;

typedef enum {
    BINOP_EMPTY = 0,
    BINOP_PLUS,
    BINOP_MINUS,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_GT,
    BINOP_GE,
    BINOP_LE,
    BINOP_EQ
} BinOp_Type;

struct BinaryOp {
    BinOp_Type type;
    Expr left;
    Expr right;
};

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

// Abstract representation of all types that can use vm
typedef union {
    Inst inst;
    double f64;
    int64_t i64;
    uint64_t u64;
    Register reg;
} Object;

#define OBJ_INST(type)   (Object) { .inst = (type) }
#define OBJ_FLOAT(val)   (Object) { .f64 = (val) }
#define OBJ_UINT(val)    (Object) { .u64 = (val) }
#define OBJ_INT(val)     (Object) { .i64 = (val) }
#define OBJ_REG(r)       (Object) { .reg = (r) }

#if defined(__GNUC__) || defined(__clang__)
#  define PACKED __attribute__((packed))
#else
#  error "Packed attributes for struct is not implemented for this compiler. This may result in a program working incorrectly. Feel free to fix that and submit a Pull Request to https://github.com/tsoding/bng"
#  define PACKED
#endif

typedef struct {
    size_t magic;
    size_t entry;
    size_t program_size;
} PACKED Luna_File_Meta;

#endif // TYPES_H_
