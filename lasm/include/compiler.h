#ifndef COMPILER_H_
#define COMPILER_H_

#include "parser.h"

#define USAGE(program) \
    fprintf(stdout, "Usage: %s -i <input.asm> -o <output.ln> [options]\n", (program))

Lasm *lasm_init();
void lasm_help();
void lasm_cleanup(Lasm *L);
void lasm_load_file(Lasm *L);
void lasm_cmd_args(Lasm *L, int *argc, char ***argv);

String_View lasm_cut_comments_from_src(String_View *sv);

void lasm_cut_comments_from_line(String_View *line);
void lasm_save_program_to_file(Lasm *L);
void lasm_translate_source(Lasm *L);

Linizer lexical_analyze(Arena *global, String_View src, Hash_Table *ht);

#endif // COMPILER_H_