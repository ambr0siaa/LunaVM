#ifndef COMMON_H_
#define COMMON_H_

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "table.h"

#define LUNA_API extern
#define LUNA_FUNC static inline
#define LUNA_MAGIC 0x4e4c
#define LUNA_TAB   "  "

#define LUNA_STATUS_OK  1
#define LUNA_STATUS_HLT 2
#define LUNA_STATUS_DFR 3
#define LUNA_STATUS_ERR 0

#define luna_staterr(L) (L->status == LUNA_STATUS_ERR)
#define luna_stathlt(L) (L->status == LUNA_STATUS_HLT)
#define luna_badcase(L) do if (luna_staterr((L))) goto defer; while(0)

#define luna_assert(x) assert(x)

#define LUNA_REGS 10
#define LABEL_MAP_INIT_CAP 256

typedef unsigned char  u8;
typedef signed char    i8;
typedef unsigned short u16;
typedef signed short   i16;
typedef unsigned int   u32;
typedef signed int     i32;
typedef unsigned long  u64;
typedef signed long    i64;

typedef union {
    u8     as_u8;
    i8     as_i8;
    u16    as_u16;
    i16    as_i16;
    u32    as_u32;
    i32    as_i32;
    u64    as_u64;
    i64    as_i64;
    char   as_ch;
    float  as_f32;
    double as_f64;
    void*  as_ptr;
} LValue;

typedef enum {
    OBJ_I8=0, OBJ_U8, OBJ_CH, OBJ_I16,
    OBJ_U16, OBJ_I32, OBJ_U32, OBJ_F32,
    OBJ_I64,  OBJ_U64,  OBJ_F64, OBJ_EMPTY,
    AMOUNT_OF_OBJ
} LObject_Type;

typedef struct {
    LObject_Type t;
    LValue v;
} LObject;

#define OBJECT_CH(val)  (LObject) {.t = OBJ_CH,  .v = (LValue) {.as_ch = (val)}}
#define OBJECT_I8(val)  (LObject) {.t = OBJ_I8,  .v = (LValue) {.as_i8 = (val)}}
#define OBJECT_U8(val)  (LObject) {.t = OBJ_U8,  .v = (LValue) {.as_u8 = (val)}}
#define OBJECT_I16(val) (LObject) {.t = OBJ_I16, .v = (LValue) {.as_i16 = (val)}}
#define OBJECT_U16(val) (LObject) {.t = OBJ_U16, .v = (LValue) {.as_u16 = (val)}}
#define OBJECT_I32(val) (LObject) {.t = OBJ_I32, .v = (LValue) {.as_i32 = (val)}}
#define OBJECT_U32(val) (LObject) {.t = OBJ_U32, .v = (LValue) {.as_u32 = (val)}}
#define OBJECT_I64(val) (LObject) {.t = OBJ_I64, .v = (LValue) {.as_i64 = (val)}}
#define OBJECT_U64(val) (LObject) {.t = OBJ_U64, .v = (LValue) {.as_u64 = (val)}}
#define OBJECT_F32(val) (LObject) {.t = OBJ_F32, .v = (LValue) {.as_f32 = (val)}}
#define OBJECT_F64(val) (LObject) {.t = OBJ_F64, .v = (LValue) {.as_f64 = (val)}}
#define OBJECT_EMPTY    (LObject) {.t = OBJ_EMPTY}

LUNA_API const size_t type_sizes[AMOUNT_OF_OBJ];

typedef u8 Code;
typedef u16 Instruction;
typedef LValue Register;

typedef enum {
    INST_MOV=1, INST_ADD, INST_SUB,  INST_MUL, INST_DIV,  INST_MOD,
    INST_AND,   INST_NOT, INST_XOR,  INST_SHL, INST_SHR,  INST_OR,
    INST_PUSH,  INST_POP, INST_CALL, INST_RET, INST_JMP,  INST_JNZ,
    INST_JZ,    INST_CMP, INST_GE,   INST_GT,  INST_LT,   INST_LE,
    INST_HLT,   INST_DBR, INST_VLAD, INST_ALC, INST_PULL, IC
} IOpcode;

#define IARG_VAL 0
#define IARG_REG 1

#define ISEG_STACK   0
#define ISEG_STMTIC  1
#define ISEG_DYNAMIC 2

#define IVAL_PTR   0x8
#define IVAL_LABEL 0x4

#define iget_opcode(i) ((i) & 0x3f)
#define iget_label(i)  (((i) & 0x4000) >> 0xe)
#define iget_mode(i)   (((i) & 0x3c0) >> 0x6)
#define iget_arg1(i)   (((i) & 0x400) >> 0xa)
#define iget_arg2(i)   (((i) & 0x800) >> 0xb)
#define iget_seg(i)    (((i) & 0x3000) >> 0xc)
#define iget_ptr(i)    (((i) & 0x8000) >> 0xf)

#define ipush_opcode(i, op) ((i) | (op))
#define ipush_arg(i, a, o)  ((i) | ((a) << (0xa + (o))))
#define ipush_mode(i, m)    ((i) | ((m) << 0x6))
#define ipush_loc(i, l)     ((i) | ((l) << 0xc))

#define definst(opcode, mode, arg1, arg2, loc) \
(u16)(((loc) << 0xc) | ((((arg2) << 0x1) | (arg1)) << 0xa) | ((mode) << 0x6) | (opcode))

LUNA_API const char *inst_names[IC];
/*
* Instruction : [opcode] [args] [location] [arg type]                           :: 16 bit
* Opcode      : enumeration `IOpcode`                                           :: 5  bit
* Args        : Types of args for instruction. They maybe as Value or Register  :: 2  bit
* Inst mode   : <i,u>16, <i/u>8, <i/u>32, <i/u>64, float, double, char          :: 4  bit
* Location    : ptr | label | memmory segment <stack, static, dynamic>          :: 4  bit
*
* Marking:  [ptr][label][segment][arg2][arg1][type][opcode]
*           1bit  1bit    2bit    1bit  1bit  4bit   6bit
*
* If `type` > 11 than Instruction is `Divert` working of
* next Instruction to some container (pointer or regiseter). The main diffrence between `Default` and `Divert` Instruction
* is that bits of `opcode` replace by type of destination 
* value. Well, this means that bits of `type` moving to
* place of `opcode`. Next 4/8 bytes this index to where pushing
* `Divert` value and after `Divert` instrucion placing
* `Default` instruction that value is `Divert`
*/

typedef struct {
    size_t row, col;
    const char *file;
} Location;

typedef enum {
    EXPR_NONE = 0, EXPR_INT, EXPR_UINT,
    EXPR_FLT, EXPR_STR, EXPR_REG,
} Expr_Type;

typedef union {
    i64 i;
    u64 u;
    double f;
    String_View s;
} Expr_Value;

typedef struct {
    Expr_Type t;
    Expr_Value v;
} Expr;

#define EXPR_EMPTY (Expr) {0}

typedef struct {
    size_t capacity;
    size_t count;
    Expr **items; 
} StmtArgs;

typedef struct {
    u32 idx;
    u8 mode;
    hash_t hash; /* for map */
    Expr_Value v;
    Location loc;
    String_View name;
} Label;

typedef struct {
    u32 index; /* Unique identifier of label */
    size_t capacity;
    size_t scaler;
    size_t count;
    Label *items;
} Label_Map;

typedef struct {
    u8 section;
    Label l;
} Stmt_Label;

typedef struct {
    u8 mode;
    IOpcode opcode;
    StmtArgs args;
} Stmt_Inst;

typedef union {
    Stmt_Inst  inst;
    Stmt_Label label;
} Stmt_Value;

typedef enum {
    STMT_NONE = 0,
    STMT_INST,
    STMT_LABEL,
} Stmt_Type;

typedef struct {
    Stmt_Type t;
    Stmt_Value v;
    Location loc;
} Statement;

struct backpatch {
    String_View name;
    size_t addr;
};

typedef struct {
    size_t count;
    size_t capacity;
    struct backpatch *items;
} Backpatch;


typedef struct {
    u8 zf; /* Zero flag */
    u8 debug; /* Debug mode */
    i64 limit; /* limit of execution */
    size_t sc, csc; /* Capacity of stack and callstack */
    size_t ip, sp, fp, csp; /* Control registers */
    Register regs[LUNA_REGS]; /* Data registers */
    Code *stack; /* Memmory segment */
    u64 *callstack; /* Saved callinfo */
} Core;

typedef struct {
    size_t entry; /* Entry point */
    size_t pc, ps; /* Program size and capacity */
    u8 status; /* Current translation status */
    Core core; /* Core of virtual machine */
    Table opcodes; /* Table of opcodes for instructions*/
    Label_Map lmap; /* Map with current labels */
    String_View src; /* Read file */
    const char *file; /* File path */
    Backpatch *bp; /* Unresolved jumping labels */
    Statement *s; /* Current parsing statement */
    Code *code; /* Translated code */
} Luna;

typedef enum {
    EXCP_LEXICAL,
    EXCP_PARSING,
    EXCP_TRANSLATE,
    EXCP_PROGRAM
} Excp_Level;

LUNA_API void luna_report(const char *fmt, ...);
LUNA_API void luna_excp(Excp_Level level, Location *loc, const char *fmt, ...);
LUNA_API void inst_info(Instruction inst);

/* NOTE: Stolen from https://stackoverflow.com/a/3312896 */
#if defined(__GNUC__) || defined(__clang__)
#  define PACKED( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
#  define PACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
#  error "Packed attributes for struct is not implemented for this compiler. This may result in a program working incorrectly"
#endif
PACKED(struct Luna_FileMeta {
    size_t magic;
    size_t entry;
    size_t program_size;
});

typedef struct Luna_FileMeta Luna_FileMeta;

#endif /* COMMON_H_ */
