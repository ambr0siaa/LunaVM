#include "compiler.h"

int main(int argc, char **argv)
{
    Lasm *L = lasm_init();
    lasm_cmd_args(L, &argc, &argv);

    lasm_load_file(L);
    lasm_translate_source(L);
    lasm_save_program_to_file(L);

    fprintf(stdout, "lasm: file `%s` was translated to `%s`\n", L->input_file, L->output_file);

    lasm_cleanup(L);
    return EXIT_SUCCESS;
}
