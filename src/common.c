#include "common.h"

const char *inst_names[IC] = {
    [INST_MOV]  = "mov",  [INST_ADD] = "add",  [INST_SUB]  = "sub",  [INST_MUL] = "mul", 
    [INST_DIV]  = "div",  [INST_MOD] = "mod",  [INST_AND]  = "and",  [INST_NOT] = "not",
    [INST_XOR]  = "xor",  [INST_SHL] = "shl",  [INST_SHR]  = "shr",  [INST_OR]  = "or", 
    [INST_PUSH] = "push", [INST_POP] = "pop",  [INST_CALL] = "call", [INST_RET] = "ret", 
    [INST_JMP]  = "jmp",  [INST_JNZ] = "jnz",  [INST_JZ]   = "jz",   [INST_CMP] = "cmp", 
    [INST_GE]   = "ge",   [INST_GT]  = "gt",   [INST_LT]   = "lt",   [INST_LE]  = "le", 
    [INST_HLT]  = "hlt",  [INST_DBR] = "dbr",  [INST_VLAD] = "vlad", [INST_ALC] = "alloca",
    [INST_PULL] = "pull"
};

const size_t type_sizes[AMOUNT_OF_OBJ] = {
    [OBJ_I8]  = 1, [OBJ_U8]  = 1, [OBJ_CH]  = 1, [OBJ_I16] = 2, [OBJ_U16] = 2,
    [OBJ_I32] = 4, [OBJ_U32] = 4, [OBJ_F32] = 4, [OBJ_I64] = 8, [OBJ_U64] = 8,
    [OBJ_F64] = 8,
};

void luna_report(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

void luna_excp(Excp_Level level, Location *loc, const char *fmt, ...)
{
    fprintf(stderr, "File %s(%zu,%zu)\n    ", loc->file, loc->row, loc->col);
    switch (level) {
        case EXCP_LEXICAL:   fprintf(stderr, "Lexical Exception : ");   break;
        case EXCP_PARSING:   fprintf(stderr, "Parsing Exception : ");   break;
        case EXCP_TRANSLATE: fprintf(stderr, "Translate Exception : "); break;
        default:       luna_assert(0 && "Unreachable exception level");
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void inst_info(Instruction inst)
{
    printf("  Opcode %s\n", inst_names[iget_opcode(inst)]);
    printf("  Mode   %u\n", iget_mode(inst));
    printf("  Arg1   %u\n", iget_arg1(inst));
    printf("  Arg2   %u\n", iget_arg2(inst));
    printf("  Label  %u\n", iget_label(inst));
    printf("  Seg    %u\n", iget_seg(inst));
    printf("  Ptr    %u\n", iget_ptr(inst));
}
