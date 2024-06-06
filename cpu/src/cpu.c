#include "./cpu.h"

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

Object cpu_fetch(CPU *const c)
{
    c->ip += 1;
    return c->program[c->ip];
}

void cpu_inst_return(CPU *c)
{
    uint64_t tmp = c->fp;
    c->ip = c->stack[c->fp - 1].u64;
    c->sp = c->stack[c->fp - 2].u64;
    c->fp = c->stack[c->fp - 3].u64;
    c->zero_flag = c->stack[c->fp - 4].u64;

    tmp -= 4;
    
    for (int i = ACCF; i >= F0; --i) {
        int t = i - CPU_REGS;
        c->regsf[t] = c->stack[--tmp].f64;
    }

    for (int i = ACC; i >= R0; --i) {
        c->regs[i] = c->stack[--tmp].i64;
    }
}

void cpu_execute_inst(CPU *const c)
{
    if (c->ip >= c->program_size) {
        fprintf(stderr, "Error: access denied\n");
        exit(1);
    }

    uint64_t ip = c->ip;
    Inst inst = c->program[ip].inst;

    Register reg1;
    Register reg2;
    Object operand1;

    switch (inst) {
        case INST_MOV_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0) CPU_OP(c, regsf, c->regsf[reg2 - CPU_REGS], reg1 - CPU_REGS, IP_INC_TRUE);
            else CPU_OP(c, regs, c->regs[reg2], reg1, IP_INC_TRUE);

            break;

        case INST_MOV_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) CPU_OP(c, regsf, operand1.f64, reg1 - CPU_REGS, IP_INC_TRUE);
            else CPU_OP(c, regs, operand1.i64, reg1, IP_INC_TRUE);

            break;

        case INST_MOVS:
            reg1 = cpu_fetch(c).reg;
            size_t shift = cpu_fetch(c).i64 + STACK_FRAME_SIZE;

            Object stack_value = c->stack[c->stack_size - 1 - shift];

            if (reg1 >= F0) CPU_OP(c, regsf, stack_value.f64, reg1 - CPU_REGS, IP_INC_TRUE);
            else CPU_OP(c, regs, stack_value.i64, reg1, IP_INC_TRUE);
            break;

        case INST_ADD_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0) AREFMETIC_OP(c, f, c->regsf[reg1 - CPU_REGS], c->regsf[reg2 - CPU_REGS], +, reg1 - CPU_REGS);
            else AREFMETIC_OP(c, ,c->regs[reg1], c->regs[reg2], +, reg1);

            break;

        case INST_SUB_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0) AREFMETIC_OP(c, f, c->regsf[reg1 - CPU_REGS], c->regsf[reg2 - CPU_REGS], -, reg1 - CPU_REGS);
            else AREFMETIC_OP(c, ,c->regs[reg1], c->regs[reg2], -, reg1);

            break;

        case INST_DIV_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0) AREFMETIC_OP(c, f, c->regsf[reg1 - CPU_REGS], c->regsf[reg2 - CPU_REGS], /, reg1 - CPU_REGS);
            else AREFMETIC_OP(c, ,c->regs[reg1], c->regs[reg2], /, reg1);

            break;

        case INST_MUL_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0) AREFMETIC_OP(c, f, c->regsf[reg1 - CPU_REGS], c->regsf[reg2 - CPU_REGS], *, reg1 - CPU_REGS);
            else AREFMETIC_OP(c, ,c->regs[reg1], c->regs[reg2], *, reg1);

            break;

        case INST_ADD_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) AREFMETIC_OP(c, f, c->regsf[reg1 - CPU_REGS], operand1.f64, +, reg1 - CPU_REGS);
            else AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, +, reg1);

            break;

        case INST_SUB_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) AREFMETIC_OP(c, f, c->regsf[reg1 - CPU_REGS], operand1.f64, -, reg1 - CPU_REGS);
            else AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, -, reg1);

            break;

        case INST_DIV_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) AREFMETIC_OP(c, f, c->regsf[reg1 - CPU_REGS], operand1.f64, /, reg1 - CPU_REGS);
            else AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, /, reg1);

            break;

        case INST_MUL_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) AREFMETIC_OP(c, f, c->regsf[reg1 - CPU_REGS], operand1.f64, *, reg1 - CPU_REGS);
            else AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, *, reg1);

            break;

        case INST_AND_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, &, reg1);
            break;

        case INST_OR_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, |, reg1);
            break;

        case INST_NOT:
            reg1 = cpu_fetch(c).reg;

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            c->regs[reg1] = ~c->regs[reg1];
            c->ip += 1;
            break;

        case INST_XOR_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }
            
            AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, ^, reg1);
            break;

        case INST_SHR_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, >>, reg1);
            break;

        case INST_SHL_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            if (reg1 >= F0) {
                fprintf(stderr, "Error: for bitwise operation need not float register\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], operand1.i64, <<, reg1);
            break;
        
        case INST_AND_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], c->regs[reg2], &, reg1);
            break;

        case INST_OR_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], c->regs[reg2], |, reg1);
            break;

        case INST_XOR_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], c->regs[reg2], ^, reg1);
            break;

        case INST_SHR_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], c->regs[reg2], >>, reg1);
            break;

        case INST_SHL_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg1 >= F0 || reg2 >= F0) {
                fprintf(stderr, "Error: registers must be non float for bitwise operation\n");
                exit(1);
            }

            AREFMETIC_OP(c, , c->regs[reg1], c->regs[reg2], <<, reg1);
            break;


        case INST_JMP:
            operand1.u64 = cpu_fetch(c).u64;
            c->ip = operand1.u64;
            break;

        case INST_JNZ:
            operand1 = cpu_fetch(c);
            if (c->zero_flag == 1) c->ip = operand1.u64;
            else c->ip += 1;
            break;

        case INST_CMP:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;
            
            if (reg1 >= F0 && reg2 >= F0) {
                c->zero_flag = c->regsf[reg1 - CPU_REGS] == c->regsf[reg2 - CPU_REGS];
            } else if (reg1 < F0 && reg1 < F0) {
                c->zero_flag = c->regs[reg1] == c->regs[reg2];
            } else {
                fprintf(stderr, "Error: in inst `cmp` registers must be with equal types\n");
                exit(1);
            }

            c->ip += 1;
            break;

        case INST_DBR:
            reg1 = cpu_fetch(c).reg;
            printf("%s: ", reg_as_cstr(reg1));

            if (reg1 >= F0) printf("%lf\n",c->regsf[reg1 - CPU_REGS]);
            else printf("%"PRIi64"\n", c->regs[reg1]);

            c->ip += 1;
            break;

        case INST_HLT:
            c->halt = 1;
            break;

        case INST_JZ:
            operand1 = cpu_fetch(c);

            if (c->zero_flag == 0) c->ip = operand1.u64;
            else c->ip += 1;
            break;

        case INST_PUSH_V:
            if (c->stack_size > STACK_CAPACITY) {
                fprintf(stderr, "Error: stack overflow\n");
                exit(1);
            }

            operand1 = cpu_fetch(c);
            CPU_OP(c, stack, operand1, c->stack_size, IP_INC_TRUE);
            c->stack_size++;
            c->sp++;
            break;

        case INST_PUSH_R:
            if (c->stack_size > STACK_CAPACITY) {
                fprintf(stderr, "Error: stack overflow\n");
                exit(1);
            }

            reg1 = cpu_fetch(c).reg;

            if (reg1 >= F0) operand1 = OBJ_FLOAT(c->regsf[reg1 - CPU_REGS]);
            else operand1 = OBJ_INT(c->regs[reg1]);

            CPU_OP(c, stack, operand1, c->stack_size++, IP_INC_TRUE);
            c->sp++;
            break;

        case INST_POP_R:
            if (c->stack_size < 1) {
                fprintf(stderr, "Error: stack underflow\n");
                exit(1);
            }

            reg1 = cpu_fetch(c).reg;
            
            if (reg1 >= F0) CPU_OP(c, regsf, c->stack[c->stack_size - 1].f64, reg1 - CPU_REGS, IP_INC_TRUE);
            else CPU_OP(c, regs, c->stack[c->stack_size - 1].i64, reg1, IP_INC_TRUE);

            c->stack_size -= 1;
            c->sp -= 1;
            break;

        case INST_POP_N:
            if (c->stack_size < 1) {
                fprintf(stderr, "Error: stack underflow\n");
                exit(1);
            }

            c->stack_size -= 1;
            c->sp -= 1;
            c->ip += 1;
            break;

        case INST_CALL:
            operand1 = cpu_fetch(c);

            for (int i = R0; i < ACC + 1; ++i) {
                CPU_OP(c, stack, OBJ_INT(c->regs[i]), c->stack_size++, IP_INC_FLASE);
            }

            for (int i = F0; i < ACCF + 1; ++i) {
                CPU_OP(c, stack, OBJ_FLOAT(c->regsf[i - CPU_REGS]), c->stack_size++, IP_INC_FLASE);
            }

            CPU_OP(c, stack, OBJ_UINT(c->zero_flag), c->stack_size++, IP_INC_FLASE);
            CPU_OP(c, stack, OBJ_UINT(c->fp), c->stack_size++, IP_INC_FLASE);
            CPU_OP(c, stack, OBJ_UINT(c->sp), c->stack_size++, IP_INC_FLASE);
            CPU_OP(c, stack, OBJ_UINT(c->ip), c->stack_size++, IP_INC_FLASE);

            c->sp += STACK_FRAME_SIZE;
            c->fp = c->sp;
            c->ip = operand1.u64;
            break;

        case INST_RET_N:
            cpu_inst_return(c);
            c->ip += 1;
            break;

        case INST_RET_RR:
            reg1 = cpu_fetch(c).reg;
            reg2 = cpu_fetch(c).reg;

            if (reg2 >= F0) operand1.f64 = c->regsf[reg2 - CPU_REGS];
            else operand1.i64 = c->regs[reg2];

            cpu_inst_return(c);

            if (reg1 >= F0) CPU_OP(c, regsf, operand1.f64, reg1 - CPU_REGS, IP_INC_TRUE);
            else CPU_OP(c, regs, operand1.i64, reg1, IP_INC_TRUE);
            break;

        case INST_RET_RV:
            reg1 = cpu_fetch(c).reg;
            operand1 = cpu_fetch(c);

            cpu_inst_return(c);

            if (reg1 >= F0) CPU_OP(c, regsf, operand1.f64, reg1 - CPU_REGS, IP_INC_TRUE);
            else CPU_OP(c, regs, operand1.i64, reg1, IP_INC_TRUE);
            break;

        case INST_VLAD:
            printf("Vlad eat's poop!\n");
            c->ip += 1;
            break;

        case IC:
        default:
            fprintf(stderr, "Error: undefine instruction `%s`\n", inst_as_cstr(inst));
            exit(1);
    }
}

void cpu_execute_program(CPU *const c, int debug, int limit, int stk)
{
    size_t ulimit = (size_t)(limit);
    for (size_t i = 0; c->halt == 0; ++i) {
        if (i > ulimit) break;
        if (debug) debug_regs(c);
        if (stk) debug_stack(c);
        cpu_execute_inst(c);
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

void debug_regs(CPU *const c)
{
    printf("ip: %"PRIu64"\n", c->ip);
    printf("sp: %"PRIu64"\n", c->sp);
    printf("fp: %"PRIu64"\n", c->fp);
    for (size_t i = 0; i < RC; ++i) {
        if (i >= F0) printf("%s: %lf\n", reg_as_cstr(i), c->regsf[i - CPU_REGS]); 
        else printf("%s: %"PRIi64"\n", reg_as_cstr(i), c->regs[i]);
    }
    printf("zero flag: %d\n", c->zero_flag);
    printf("halt: %d\n", c->halt);
    printf("\n");
}

// TODO: rework stack debug
void debug_stack(CPU *const c)
{
    printf("Stack:\n");
    if (c->stack_size == 0) printf("    empty\n");
    for (size_t i = 0; i < c->stack_size; ++i) {
        printf("    i64: %"PRIi64", u64: %"PRIu64", f64: %lf\n",
                c->stack[i].i64, c->stack[i].u64, c->stack[i].f64);
    }
}

void load_program_to_cpu(CPU *c, Object *program, size_t program_size)
{
    size_t all_size = sizeof(program[0]) * program_size;
    memcpy(c->program, program, all_size);
    c->program_size += program_size;
}   

void load_program_from_file(Arena *arena, CPU *c, const char *file_path)
{
    if (c->program_capacity == 0) {
        c->program_capacity = PROGRAM_INIT_CAPACITY;
        c->program = arena_alloc(arena, c->program_capacity * sizeof(*c->program));
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


    c->program_size = fread(c->program, sizeof(*c->program), meta.program_size, fp);
    if (meta.program_size != c->program_size) {
        fprintf(stderr, "Error: expected %"PRIi64" program size reading %"PRIi64"",
                meta.program_size, c->program_size);
        exit(1);
    }

    c->ip = meta.entry;

    fclose(fp);
}

void cpu_clean_program(CPU *const c)
{
    free(c->program);
    c->program = NULL;
    c->program_capacity = 0;
    c->program_size = 0;
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