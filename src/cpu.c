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

    // TODO: emplemation for F-insts

    switch (inst) {
        case INST_MOVI:
            reg1 = c->program[++c->ip].reg;
            c->regs[reg1] = c->program[++c->ip].i64;
            c->ip += 1;
            break;

        case INST_ADDI:
            c->regs[ACC] = c->regs[c->program[++c->ip].reg] + c->regs[c->program[++c->ip].reg];
            c->ip += 1;
            break;
        
        case INST_SUBI:
            c->regs[ACC] = c->regs[c->program[++c->ip].reg] - c->regs[c->program[++c->ip].reg];
            c->ip += 1;
            break;

        case INST_MULI:
            c->regs[ACC] = c->regs[c->program[++c->ip].reg] * c->regs[c->program[++c->ip].reg];
            c->ip += 1;
            break;

        case INST_DIVI:
            c->regs[ACC] = c->regs[c->program[++c->ip].reg] / c->regs[c->program[++c->ip].reg];
            c->ip += 1;
            break;

        case INST_MOV:
            reg1 = c->program[++c->ip].reg;
            reg2 = c->program[++c->ip].reg; 

            c->regs[reg1] = c->regs[reg2];
            c->ip += 1;
            break;

        case INST_JMP:
            c->ip = c->program[++c->ip].u64;
            break;

        case INST_JNZ:
            operand1 = c->program[++c->ip];
            if (c->zero_flag) {
                c->ip = operand1.u64;
            } 
            else c->ip += 1;
            break;

        case INST_CMP:
            c->zero_flag = c->regs[c->program[++c->ip].reg] == c->regs[c->program[++c->ip].reg];
            c->ip += 1;
            break;

        case INST_DBR:
            reg1 = c->program[++c->ip].reg;
            printf("%s: %li\n", reg_as_cstr(reg1), c->regs[reg1]);
            c->ip += 1;
            break;

        case INST_HLT:
            c->halt = 1;
            break;

        case INST_JZ:
            operand1 = c->program[++c->ip];
            if (!c->zero_flag) {
                c->ip = operand1.u64;
            } 
            else c->ip += 1;
            break;

        case IC:
        default:
            fprintf(stderr, "Error: undefine instruction\n");
            exit(1);
    }
}

void cpu_execute_program(CPU *c, int debug, int limit)
{
    for (size_t i = 0; i < limit && c->halt == 0; ++i) {
        if (debug) debug_regs(c);
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
        case IC:        
        default:
            assert(0 && "unknown inst");
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
        default:       return 0;
    }
}

int inst_has_1_op(Inst inst)
{
    switch (inst) {
        case INST_JMP: return 1;
        case INST_JNZ: return 1;
        case INST_DBR: return 1;
        case INST_JZ:  return 1;
        default:       return 0;
    }
}

void debug_regs(CPU *c)
{
    printf("ip: %lu\n", c->ip);
    for (size_t i = 0; i < RIC; ++i) {
        printf("%s: %li\n", reg_as_cstr(i), c->regs[i]);
    }
    printf("acc: %li\n", c->regs[ACC]);
    printf("zero flag: %d\n", c->zero_flag);
    printf("halt: %d\n", c->halt);
    printf("\n");
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
    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file by `%s` path\n", file_path);
        exit(1);
    }

    if (fseek(fp, 0, SEEK_END) < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    size_t file_size = ftell(fp);
    if (file_size < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    if (fseek(fp, 0, SEEK_SET) < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    c->program_size = fread(c->program, sizeof(c->program[0]), file_size / sizeof(c->program[0]), fp);

    if (ferror(fp)) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    fclose(fp);
}