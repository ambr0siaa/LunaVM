#include "../include/compiler.h"

#define USAGE(program) \
    fprintf(stderr, "Usage: %s -i <input.asm> -o <output.ln> [-h help] [-db... debugs, to get all usages run with -h]\n", (program))

static Const_Table ct = { .capacity = CT_CAPACITY };
static Hash_Table inst_table = {0};
static Program_Jumps PJ = {0};
static Arena arena = {0};
static CPU cpu = {0};

void lasm_help();

int main(int argc, char **argv)
{
    const char *program = luna_shift_args(&argc, &argv);

    int db_ht = HT_DEBUG_FALSE;
    int db_lex = LEX_DEBUG_FALSE;
    int db_lnz = LNZ_DEBUG_FALSE;
    int db_line = LINE_DEBUG_FALSE;
    int db_lex_txts = LEX_DEBUG_TXTS_FALSE;
    int db_output_program = BLOCK_CHAIN_DEBUG_FALSE;

    const char *output_file_path = NULL;
    const char *input_file_path = NULL;

    if (argc < 1) {
        USAGE(program);
        fprintf(stderr, "Error: %s expected input and output\n", program);
        return EXIT_FAILURE;
    }

    while (argc > 0) {
        const char *flag = luna_shift_args(&argc, &argv);
        if (!strcmp("-o", flag)) {
            if (argc < 1) {
                fprintf(stderr, "Error: %s expected output file\n", program);
                return EXIT_FAILURE;
            }
            
            output_file_path = luna_shift_args(&argc, &argv);

        } else if (!strcmp("-i", flag)) {
            if (argc < 1) {
                fprintf(stderr, "Error: %s expected input file\n", program);
                return EXIT_FAILURE;
            }

            input_file_path = luna_shift_args(&argc, &argv);

        } else if (!strcmp("-dbFull", flag)) {
            db_ht = HT_DEBUG_TRUE;
            db_lex = LEX_DEBUG_TRUE;
            db_lnz = LNZ_DEBUG_TRUE;
            db_line = LINE_DEBUG_TRUE;
            db_lex_txts = LEX_DEBUG_TXTS_TRUE;

        } else if (!strcmp("-dbHt", flag)) {
            db_ht = HT_DEBUG_TRUE;

        } else if (!strcmp("-dbLex", flag)) {
            db_lex = LEX_DEBUG_TRUE;

        } else if (!strcmp("-dbLnz", flag)) {
            db_lnz = LNZ_DEBUG_TRUE;

        } else if (!strcmp("-dbLexTxts", flag)) {
            db_lex_txts = LEX_DEBUG_TXTS_TRUE;

        } else if (!strcmp("-dbLine", flag)) {
            db_line = LINE_DEBUG_TRUE;

        } else if (!strcmp("-dbPrg", flag)) {
            db_output_program = BLOCK_CHAIN_DEBUG_TRUE;

        } else if (!strcmp("-h", flag)) {
            USAGE(program);
            lasm_help();
            return EXIT_SUCCESS;

        } else {
            fprintf(stderr, "Error: unknown flag `%s`\n", flag);
            return EXIT_FAILURE;
        }
    }

    String_View src = lasm_load_file(&arena, input_file_path);
    lasm_translate_source(&arena, src, &cpu, &PJ, &inst_table, &ct, db_lex, db_lnz, db_ht, db_line, db_lex_txts, db_output_program);
    lasm_save_program_to_file(&cpu, output_file_path);

    arena_free(&arena);

    fprintf(stdout, "lasm: file `%s` was translated to `%s`\n", input_file_path, output_file_path);
    fprintf(stdout, "lasm: translation has done.\n");
    
    return EXIT_SUCCESS;
}

void lasm_help()
{
    fprintf(stdout, "\n-------------------------------------------- Lasm Usage --------------------------------------------\n\n");
    fprintf(stdout, "-o          mandatory flag. Input file for output (output is luna's byte code) with extention `.ln`\n");
    fprintf(stdout, "-i          mandatory flag. Input source code with extention `.asm`\n");
    fprintf(stdout, "-dbHt       print a hash table state\n");
    fprintf(stdout, "-dbLnz      print a linizer that was formed by lexer\n");
    fprintf(stdout, "-dbLex      print a lexer that was formed by source code\n");
    fprintf(stdout, "-dbLine     print lines with instruction and them kind\n");
    fprintf(stdout, "-dbLexTxts  print lexer's tokens with type `TYPE_TXT` while working function `lexer`\n");
    fprintf(stdout, "-dbFull     print all debug info into console\n");
    fprintf(stdout, "\n---------------------------------------------------------------------------------------------------\n");
}