#include "compiler.h"

Lasm *lasm_init()
{
    Lasm *L = malloc(sizeof(Lasm));
    L->a = malloc(sizeof(Arena));

    L->jmps             = (Label_List)  {0};
    L->defered_jmps     = (Label_List)  {0};
    L->src              = (String_View) {0};
    L->input_file       = NULL;
    L->output_file      = NULL;
    L->program          = NULL;
    L->program_capacity = 0;
    L->program_size     = 0;
    L->entry            = 0;

    inst_table_init(L->a, &L->instT, 0);
    ct_init(L->a, &L->constT, CT_CAPACITY);

    return L;
}

void lasm_cleanup(Lasm *L)
{
    L->program_capacity = 0;
    L->program_size = 0;
    L->input_file = NULL;
    L->output_file = NULL;

    arena_free(L->a);
    free(L->a);
    free(L);
    L = NULL;
}

void lasm_cmd_args(Lasm *L, int *argc, char ***argv)
{
    const char *program = luna_shift_args(argc, argv);

    if (*argc < 1)
        pr_error(ERRI, "%s expected input and output\n", program);

    while (*argc > 0) {
        const char *flag = luna_shift_args(argc, argv);
        if (!strcmp("-o", flag)) {
            if (*argc < 1)
                pr_error(ERRI, "%s expected output file\n", program);
            L->output_file = luna_shift_args(argc, argv);

        } else if (!strcmp("-i", flag)) {
            if (*argc < 1)
                pr_error(ERRI, "%s expected input file\n", program);
            L->input_file = luna_shift_args(argc, argv);

        } else if (!strcmp("-h", flag)) {
            lasm_help();
            exit(0);

        } else if (!strcmp("-g", flag)) {
            err_global.defined = 1;

        } else
            pr_error(ERRI, "unknown flag `%s`\n", flag);
    }

    err_global.a = L->a;
    err_global.program = L->input_file;
}

void lasm_load_file(Lasm *L)
{
    FILE *f = fopen(L->input_file, "rb");
    if (!f) pr_error(ERRI, "cannot open file `%s`\n", L->input_file);

    if (fseek(f, 0, SEEK_END) < 0)
        goto error;

    long int file_size = ftell(f);
    if (file_size < 0) goto error;

    char *buf = arena_alloc(L->a, file_size + 1);
    if (!buf) goto error;

    if (fseek(f, 0, SEEK_SET) < 0) goto error;

    size_t buf_len = fread(buf, sizeof(buf[0]), file_size, f);
    buf[buf_len] = '\0';

    L->src = sv_from_parts(buf, buf_len);
    fclose(f);
    return;

error:
    pr_error(ERRI, "cannot read from file `%s`\n", L->input_file);
}

void lasm_save_program_to_file(Lasm *L)
{
    FILE *fp = fopen(L->output_file, "wb");
    if (!fp)
        pr_error(ERRI, "cannot open file by `%s` path\n", L->output_file);

    Luna_File_Meta meta = {
        .magic = LUNA_MAGIC,
        .entry = L->entry,
        .program_size = L->program_size
    };

    fwrite(&meta, sizeof(meta), 1, fp);
    if (ferror(fp)) goto error;

    fwrite(L->program, sizeof(L->program[0]), L->program_size, fp);
    if (ferror(fp)) goto error;

    fclose(fp);
    return;

error:
    pr_error(ERRI, "Error: cannot write to `%s` file\n", L->output_file);
}

void lasm_translate_source(Lasm *L)
{
    Linizer linizer = lexical_analyze(L->a, L->src, &L->instT);
    LasmState *ls = parser_primary(L, &linizer);
    parser_secondary(ls, L);
}

void lasm_help()
{
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "\t-i          mandatory flag. Input source code with extention `.asm`\n");
    fprintf(stdout, "\t-o          mandatory flag. Input file for output (output is Luna's byte code) with extention `.ln`\n");
    fprintf(stdout, "\t-dbHt       print a hash table state\n");
    fprintf(stdout, "\t-dbLnz      print a linizer that was formed from lexer\n");
    fprintf(stdout, "\t-dbLex      print a lexer that was formed from source code\n");
    fprintf(stdout, "\t-dbLine     print lines with instruction and them kind\n");
    fprintf(stdout, "\t-dbLexTxts  print lexer's tokens with type `TYPE_TXT` while working function `lexer`\n");
    fprintf(stdout, "\t-dbFull     print all debug info into console\n");
}