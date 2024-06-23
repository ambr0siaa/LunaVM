#ifndef COMPILER_H_
#define COMPILER_H_

#include "parser.h"

#define USAGE(program) \
    fprintf(stdout, "Usage: %s -i <input.asm> -o <output.ln> [options]\n", (program))

Lasm *lasm_init();

void lasm_help();
void lasm_cleanup(Lasm *L);
void lasm_load_file(Lasm *L);
void lasm_translate_source(Lasm *L);
void lasm_save_program_to_file(Lasm *L);
void lasm_cmd_args(Lasm *L, int *argc, char ***argv);

#endif // COMPILER_H_