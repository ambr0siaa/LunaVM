#include "../include/lasm.h"

#define USAGE(program) \
    fprintf(stderr, "Usage: %s <input.asm> <output.ven>\n", (program))

static Program_Jumps PJ = {0};
static CPU cpu = {0};

int main(int argc, char **argv)
{
    const char *program = luna_shift_args(&argc, &argv);

    if (argc < 1) {
        USAGE(program);
        fprintf(stderr, "Error: %s expected input and output\n", program);
        return EXIT_FAILURE;
    }

    const char *input_file_path = luna_shift_args(&argc, &argv);

    if (argc < 1) {
        USAGE(program);
        fprintf(stderr, "Error: %s expected output\n", program);
        return EXIT_FAILURE;
    }

    const char *output_file_path = luna_shift_args(&argc, &argv);
    String_View src = asm_load_file(input_file_path);

    asm_translate_source(&cpu, &PJ, src);
    load_program_to_file(&cpu, output_file_path);

    fprintf(stdout, "file `%s` was translated to `%s`\n", input_file_path, output_file_path);
    fprintf(stdout, "done.\n");

    free(src.data);
    free(PJ.current.lables);
    free(PJ.deferred.lables);
    cpu_clean_program(&cpu);

    return EXIT_SUCCESS;
}