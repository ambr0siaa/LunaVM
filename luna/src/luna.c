#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "luna.h"

#define IP_INC_TRUE 1
#define IP_INC_FLASE 0

Inst inst_defs[IC][NUMBER_OF_KINDS] = {
    [INST_MOV]  = { [KIND_REG_REG] = INST_MOV_RR, [KIND_REG_VAL] = INST_MOV_RV, [KIND_VAL] = INST_MOVS   },
    [INST_RET]  = { [KIND_REG_REG] = INST_RET_RR, [KIND_REG_VAL] = INST_RET_RV, [KIND_NONE] = INST_RET_N },
    [INST_ADD]  = { [KIND_REG_REG] = INST_ADD_RR, [KIND_REG_VAL] = INST_ADD_RV },
    [INST_SUB]  = { [KIND_REG_REG] = INST_SUB_RR, [KIND_REG_VAL] = INST_SUB_RV },
    [INST_MUL]  = { [KIND_REG_REG] = INST_MUL_RR, [KIND_REG_VAL] = INST_MUL_RV },
    [INST_DIV]  = { [KIND_REG_REG] = INST_DIV_RR, [KIND_REG_VAL] = INST_DIV_RV },
    [INST_AND]  = { [KIND_REG_REG] = INST_AND_RR, [KIND_REG_VAL] = INST_AND_RV },
    [INST_OR]   = { [KIND_REG_REG] = INST_OR_RR , [KIND_REG_VAL] = INST_OR_RV  },
    [INST_XOR]  = { [KIND_REG_REG] = INST_XOR_RR, [KIND_REG_VAL] = INST_XOR_RV },
    [INST_SHR]  = { [KIND_REG_REG] = INST_SHR_RR, [KIND_REG_VAL] = INST_SHR_RV },
    [INST_SHL]  = { [KIND_REG_REG] = INST_SHL_RR, [KIND_REG_VAL] = INST_SHL_RV },
    [INST_PUSH] = { [KIND_VAL]     = INST_PUSH_V, [KIND_REG]     = INST_PUSH_R },
    [INST_POP]  = { [KIND_REG]     = INST_POP_R,  [KIND_NONE]    = INST_POP_N  },
    [INST_DBR]  = { [KIND_REG]     = INST_DBR  },
    [INST_NOT]  = { [KIND_REG]     = INST_NOT  },
    [INST_CALL] = { [KIND_VAL]     = INST_CALL },
    [INST_JMP]  = { [KIND_VAL]     = INST_JMP  },
    [INST_JNZ]  = { [KIND_VAL]     = INST_JNZ  },
    [INST_JZ]   = { [KIND_VAL]     = INST_JZ   },
    [INST_HLT]  = { [KIND_NONE]    = INST_HLT  },
    [INST_VLAD] = { [KIND_NONE]    = INST_VLAD },
    [INST_CMP]  = { [KIND_REG_REG] = INST_CMP  }
};

#define AREFMETIC_OP(c, type, op1, op2, operator, dst) \
    do {                                               \
        (c)->regs##type [dst] = (op1) operator (op2);  \
        (c)->ip += 1;                                  \
    } while (0)

#define LUNA_OP(c, place, op, index, on) \
    do {                                \
        (c)->place [(index)] = op;      \
        if (on) (c)->ip += 1;           \
    } while(0)

Object luna_fetch(Luna *const L)
{
    L->ip += 1;
    return L->program[L->ip];
}

void luna_inst_return(Luna *L)
{
    uint64_t tmp = L->fp;
    L->ip = L->stack[L->fp - 1].u64;
    L->sp = L->stack[L->fp - 2].u64;
    L->fp = L->stack[L->fp - 3].u64;
    L->zero_flag = L->stack[L->fp - 4].u64;

    tmp -= 4;
    
    for (int i = ACCF; i >= F0; --i) {
        int t = i - LUNA_REGS;
        L->regsf[t] = L->stack[--tmp].f64;
    }

    for (int i = ACC; i >= R0; --i) {
        L->regs[i] = L->stack[--tmp].i64;
    }
}

void luna_execute_inst(Luna *const L)
{
    if (L->ip >= L->program_size) {
        fprintf(stderr, "Error: access denied\n");
        exit(1);
    }

    uint64_t ip = L->ip;
    Inst inst = L->program[ip].inst;

    Register reg1;
    Register reg2;
    Object operand1;

    switch (inst) {
        case INST_MOV_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0) LUNA_OP(L, regsf, L->regsf[reg2 - LUNA_REGS], reg1 - LUNA_REGS, IP_INC_TRUE);
            else LUNA_OP(L, regs, L->regs[reg2], reg1, IP_INC_TRUE);

            break;

        case INST_MOV_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) LUNA_OP(L, regsf, operand1.f64, reg1 - LUNA_REGS, IP_INC_TRUE);
            else LUNA_OP(L, regs, operand1.i64, reg1, IP_INC_TRUE);

            break;

        case INST_MOVS:
            reg1 = luna_fetch(L).reg;
            size_t shift = luna_fetch(L).i64 + LUNA_STACK_FRAME_SIZE;

            Object stack_value = L->stack[L->stack_size - 1 - shift];

            if (reg1 >= F0) LUNA_OP(L, regsf, stack_value.f64, reg1 - LUNA_REGS, IP_INC_TRUE);
            else LUNA_OP(L, regs, stack_value.i64, reg1, IP_INC_TRUE);
            break;

        case INST_ADD_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0) AREFMETIC_OP(L, f, L->regsf[reg1 - LUNA_REGS], L->regsf[reg2 - LUNA_REGS], +, reg1 - LUNA_REGS);
            else AREFMETIC_OP(L, ,L->regs[reg1], L->regs[reg2], +, reg1);

            break;

        case INST_SUB_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0) AREFMETIC_OP(L, f, L->regsf[reg1 - LUNA_REGS], L->regsf[reg2 - LUNA_REGS], -, reg1 - LUNA_REGS);
            else AREFMETIC_OP(L, ,L->regs[reg1], L->regs[reg2], -, reg1);

            break;

        case INST_DIV_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0) AREFMETIC_OP(L, f, L->regsf[reg1 - LUNA_REGS], L->regsf[reg2 - LUNA_REGS], /, reg1 - LUNA_REGS);
            else AREFMETIC_OP(L, ,L->regs[reg1], L->regs[reg2], /, reg1);

            break;

        case INST_MUL_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0) AREFMETIC_OP(L, f, L->regsf[reg1 - LUNA_REGS], L->regsf[reg2 - LUNA_REGS], *, reg1 - LUNA_REGS);
            else AREFMETIC_OP(L, ,L->regs[reg1], L->regs[reg2], *, reg1);

            break;

        case INST_ADD_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) AREFMETIC_OP(L, f, L->regsf[reg1 - LUNA_REGS], operand1.f64, +, reg1 - LUNA_REGS);
            else AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, +, reg1);

            break;

        case INST_SUB_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) AREFMETIC_OP(L, f, L->regsf[reg1 - LUNA_REGS], operand1.f64, -, reg1 - LUNA_REGS);
            else AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, -, reg1);

            break;

        case INST_DIV_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) AREFMETIC_OP(L, f, L->regsf[reg1 - LUNA_REGS], operand1.f64, /, reg1 - LUNA_REGS);
            else AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, /, reg1);

            break;

        case INST_MUL_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) AREFMETIC_OP(L, f, L->regsf[reg1 - LUNA_REGS], operand1.f64, *, reg1 - LUNA_REGS);
            else AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, *, reg1);

            break;

        case INST_AND_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, &, reg1);
            break;

        case INST_OR_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, |, reg1);
            break;

        case INST_NOT:
            reg1 = luna_fetch(L).reg;

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            L->regs[reg1] = ~L->regs[reg1];
            L->ip += 1;
            break;

        case INST_XOR_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }
            
            AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, ^, reg1);
            break;

        case INST_SHR_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, >>, reg1);
            break;

        case INST_SHL_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], operand1.i64, <<, reg1);
            break;
        
        case INST_AND_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], L->regs[reg2], &, reg1);
            break;

        case INST_OR_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], L->regs[reg2], |, reg1);
            break;

        case INST_XOR_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], L->regs[reg2], ^, reg1);
            break;

        case INST_SHR_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], L->regs[reg2], >>, reg1);
            break;

        case INST_SHL_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(L, , L->regs[reg1], L->regs[reg2], <<, reg1);
            break;

        case INST_JMP:
            operand1.u64 = luna_fetch(L).u64;
            L->ip = operand1.u64;
            break;

        case INST_JNZ:
            operand1 = luna_fetch(L);
            if (L->zero_flag == 1) L->ip = operand1.u64;
            else L->ip += 1;
            break;

        case INST_CMP:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;
            
            if (reg1 >= F0 && reg2 >= F0) {
                L->zero_flag = L->regsf[reg1 - LUNA_REGS] == L->regsf[reg2 - LUNA_REGS];
            } else if (reg1 < F0 && reg1 < F0) {
                L->zero_flag = L->regs[reg1] == L->regs[reg2];
            } else {
                fprintf(stderr, "Error: in inst `cmp` registers must be with equal types\n");
                exit(1);
            }

            L->ip += 1;
            break;

        case INST_DBR:
            reg1 = luna_fetch(L).reg;
            printf("%s: ", reg_as_cstr(reg1));

            if (reg1 >= F0) printf("%lf\n",L->regsf[reg1 - LUNA_REGS]);
            else printf("%"PRIi64"\n", L->regs[reg1]);

            L->ip += 1;
            break;

        case INST_HLT:
            L->halt = 1;
            break;

        case INST_JZ:
            operand1 = luna_fetch(L);

            if (L->zero_flag == 0) L->ip = operand1.u64;
            else L->ip += 1;
            break;

        case INST_PUSH_V:
            if (L->stack_size > LUNA_STACK_CAPACITY) {
                fprintf(stderr, "Error: stack overflow\n");
                exit(1);
            }

            operand1 = luna_fetch(L);
            LUNA_OP(L, stack, operand1, L->stack_size, IP_INC_TRUE);
            L->stack_size++;
            L->sp++;
            break;

        case INST_PUSH_R:
            if (L->stack_size > LUNA_STACK_CAPACITY) {
                fprintf(stderr, "Error: stack overflow\n");
                exit(1);
            }

            reg1 = luna_fetch(L).reg;

            if (reg1 >= F0) operand1 = OBJ_FLOAT(L->regsf[reg1 - LUNA_REGS]);
            else operand1 = OBJ_INT(L->regs[reg1]);

            LUNA_OP(L, stack, operand1, L->stack_size++, IP_INC_TRUE);
            L->sp++;
            break;

        case INST_POP_R:
            if (L->stack_size < 1) {
                fprintf(stderr, "Error: stack underflow\n");
                exit(1);
            }

            reg1 = luna_fetch(L).reg;
            
            if (reg1 >= F0) LUNA_OP(L, regsf, L->stack[L->stack_size - 1].f64, reg1 - LUNA_REGS, IP_INC_TRUE);
            else LUNA_OP(L, regs, L->stack[L->stack_size - 1].i64, reg1, IP_INC_TRUE);

            L->stack_size -= 1;
            L->sp -= 1;
            break;

        case INST_POP_N:
            if (L->stack_size < 1) {
                fprintf(stderr, "Error: stack underflow\n");
                exit(1);
            }

            L->stack_size -= 1;
            L->sp -= 1;
            L->ip += 1;
            break;

        case INST_CALL:
            operand1 = luna_fetch(L);

            for (int i = R0; i < ACC + 1; ++i) {
                LUNA_OP(L, stack, OBJ_INT(L->regs[i]), L->stack_size++, IP_INC_FLASE);
            }

            for (int i = F0; i < ACCF + 1; ++i) {
                LUNA_OP(L, stack, OBJ_FLOAT(L->regsf[i - LUNA_REGS]), L->stack_size++, IP_INC_FLASE);
            }

            LUNA_OP(L, stack, OBJ_UINT(L->zero_flag), L->stack_size++, IP_INC_FLASE);
            LUNA_OP(L, stack, OBJ_UINT(L->fp), L->stack_size++, IP_INC_FLASE);
            LUNA_OP(L, stack, OBJ_UINT(L->sp), L->stack_size++, IP_INC_FLASE);
            LUNA_OP(L, stack, OBJ_UINT(L->ip), L->stack_size++, IP_INC_FLASE);

            L->sp += LUNA_STACK_FRAME_SIZE;
            L->fp = L->sp;
            L->ip = operand1.u64;
            break;

        case INST_RET_N:
            luna_inst_return(L);
            L->ip += 1;
            break;

        case INST_RET_RR:
            reg1 = luna_fetch(L).reg;
            reg2 = luna_fetch(L).reg;

            if (reg2 >= F0) operand1.f64 = L->regsf[reg2 - LUNA_REGS];
            else operand1.i64 = L->regs[reg2];

            luna_inst_return(L);

            if (reg1 >= F0) LUNA_OP(L, regsf, operand1.f64, reg1 - LUNA_REGS, IP_INC_TRUE);
            else LUNA_OP(L, regs, operand1.i64, reg1, IP_INC_TRUE);
            break;

        case INST_RET_RV:
            reg1 = luna_fetch(L).reg;
            operand1 = luna_fetch(L);

            luna_inst_return(L);

            if (reg1 >= F0) LUNA_OP(L, regsf, operand1.f64, reg1 - LUNA_REGS, IP_INC_TRUE);
            else LUNA_OP(L, regs, operand1.i64, reg1, IP_INC_TRUE);
            break;

        case INST_VLAD:
            printf("Vlad eat's poop!\n");
            L->ip += 1;
            break;

        case IC:
        default:
            fprintf(stderr, "Error: undefine instruction `%s`\n", inst_as_cstr(inst));
            exit(1);
    }
}

void luna_execute_program(Luna *const L, int debug, int limit, int stk)
{
    size_t ulimit = (size_t)(limit);
    for (size_t i = 0; L->halt == 0; ++i) {
        if (i > ulimit) break;
        if (debug) debug_regs(L);
        if (stk) debug_stack(L);
        luna_execute_inst(L);
    }
}

char *inst_as_cstr(Inst inst)
{
    switch (inst) {
        case INST_ADD:    return "add";
        case INST_SUB:    return "sub";
        case INST_DIV:    return "div";
        case INST_MUL:    return "mul";

        case INST_ADD_RR: return "addr";
        case INST_SUB_RR: return "subr";
        case INST_DIV_RR: return "divr";
        case INST_MUL_RR: return "mulr";

        case INST_ADD_RV: return "addv";
        case INST_SUB_RV: return "subv";
        case INST_DIV_RV: return "divv";
        case INST_MUL_RV: return "mulv";

        case INST_AND:    return "and";
        case INST_OR:     return "or";
        case INST_NOT:    return "not";
        case INST_XOR:    return "xor";
        case INST_SHR:    return "shr";
        case INST_SHL:    return "shl";

        case INST_AND_RR: return "andr";
        case INST_OR_RR:  return "orr";
        case INST_XOR_RR: return "xorr";
        case INST_SHR_RR: return "shrr";
        case INST_SHL_RR: return "shlr";

        case INST_AND_RV: return "andv";
        case INST_OR_RV:  return "orv";
        case INST_XOR_RV: return "xorv";
        case INST_SHR_RV: return "shrv";
        case INST_SHL_RV: return "shlv";

        case INST_MOV:    return "mov";
        case INST_MOVS:   return "movs";
        case INST_MOV_RR: return "movr";
        case INST_MOV_RV: return "movv";
        
        case INST_HLT:    return "hlt";
        case INST_DBR:    return "dbr";

        case INST_CMP:    return "cmp";
        case INST_JMP:    return "jmp";
        case INST_JNZ:    return "jnz";
        case INST_JZ:     return "jz";
        
        case INST_POP:    return "pop";
        case INST_POP_R:  return "popr";
        case INST_POP_N:  return "popn";

        case INST_PUSH:   return "push";
        case INST_PUSH_R: return "pshr";
        case INST_PUSH_V: return "pshv";
        
        case INST_CALL:   return "call";

        case INST_RET:    return "ret";
        case INST_RET_N:  return "retn";
        case INST_RET_RR: return "retrr";
        case INST_RET_RV: return "retrv";

        case INST_VLAD:   return "vlad";
        case IC:        
        default: {
            fprintf(stderr, "Error: `%u` this is not a inst\n", inst);
            exit(1);
        }
    }
}

char *reg_as_cstr(uint64_t operand)
{
    switch (operand) {
        case R0:    return  "r0";
        case R1:    return  "r1";
        case R2:    return  "r2";
        case R3:    return  "r3";
        case R4:    return  "r4";
        case R5:    return  "r5";
        case R6:    return  "r6";
        case R7:    return  "r7";
        case R8:    return  "r8";
        case ACC:   return "acc";

        case F0:    return  "f0";
        case F1:    return  "f1";
        case F2:    return  "f2";
        case F3:    return  "f3";
        case F4:    return  "f4";
        case F5:    return  "f5";
        case F6:    return  "f6";
        case F7:    return  "f7";
        case F8:    return  "f8";
        case ACCF:  return "accf";

        case RC:
        default:
            fprintf(stderr, "Error: unreachable reg `%"PRIi64"`\n", operand);
            exit(1);
    }
}

int inst_has_2_regs(Inst inst)
{
    switch (inst) {
        case INST_MOV_RR:   return 1;
        case INST_CMP:    return 1;
        case INST_RET_RR: return 1;
        case INST_ADD_RR: return 1;
        case INST_SUB_RR: return 1;
        case INST_DIV_RR: return 1;
        case INST_MUL_RR: return 1;
        default:          return 0;
    }
}

int inst_has_no_ops(Inst inst)
{
    switch (inst) {
        case INST_HLT:  return 1;
        case INST_RET_N:  return 1;
        case INST_VLAD: return 1;
        default:        return 0;
    }
}

int inst_isjump(Inst inst)
{
    if (inst >= INST_CALL && inst <= INST_JZ)
        return 1;
    return 0;
}

int inst_has_1_op(Inst inst)
{
    switch (inst) {
        case INST_JMP:      return 1;
        case INST_JNZ:      return 1;
        case INST_DBR:      return 1;
        case INST_JZ:       return 1;
        case INST_POP:      return 1;
        case INST_PUSH_R:   return 1;
        case INST_PUSH_V:   return 1;
        case INST_CALL:     return 1;
        default:            return 0;
    }
}

void debug_regs(Luna *const L)
{
    printf("ip: %"PRIu64"\n", L->ip);
    printf("sp: %"PRIu64"\n", L->sp);
    printf("fp: %"PRIu64"\n", L->fp);
    for (size_t i = 0; i < RC; ++i) {
        if (i >= F0) printf("%s: %lf\n", reg_as_cstr(i), L->regsf[i - LUNA_REGS]); 
        else printf("%s: %"PRIi64"\n", reg_as_cstr(i), L->regs[i]);
    }
    printf("zero flag: %d\n", L->zero_flag);
    printf("halt: %d\n", L->halt);
    printf("\n");
}

// TODO: rework stack debug
void debug_stack(Luna *const L)
{
    printf("Stack:\n");
    if (L->stack_size == 0) printf("    empty\n");
    for (size_t i = 0; i < L->stack_size; ++i) {
        printf("    i64: %"PRIi64", u64: %"PRIu64", f64: %lf\n",
                L->stack[i].i64, L->stack[i].u64, L->stack[i].f64);
    }
}

void load_program_to_luna(Luna *L, Object *program, size_t program_size)
{
    size_t all_size = sizeof(program[0]) * program_size;
    memcpy(L->program, program, all_size);
    L->program_size += program_size;
}   

void load_program_from_file(Arena *arena, Luna *L, const char *file_path)
{
    if (L->program_capacity == 0) {
        L->program_capacity = LUNA_PROGRAM_INIT_CAPACITY;
        L->program = arena_alloc(arena, L->program_capacity * sizeof(*L->program));
    }

    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file by `%s` path: %s\n",
            file_path, strerror(errno));
        exit(1);
    }

    Luna_File_Meta meta = {0};
    size_t n = fread(&meta, sizeof(meta), 1, fp);
    if (n < 1) {
        fprintf(stderr, "Error: cannot open file by `%s` path: %s\n",
            file_path, strerror(errno));
        exit(1);
    }

    if (meta.magic != LUNA_MAGIC) {
        fprintf(stderr, "Error: not a Luna Bytecode Format - ln\n");
        fprintf(stderr, "Expected 0x%04x but provided 0x%04lx",
                LUNA_MAGIC, meta.magic);
        exit(1);
    }


    L->program_size = fread(L->program, sizeof(*L->program), meta.program_size, fp);
    if (meta.program_size != L->program_size) {
        fprintf(stderr, "Error: expected %"PRIi64" program size reading %"PRIi64"",
                meta.program_size, L->program_size);
        exit(1);
    }

    L->ip = meta.entry;

    fclose(fp);
}

void luna_clean_program(Luna *const L)
{
    free(L->program);
    L->program = NULL;
    L->program_capacity = 0;
    L->program_size = 0;
}

char *luna_shift_args(int *argc, char ***argv)
{
    if (*argc <= 0) {
        fprintf(stderr, "Error: not enough args for `luna_shift_args`\n");
        exit(1);
    }

    char *result = **argv;

    *argv += 1;
    *argc -= 1;

    return result;
}
