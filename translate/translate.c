#include "../include/asm.h"

#define USAGE(program)                                                   \
    fprintf(stderr, "Usage: %s <input.asm> <output.ven>\n", (program))

CPU cpu = {0};
Program_Jumps PJ = {0};

int main(int argc, char **argv)
{
    const char *program = asm_shift_args(&argc, &argv);

    if (argc < 1) {
        USAGE(program);
        fprintf(stderr, "Error: %s expected input and output\n", program);
        exit(1);
    }

    const char *input_file_path = asm_shift_args(&argc, &argv);

    if (argc == 0) {
        USAGE(program);
        fprintf(stderr, "Error: %s expected output\n", program);
        exit(1);       
    }

    const char *output_file_path = asm_shift_args(&argc, &argv);
    String_View src = asm_load_file(input_file_path);

    asm_translate_source(&cpu, &PJ, src);
    load_program_to_file(&cpu, output_file_path);

    fprintf(stdout, "file `%s` translate to `%s`\n", input_file_path, output_file_path);
    return 0;
}