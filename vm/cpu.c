#include "../include/cpu.h"

char *reg_as_cstr(uint64_t operand)
{
    switch (operand) {
        case R0: return  "r0";
        case R1: return  "r1";
        case R2: return  "r2";
        case R3: return  "r3";
        case R4: return  "r4";
        case R5: return  "r5";
        case R6: return  "r6";
        case R7: return  "r7";
        case R8: return  "r8";
        case ACC: return "acc";

        case F0: return  "f0";
        case F1: return  "f1";
        case F2: return  "f2";
        case F3: return  "f3";
        case F4: return  "f4";
        case F5: return  "f5";
        case F6: return  "f6";
        case F7: return  "f7";
        case F8: return  "f8";
        case ACCF: return "accf";

        case RC:
        default:
            fprintf(stderr, "Error: unreachable `%li`\n", operand);
            exit(1);
    }
}

#define AREFMETIC_OP(c, type, op1, op2, operand, acc)       \
    do {                                                    \
        (c)->regs##type [acc] = (op1) operand (op2);        \
        (c)->ip += 1;                                       \
    } while (0)

#define STACK_OP(c, place, op1, index, on)  \
    do {                                    \
        (c)->place [(index)] = op1;         \
        if (on) (c)->ip += 1;               \
    } while(0)

void cpu_execute_inst(CPU *c)
{
    if (c->ip >= c->program_size) {
        fprintf(stderr, "Error: illigal instruction access\n");
        exit(1);
    }

    Inst inst = c->program[c->ip].inst;

    Register reg1;
    Register reg2;
    Object operand1;

    // TODO: Impelementation of INST_CALL and INST_RET

    switch (inst) {
        case INST_MOVI:
            reg1 = c->program[++c->ip].reg;
            c->regs[reg1] = c->program[++c->ip].i64;
            c->ip += 1;
            break;

        case INST_MOVF:
            reg1 = c->program[++c->ip].reg;
            operand1 = c->program[++c->ip];

            c->regsf[reg1] = operand1.f64;
            c->ip += 1;
            break;

        case INST_ADDI:
            operand1.i64 = c->regs[c->program[++c->ip].reg];
            AREFMETIC_OP(c, ,operand1.i64, c->regs[c->program[++c->ip].reg], +, ACC);
            break;
        
        case INST_SUBI:
            operand1.i64 = c->regs[c->program[++c->ip].reg];
            AREFMETIC_OP(c, ,operand1.i64, c->regs[c->program[++c->ip].reg], -, ACC);
            break;

        case INST_MULI:
            operand1.i64 = c->regs[c->program[++c->ip].reg];
            AREFMETIC_OP(c, ,operand1.i64, c->regs[c->program[++c->ip].reg], *, ACC);
            break;

        case INST_DIVI:
            operand1.i64 = c->regs[c->program[++c->ip].reg];
            AREFMETIC_OP(c, ,operand1.i64, c->regs[c->program[++c->ip].reg], /, ACC);
            break;

        case INST_ADDF:
            operand1.f64 = c->regsf[c->program[++c->ip].reg];
            AREFMETIC_OP(c, f, operand1.f64, c->regsf[c->program[++c->ip].reg], +, ACCF);
            break;
        
        case INST_SUBF:
            operand1.f64 = c->regsf[c->program[++c->ip].reg];
            AREFMETIC_OP(c, f, operand1.f64, c->regsf[c->program[++c->ip].reg], -, ACCF);
            break;

        case INST_MULF:
            operand1.f64 = c->regsf[c->program[++c->ip].reg];
            AREFMETIC_OP(c, f, operand1.f64, c->regsf[c->program[++c->ip].reg], *, ACCF);
            break;

        case INST_DIVF:
            operand1.f64 = c->regsf[c->program[++c->ip].reg];
            AREFMETIC_OP(c, f, operand1.f64, c->regsf[c->program[++c->ip].reg], /, ACCF);
            break;

        case INST_MOV:
            reg1 = c->program[++c->ip].reg;
            reg2 = c->program[++c->ip].reg;

            if (reg1 >= F0) c->regsf[reg1] = c->regsf[reg2];
            else c->regs[reg1] = c->regs[reg2];

            c->ip += 1;
            break;

        case INST_JMP:
            operand1.u64 = c->program[++c->ip].u64; 
            c->ip = operand1.u64;
            break;

        case INST_JNZ:
            operand1 = c->program[++c->ip];
            if (c->zero_flag) {
                c->ip = operand1.u64;
            }
            else c->ip += 1;
            break;

        case INST_CMP:
            // TODO: support floating point numbers
            reg1 = c->program[++c->ip].reg;
            reg2 = c->program[++c->ip].reg;
            c->zero_flag = c->regs[reg1] == c->regs[reg2];
            c->ip += 1;
            break;

        case INST_DBR:
            reg1 = c->program[++c->ip].reg;
            printf("%s: ", reg_as_cstr(reg1));

            if (reg1 >= F0) printf("%lf\n",c->regsf[reg1]);
            else printf("%li\n", c->regs[reg1]);

            c->ip += 1;
            break;

        case INST_HLT:
            c->halt = 1;
            break;

        case INST_JZ:
            operand1 = c->program[++c->ip];

            if (!c->zero_flag) c->ip = operand1.u64;
            else c->ip += 1;
            break;

        case INST_PUSH_VAL:
            if (c->stack_size > STACK_CAPACITY) {
                fprintf(stderr, "Error: stack overflow\n");
                exit(1);
            }

            c->stack_size++;
            operand1 = c->program[++c->ip];
            STACK_OP(c, stack, operand1, c->sp++, 1);
            break;

        case INST_PUSH_REG:
            if (c->stack_size > STACK_CAPACITY) {
                fprintf(stderr, "Error: stack overflow\n");
                exit(1);
            }

            reg1 = c->program[++c->ip].reg;

            if (reg1 >= F0) operand1 = OBJ_FLOAT(c->regsf[reg1]);
            else operand1 = OBJ_INT(c->regs[reg1]);

            c->stack_size++;
            STACK_OP(c, stack, operand1, c->sp++, 1);
            break;

        case INST_POP:
            if (c->stack_size < 1) {
                fprintf(stderr, "Error: stack underflow\n");
                exit(1);
            }

            reg1 = c->program[++c->ip].reg;
            
            --c->stack_size;
            if (reg1 >= F0) STACK_OP(c, regsf, c->stack[--c->sp].f64, reg1, 1);
            else STACK_OP(c, regs, c->stack[--c->sp].i64, reg1, 1);
            break;

        case INST_CALL:
            operand1 = c->program[++c->ip];

            for (size_t i = R0; i <= ACC; ++i) {
                STACK_OP(c, stack, OBJ_INT(c->regs[i]), c->stack_size++, 0);
            }

            for (size_t i = F0; i <= ACCF; ++i) {
                STACK_OP(c, stack, OBJ_FLOAT(c->regsf[i]), c->stack_size++, 0);
            }

            STACK_OP(c, stack, OBJ_UINT(c->fp), c->stack_size++, 0);
            STACK_OP(c, stack, OBJ_UINT(c->zero_flag), c->stack_size++, 0);
            STACK_OP(c, stack, OBJ_UINT(c->sp), c->stack_size++, 0);
            STACK_OP(c, stack, OBJ_UINT(c->ip), c->stack_size++, 0);

            c->sp += ALL_REGS + 1;
            c->fp = c->sp;
            c->ip = operand1.u64;
            break;

        case INST_RET:
            c->ip = c->stack[c->stack_size - 1].u64;
            c->sp = c->stack[c->fp - 2].u64;
            c->zero_flag = c->stack[c->fp - 3].u64;
            c->fp = c->stack[c->fp - 4].u64;

            for (int i = ACCF; i >= F0; --i) {
                STACK_OP(c, regsf, c->stack[--c->stack_size].f64, i, 0);
            }

            for (int i = ACC; i >= R0; --i) {
                STACK_OP(c, regs, c->stack[--c->stack_size].i64, i, 0);
            }

            c->ip += 1;
            break;

        case IC:
        default:
            fprintf(stderr, "Error: undefine instruction\n");
            exit(1);
    }
}

void cpu_execute_program(CPU *c, int debug, int limit, int stk)
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
        case INST_ADDI:     return "addi";
        case INST_SUBI:     return "subi";
        case INST_DIVI:     return "divi";
        case INST_MULI:     return "muli";
        case INST_ADDF:     return "addf";
        case INST_SUBF:     return "subf";
        case INST_DIVF:     return "divf";
        case INST_MULF:     return "mulf";
        case INST_MOVI:     return "movi";
        case INST_MOVF:     return "movf";
        case INST_HLT:      return "hlt";
        case INST_DBR:      return "dbr";
        case INST_MOV:      return "mov";
        case INST_JMP:      return "jmp";
        case INST_JNZ:      return "jnz";
        case INST_JZ:       return "jz";
        case INST_CMP:      return "cmp";
        case INST_PUSH_REG: return "pshr";
        case INST_PUSH_VAL: return "pshv";
        case INST_POP:      return "pop";
        case INST_CALL:     return "call";
        case INST_RET:      return "ret";
        case IC:        
        default:
            fprintf(stderr, "Error: `%u` this is not a inst\n", inst);
            exit(1);
    }
}

int inst_has_2_regs(Inst inst)
{
    switch (inst) {
        case INST_MOV:  return 1;
        case INST_CMP:  return 1;
        case INST_ADDI: return 1;
        case INST_SUBI: return 1;
        case INST_DIVI: return 1;
        case INST_MULI: return 1;
        case INST_ADDF: return 1;
        case INST_SUBF: return 1;
        case INST_DIVF: return 1;
        case INST_MULF: return 1;
        default:        return 0;
    }
}

int inst_has_no_ops(Inst inst)
{
    switch (inst) {
        case INST_HLT: return 1;
        case INST_RET: return 1;
        default:       return 0;
    }
}

int inst_has_1_op(Inst inst)
{
    switch (inst) {
        case INST_JMP:      return 1;
        case INST_JNZ:      return 1;
        case INST_DBR:      return 1;
        case INST_JZ:       return 1;
        case INST_POP:      return 1;
        case INST_PUSH_REG: return 1;
        case INST_PUSH_VAL: return 1;
        case INST_CALL:     return 1;
        default:            return 0;
    }
}

void debug_regs(CPU *c)
{
    printf("ip: %lu\n", c->ip);
    for (size_t i = 0; i < RC; ++i) {
        printf("%s: %li\n", reg_as_cstr(i), c->regs[i]);
    }
    printf("acc: %li\n", c->regs[ACC]);
    printf("zero flag: %d\n", c->zero_flag);
    printf("halt: %d\n", c->halt);
    printf("\n");
}

void debug_stack(CPU *c)
{
    printf("Stack:\n");
    if (c->stack_size == 0) printf("    empty\n");
    for (size_t i = 0; i < c->stack_size; ++i) {
        printf("    i64: %li, u64: %lu, f64: %lf\n",
                c->stack[i].i64, c->stack[i].u64, c->stack[i].f64);
    }
}

void load_program_to_cpu(CPU *c, Object *program, size_t program_size)
{
    size_t all_size = sizeof(program[0]) * program_size;
    memcpy(c->program, program, all_size);
    c->program_size += program_size;
}   

void load_program_to_file(CPU *c, const char *file_path)
{
    FILE *fp = fopen(file_path, "wb");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file by `%s` path\n", file_path);
        exit(1);
    }

    size_t count = fwrite(c->program, sizeof(c->program[0]), c->program_size, fp);
    if (count != c->program_size) {
        fprintf(stderr, "Error: cannot write to file `%s`", file_path);
        exit(1);
    }

    if (ferror(fp)) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    fclose(fp);
}

void load_program_from_file(CPU *c, const char *file_path)
{
    if (c->program_capacity == 0) {
        c->program_capacity = PROGRAM_INIT_CAPACITY;
        c->program = malloc(c->program_capacity * sizeof(c->program[0]));
    }

    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file by `%s` path\n", file_path);
        exit(1);
    }

    if (fseek(fp, 0, SEEK_END) < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    long int file_size = ftell(fp);
    if (file_size < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    size_t object_count = file_size / sizeof(c->program[0]);

    if (object_count >= c->program_capacity) {
        do { 
            c->program_capacity *= 2; 
        } while (object_count >= c->program_capacity);
        c->program = realloc(c->program, c->program_capacity); 
    }

    if (fseek(fp, 0, SEEK_SET) < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    c->program_size = fread(c->program, sizeof(c->program[0]), object_count, fp);

    if (ferror(fp)) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    fclose(fp);
}