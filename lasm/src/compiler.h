#ifndef COMPILER_H_
#define COMPILER_H_

#include "parser.h"

#define USAGE(program) \
    fprintf(stdout, "Usage: %s -i <input.asm> -o <output.ln> [options]\n", (program))

LUNA_API Lasm *lasm_init();

LUNA_API void lasm_help();
LUNA_API void lasm_cleanup(Lasm *L);
LUNA_API void lasm_load_file(Lasm *L);
LUNA_API void lasm_translate_source(Lasm *L);
LUNA_API void lasm_save_program_to_file(Lasm *L);
LUNA_API void lasm_cmd_args(Lasm *L, int *argc, char ***argv);

#endif // COMPILER_H_
