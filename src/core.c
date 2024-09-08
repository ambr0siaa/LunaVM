#include <stdlib.h>
#include <string.h>

#include "core.h"


#define STACK_CAPACITY 32
#define CALLSTACK_CAPACITY 16
#define CALLSTACK_LIMIT 65536
#define STACK_LIMIT 4294967296

void luna_core_init(Arena *a, Luna *L)
{
    L->core.ip = L->entry;
    L->core.sc = STACK_CAPACITY;
    L->core.csc = CALLSTACK_CAPACITY;
    L->core.fp = L->core.sp = L->core.csp = L->core.zf = 0;
    L->core.stack = arena_alloc(a, sizeof(u8)*L->core.sc);
    L->core.callstack = arena_alloc(a, sizeof(u64)*L->core.csc);
}

static inline void luna_core_excp(const char *excp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Core Exception : %s\n"LUNA_TAB, excp);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

#define luna_fetch(L, size, dst)                        \
    do {                                                \
        luna_assert((L)->ps != 0);                      \
        memcpy(dst, (L)->code + (L)->core.ip, size);    \
        (L)->core.ip += size;                           \
    } while(0)

#define luna_binOp_all(op, v1, v2, mode)                                                                    \
    do {                                                                                                    \
        switch (mode) {                                                                                     \
            case OBJ_I8:  (v1)->as_i8  = (v1)->as_i8  op (v2)->as_i8;  break;                               \
            case OBJ_U8:  (v1)->as_u8  = (v1)->as_u8  op (v2)->as_u8;  break;                               \
            case OBJ_CH:  (v1)->as_ch  = (v1)->as_ch  op (v2)->as_ch;  break;                               \
            case OBJ_I16: (v1)->as_i16 = (v1)->as_i16 op (v2)->as_i16; break;                               \
            case OBJ_U16: (v1)->as_u16 = (v1)->as_u16 op (v2)->as_u16; break;                               \
            case OBJ_I32: (v1)->as_i32 = (v1)->as_i32 op (v2)->as_i32; break;                               \
            case OBJ_U32: (v1)->as_u32 = (v1)->as_u32 op (v2)->as_u32; break;                               \
            case OBJ_I64: (v1)->as_i64 = (v1)->as_i64 op (v2)->as_i64; break;                               \
            case OBJ_U64: (v1)->as_u64 = (v1)->as_u64 op (v2)->as_u64; break;                               \
            case OBJ_F32: (v1)->as_f32 = (v1)->as_f32 op (v2)->as_f32; break;                               \
            case OBJ_F64: (v1)->as_f64 = (v1)->as_f64 op (v2)->as_f64; break;                               \
            default: {                                                                                      \
                luna_core_excp("Binary operation", "At byte 0x%LX -> unreachable mode %u", L->core.ip, mode);    \
                L->status = LUNA_STATUS_ERR;                                                                \
            }                                                                                               \
        }                                                                                                   \
    } while(0)

#define binaryOp_body(func, op)                                                  \
        if (arg2 == IARG_REG) {                                                 \
            reg_t r2 = luna_fetchR((L));                                        \
            func(op, &(L)->core.regs[r1], &(L)->core.regs[r2], mode); \
        } else {                                                                \
            LObject val = luna_fetchO((L), mode);                               \
            func(op, &(L)->core.regs[r1], &val.v, mode);              \
        }                                                                       \

#define luna_binaryOp(L, op)                                                    \
    do {                                                                        \
        luna_assert(arg1 == IARG_REG);                                          \
        reg_t r1 = luna_fetchR((L));                                            \
        binaryOp_body(luna_binOp_all, op)                                       \
    } while(0)

#define luna_compareOp(L, op)                                                   \
    do {                                                                        \
        luna_assert(arg1 == IARG_REG);                                          \
        reg_t r1 = luna_fetchR((L));                                            \
        Register buf = (L)->core.regs[r1];                                      \
        binaryOp_body(luna_binOp_all, op);                                      \
        if (mode == OBJ_F32) {                                                  \
            (L)->core.zf = (u8)(L)->core.regs[r1].as_f32;                       \
        } else if (mode == OBJ_F64) {                                           \
            (L)->core.zf = (u8)(L)->core.regs[r1].as_f64;                       \
        } else {                                                                \
            (L)->core.zf = (L)->core.regs[r1].as_u8;                            \
        }                                                                       \
        (L)->core.regs[r1] = buf;                                               \
        (L)->status = LUNA_STATUS_OK;                                           \
    } while(0)

#define luna_binOp_inttypes(op, v1, v2, mode)                                   \
    do {                                                                        \
        switch (mode) {                                                         \
            case OBJ_I8:  (v1)->as_i8  = (v1)->as_i8  op (v2)->as_i8;  break; \
            case OBJ_U8:  (v1)->as_u8  = (v1)->as_u8  op (v2)->as_u8;  break; \
            case OBJ_CH:  (v1)->as_ch  = (v1)->as_ch  op (v2)->as_ch;  break; \
            case OBJ_I16: (v1)->as_i16 = (v1)->as_i16 op (v2)->as_i16; break; \
            case OBJ_U16: (v1)->as_u16 = (v1)->as_u16 op (v2)->as_u16; break; \
            case OBJ_I32: (v1)->as_i32 = (v1)->as_i32 op (v2)->as_i32; break; \
            case OBJ_U32: (v1)->as_u32 = (v1)->as_u32 op (v2)->as_u32; break; \
            case OBJ_I64: (v1)->as_i64 = (v1)->as_i64 op (v2)->as_i64; break; \
            case OBJ_U64: (v1)->as_u64 = (v1)->as_u64 op (v2)->as_u64; break; \
            default: {                                                        \
                luna_core_excp("Binary operation", "At byte 0x%LX -> cannot use mode %u for only integer operation", L->core.ip, mode);      \
                L->status = LUNA_STATUS_ERR;                                    \
            }                                                                 \
        }                                                                   \
    } while(0)

#define luna_binaryOp_int(L, op)                \
    do {                                        \
        luna_assert(arg1 == IARG_REG);          \
        reg_t r1 = luna_fetchR(L);              \
        binaryOp_body(luna_binOp_inttypes, op); \
    } while(0)


Instruction luna_fetchI(Luna *L)
{
    Instruction i = 0;
    luna_fetch(L, sizeof(u16), &i);
    return i;
}

LObject luna_fetchO(Luna *L, LObject_Type type)
{
    LObject o = { .t = type };
    luna_fetch(L, type_sizes[o.t], &o.v.as_u64);
    return o;
}

reg_t luna_fetchR(Luna *L)
{
    reg_t r = 0;
    luna_fetch(L, sizeof(u8), &r);
    return r;
}

Address luna_fetchA(Luna *L)
{
    Address a = 0;
    luna_fetch(L, sizeof(u64), &a);
    return a;
}

#define callstack_get(L, a) (L)->core.callstack[(a)]

void callstack_push(Arena *a, Core *c, u64 data)
{
    if (c->csp + 1 > c->csc) {
        size_t old_size = sizeof(u64)*c->csc;
        c->csc *= 2;
        luna_assert(c->csc >= CALLSTACK_LIMIT && "Callstack Overflow");
        c->callstack = arena_realloc(a, c->callstack, old_size, c->csc*sizeof(u64));
    }
    c->callstack[c->csp++] = data;
}

void register_dump(Luna *L, reg_t r, u8 mode)
{
    Register reg = L->core.regs[r];
    printf("Regiser%u ", r);
    switch (mode) {
        case OBJ_I8: {
            luna_report("%d", reg.as_i8);
            break;
        } case OBJ_U8: {
            luna_report("%u", reg.as_u8);
            break;
        } case OBJ_CH: {
            luna_report("%c", reg.as_ch);
            break;
        } case OBJ_I16: {
            luna_report("%d", reg.as_i16);
            break;
        } case OBJ_U16: {
            luna_report("%u", reg.as_u16);
            break;
        } case OBJ_I32: {
            luna_report("%i", reg.as_i32);
            break;
        } case OBJ_U32: {
            luna_report("%u", reg.as_u32);
            break;
        } case OBJ_F32: {
            luna_report("%f", reg.as_f32);
            break;
        } case OBJ_I64: {
            luna_report("%li", reg.as_i64);
            break;
        } case OBJ_U64: {
            luna_report("%lu", reg.as_u64);
            break;
        } case OBJ_F64: {
            luna_report("%lf", reg.as_f64);
            break;
        } default: {
            luna_report("What the hell is that register %u???", r);
            L->status = LUNA_STATUS_ERR;
            break;
        }
    }
}

void luna_exec_inst(Arena *a, Luna *L)
{
    if (L->core.ip > L->ps) {
        luna_core_excp("Access denied", "Invalid instruction pointer to byte 0x%LX", L->core.ip);
        L->status = LUNA_STATUS_ERR;
        return;
    }
    Instruction inst = luna_fetchI(L);
    IOpcode opcode = iget_opcode(inst);
    u8 mode = iget_mode(inst);
    u8 arg1 = iget_arg1(inst);
    u8 arg2 = iget_arg2(inst);
    if (L->core.debug) {
        getchar();
        luna_report("State: inst '%s', mode %u, ip %zu", inst_names[opcode], mode, L->core.ip);
    }
    switch (opcode) {
        case INST_MOV: {
            if (arg1 == IARG_REG) {
                reg_t r1 = luna_fetchR(L);
                if (arg2 == IARG_REG) {
                    size_t r2 = luna_fetchR(L);
                    L->core.regs[r1] = L->core.regs[r2];
                } else {
                    LObject val = luna_fetchO(L, mode);
                    L->core.regs[r1] = val.v;
                } 
            } else { 
                luna_assert(0 && "TODO: Pointer stuff not implemented");
            }
            break;
        } case INST_ADD: {
            luna_binaryOp(L, +);
            break;
        } case INST_SUB: {
            luna_binaryOp(L, -);
            break;
        } case INST_MUL: {
            luna_binaryOp(L, *);
            break;
        } case INST_DIV: {
            luna_binaryOp(L, /);
            break;
        } case INST_MOD: {
            luna_binaryOp_int(L, %);
            break;
        } case INST_AND: {
            luna_binaryOp_int(L, &);
            break;
        } case INST_OR: {
            luna_binaryOp_int(L, |);
            break;
        } case INST_XOR: {
            luna_binaryOp_int(L, ^);
            break;
        } case INST_SHL: {
            luna_binaryOp_int(L, <<);
            break;
        } case INST_SHR: {
            luna_binaryOp_int(L, >>);
            break;
        } case INST_JMP: {
            Address addr = luna_fetchA(L);
            L->core.ip = addr;
            break;
        } case INST_JNZ: {
            Address addr = luna_fetchA(L);
            if (L->core.zf) L->core.ip = addr;
            break;
        } case INST_JZ: {
            Address addr = luna_fetchA(L);
            if (!L->core.zf) L->core.ip = addr;
            break;
        } case INST_CALL: {
            Address addr = luna_fetchA(L);
            callstack_push(a, &L->core, L->core.fp);
            callstack_push(a, &L->core, L->core.ip);
            L->core.fp = L->core.csp;
            L->core.ip = addr;
            break;
        } case INST_RET: {
            L->core.ip = callstack_get(L, L->core.fp - 1);
            L->core.fp = callstack_get(L, L->core.fp - 2);
            break;
        } case INST_CMP: {
            luna_compareOp(L, ==);
            break;
        } case INST_GE: {
            luna_compareOp(L, >=);
            break;
        } case INST_GT: {
            luna_compareOp(L, >);
            break;
        } case INST_LT: {
            luna_compareOp(L, <);
            break;
        } case INST_LE: {
            luna_compareOp(L, <=);
            break;
        } case INST_DBR: {
            reg_t r = luna_fetchR(L);
            register_dump(L, r, mode);
            break;
        } case INST_VLAD: {
            luna_report("Vlad eats poop!");
            break;
        } case INST_HLT: {
            L->status = LUNA_STATUS_HLT;
            break;
        } default: {
            luna_core_excp("Access denied", "At byte 0x%LX -> unknown instruction opcode 0x%LX from 0x%LX", L->core.ip - 2, opcode, inst);
            L->status = LUNA_STATUS_ERR;
        }
    }
    if (L->core.debug) {
        luna_report("State next:");
        luna_report("  -> ip %zu, sp %zu, csp %zu, fp %zu, zf %u",
                    L->core.ip, L->core.sp, L->core.csp, L->core.fp, L->core.zf);
        for (size_t i = 0; i < 10; ++i) {
            printf("  -> ");
            register_dump(L, i, mode);
        }
        luna_report("");
    }
}
