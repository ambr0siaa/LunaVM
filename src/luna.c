#include "luna.h"
#include "core.h"

#define OPTION_TSL(o) ((o) & 1) /* Translate option */
#define OPTION_ITR(o) (((o) & 2) >> 1) /* Interprete */
#define OPTION_OUT(o) (((o) & 4) >> 2) /* Make bytecode file */
#define OPTION_LIM(o) (((o) & 8) >> 3) /* Limit of execution */
#define OPTION_REG(o) (((o) & 16) >> 4) /* Registers debug*/
#define OPTION_STK(o) (((o) & 32) >> 5) /* Stack debug*/
#define OPTION_DB(o)  (((o) & 64) >> 6) /* Debug option */

LUNA_FUNC void opcodes_init(Table *opcodes)
{
    table_init(opcodes, IC * 2);
    for (size_t i = INST_MOV; i < IC; ++i) {
        Table_Item item = {0};
        char *name = (char*)inst_names[i];
        item.key = sv_from_cstr(name);
        item.hash = hash_string(item.key.data, item.key.count);
        item.value = i;
        table_insert(opcodes, item, Table);
    }
}

LUNA_FUNC Label_Map lmap_new(void)
{
    Label_Map map = {0};
    table_init(&map, LABEL_MAP_INIT_CAP);
    return map;
}

Luna *luna_init(void)
{
    Luna *L = malloc(sizeof(Luna));
    memset(L, 0, sizeof(Luna));
    L->s = malloc(sizeof(Statement));
    L->status = LUNA_STATUS_OK;
    L->file = NULL;
    L->code = NULL;
    L->bp = NULL;
    L->pc = L->ps = 0;
    L->core.limit = -1;
    return L;
}

void luna_clean(Luna *L)
{
    table_clean(&L->lmap);
    table_clean(&L->opcodes);
    if (L->code) free(L->code);
    free(L->src.data);
    free(L->s);
    free(L);
}

int luna_bytecode_out(Luna *L, const char *file_path)
{
    int status = 1;
    FILE *f = fopen(file_path, "w");
    if (!f) { status = 0; goto defer; }
    Luna_FileMeta meta = {
        .magic = LUNA_MAGIC,
        .entry = L->entry,
        .program_size = L->ps
    };
    fwrite(&meta, sizeof(meta), 1, f);
    fwrite(L->code, 1, L->ps, f);
    if (ferror(f)) { status = 0; goto defer; }
defer:
    if (!status) {
        luna_excp(EXCP_PROGRAM, NULL, "Cannot write bytecode to %s", file_path);
    }
    if (f) fclose(f);
    return status;
}

int luna_load_bytecode(Luna *L)
{
    int status = 1;
    FILE *f = fopen(L->file, "rb");
    if (!f) {
        luna_excp(EXCP_PROGRAM, NULL, "Cannot open file %s", L->file);
        status = 0; goto defer;
    }
    Luna_FileMeta meta = {0};
    size_t n = fread(&meta, sizeof(meta), 1, f);
    if (n < 1) {
        luna_excp(EXCP_PROGRAM, NULL, "Invalid read size %zu", n);
        status = 0; goto defer;
    }
    if (meta.magic != LUNA_MAGIC) {
        luna_excp(EXCP_PROGRAM, NULL, "Not a Luna bytecode format (LN)");
        luna_report("  Expected magic 0x%04X but provided 0x%04LX", LUNA_MAGIC, meta.magic);
        status = 0; goto defer;
    }
    if (!L->code) {
        L->pc = meta.program_size; 
        L->code = malloc(L->pc * sizeof(*L->code));
    }
    L->ps = fread(L->code, 1, meta.program_size, f);
    if (meta.program_size != L->ps) {
        luna_excp(EXCP_PROGRAM, NULL, "Expected %lu program size, but reading %lu", meta.program_size, L->ps);
        status = 0; goto defer;
    }
    L->entry = meta.entry;
defer:
    if (f) fclose(f);
    return status;
}

LUNA_FUNC char *shift_args(int *argc, char ***argv)
{
    char *result = **argv;
    *argc -= 1;
    *argv += 1;
    return result; 
}

LUNA_FUNC void luna_usage(const char *program)
{
    luna_report("usage:");
    luna_report("  Source files with extention 'asm', bytecode files with 'ln'");
    luna_report("    ./%s [file.asm | file.ln] [options]", program);
    luna_report("options:");
    luna_report("  By default translate file to bytecode");
    luna_report("    -h   print this usage");
    luna_report("    -o   program option, make a bytecode file from translated source. Expected '.ln' extention");
    luna_report("    -i   program option, read file as luna bytecode and interprete it. Expected '.ln' extention");
    luna_report("    -ti  program option, translate and interprete file. Expected '.asm' extention");
    luna_report("    -r   interpreting option, print state of all registers on each cycle. By default is off");
    luna_report("    -l   interpreting option, set execution limit for interpreter. By default no limit");
    luna_report("    -s   interpreting option, print stack state on each cycle. By default is off");
    luna_report("    -d   debug option, interpret bytecode step by step with all info about vm state");
}

LUNA_FUNC void command_option(int *options, char *arg)
{
    switch (arg[1]) {
        case 'h': {
            luna_usage("luna");
            *options = 0;
            break;
        } case 'i': {
            *options |= 2;
            break;
        } case 't': {
            if (arg[2] == 'i')
                *options |= 2;
            *options |= 1;
            break;
        } case 'l': {
            *options |= 8;
            break;
        } case 's': {
            *options |= 32;
            break;
        } case 'r': {
            *options |= 16;
            break;
        } case 'o': {
            *options |= 4;
            break;
        } case 'd': {
            *options |= 64;
            break;
        } default: {
            luna_report("luna: unknown option '%c'", arg[1]);
            *options = -1;
            break;
        }
    }
}

void luna_readfile(Luna *L, int options)
{
    if (OPTION_ITR(options) && !OPTION_TSL(options)) {
        if (!luna_load_bytecode(L))
            L->status = LUNA_STATUS_ERR;
    } else {
        L->src = sv_read_file(L->file);
    }
}

Backpatch *backpatch_new(Arena *a)
{
    Backpatch *bp = arena_alloc(a, sizeof(Backpatch));
    bp->capacity = 0;
    bp->count = 0;
    return bp;
}

void luna_dumpcode(Luna *L)
{
    luna_report("_________________________________________");
    Luna copy = *L;
    while (L->core.ip < L->ps) {
        printf("At byte %zu", L->core.ip);
        Instruction inst = luna_fetchI(L);
        inst_info(inst);
        u8 mode = iget_mode(inst);
        IOpcode op = iget_opcode(inst);
        switch (op) {
            case INST_MOV: {
                L->core.ip += 1;
                L->core.ip += type_sizes[mode];
                break;
            } case INST_CMP: {
                L->core.ip += 2;
                break;
            } case INST_JZ:
              case INST_JMP:
              case INST_JNZ:
              case INST_CALL: {
                L->core.ip += 8;
                break;
            } case INST_DBR: {
                L->core.ip += 1;
                break;
            } default: {
                break;
            }
        }
    }
    *L = copy;
    luna_report("_________________________________________");
}

void luna_translator(Luna *L, Arena *a)
{
    L->lmap = lmap_new();
    L->bp = backpatch_new(a);
    opcodes_init(&L->opcodes);
    Lexer lex = lexer_new(L->file, L->src);
    while (!lex_statempty(&lex)) {
        luna_translate_stmt(a, L, &lex);
        if (luna_staterr(L)) break; /* Error? */
    }
    if (L->bp && !luna_staterr(L))
        luna_backpatching(a, L);
    lexer_clean(&lex);
    /* luna_dumpcode(L); */
}

void luna_interpreter(Luna *L, int options)
{
    Arena allocs = {0};
    luna_core_init(&allocs, L);
    if (OPTION_DB(options)) {
        luna_report("Luna debug session");
        L->core.debug = 1; /* VM to debug mode */
    }
    for (i64 i = 0; !luna_stathlt(L); ++i) {
        if (L->core.limit > 0 && i > L->core.limit) break;
        luna_exec_inst(&allocs, L);
        if (luna_staterr(L)) break;
    }
    if (luna_stathlt(L)) {
        L->status = LUNA_STATUS_OK;
    }
    arena_free(&allocs);
}

int main(int argc, char **argv)
{
    int options = 0; /* Provided options from cmd */
    Arena arena = {0}; /* Placer of all dynamic allocations */
    Luna *L = luna_init(); /* Main state of luna */
    const char *output_file = NULL; /* Use when -o option provided */
    const char *program = shift_args(&argc, &argv); /* Skip program */
    if (argc == 0) { /* Expected some one option or file */
        luna_report("luna: expected input file");
        luna_usage(program);
        defer_status(LUNA_STATUS_ERR);
    }
    while (argc > 0) { /* Parse cmd arguments */
        char *arg = shift_args(&argc, &argv);
        if (arg[0] == '-') { /* Options */
            command_option(&options, arg);
            if (options < 0) { /* some shit occured */
                defer_status(LUNA_STATUS_ERR);
            } else if (options == 0) { /* -h option */
                defer_status(LUNA_STATUS_OK);
            } else if (OPTION_LIM(options)) { /* -l option */
                if (argc < 1) {
                    luna_report("luna: expected value for limit");
                    luna_usage(program);
                    defer_status(LUNA_STATUS_ERR);
                }
                arg = shift_args(&argc, &argv);
                if (!isdigit(*arg)) {
                    luna_report("luna: after option limit expected integer value");
                    luna_usage(program);
                    defer_status(LUNA_STATUS_ERR);
                }
                L->core.limit = atoi(arg);
                options &= ~0x8; /* Turn off */
            } else if (OPTION_OUT(options) && !output_file) { /* -o option */
                if (argc < 1) {
                    luna_report("luna: expected file path");
                    luna_usage(program);
                    defer_status(LUNA_STATUS_ERR);
                }
                output_file = shift_args(&argc, &argv);
            }
        } else { /* Input file path */
            L->file = arg;
            continue;
        }
    }
    if (!L->file) { /* No input file */
        luna_report("luna: expected input file");
        luna_usage(program);
        goto defer;
    }
    luna_readfile(L, options);
    luna_badcase(L); /* Cannot read file? */
    if (OPTION_ITR(options) && !OPTION_TSL(options)) { /* Only -i option */
        luna_interpreter(L, options);
        goto defer;
    }
    luna_translator(L, &arena);
    luna_badcase(L); /* If some trouble is occured */
    if (OPTION_ITR(options)) /* -ti option*/
        luna_interpreter(L, options);
    if (OPTION_OUT(options)) /* -o option */
        luna_bytecode_out(L, output_file);
defer: /* End of work */
    arena_free(&arena);
    int status = L->status;
    luna_clean(L);
    return EXIT_SUCCESS | (status ^ 1);
}
