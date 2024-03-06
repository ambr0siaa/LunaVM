#include "../kernel/asm/asm.h"

#define USAGE(program)                                                                              \
    fprintf(stderr, "Usage: %s -i <input.ven> [-l limit] [-db debug_registers] [-h]\n", (program))

static CPU cpu = {0};

int main(int argc, char **argv)
{
    const char *flag = NULL;
    const char *input_file_path = NULL;
    int limit = -1;
    int db = 0;
    int stk = 0;

    const char *program = asm_shift_args(&argc, &argv);

    while (argc != 0) {
        flag = asm_shift_args(&argc, &argv);

        if (!strcmp(flag, "-i")) {
            if (argc < 1) {
                USAGE(program);
                fprintf(stderr, "Error: after `-i` flag needs input file\n");
                return EXIT_FAILURE;
            }

            input_file_path = asm_shift_args(&argc, &argv);

        } else if (!strcmp(flag, "-l")) {
            if (argc < 1) {
                USAGE(program);
                fprintf(stderr,"Error: after `-l` must be limit\n");
                return EXIT_FAILURE;
            }

            const char *limit_cstr = asm_shift_args(&argc, &argv);

            //TODO: this checks only the first charachter
            if (!isdigit(*limit_cstr)) {
                fprintf(stderr, "Error: limit must be integer!\n");
                return EXIT_FAILURE;
            }

            limit = atoi(limit_cstr);

        } else if (!strcmp(flag, "-stk")) {
            stk = 1;

        } else if (!strcmp(flag, "-db")) {
            db = 1;

        } else if (!strcmp(flag, "-h")) {
            USAGE(program);
            printf("\n--------------------------------------- Super epic VM help ---------------------------------------\n\n");
            fprintf(stderr, "-i    input file name to execute. Expects files with `.ven` extension\n");
            fprintf(stderr, "-l    limit of executing program. Default is -1\n");
            fprintf(stderr, "-db   if flag was provided, emulator will print info about all registers state. Not enabled by default\n");
            fprintf(stderr, "-h    print this text and exit\n");
            printf("\n------------------------------------------------------------------------- Sample text ------------\n");
            return EXIT_SUCCESS;
        }
    }

    if (!input_file_path) {
        USAGE(program);
        fprintf(stderr, "Error: no input file\n");
        return EXIT_FAILURE;
    }

    load_program_from_file(&cpu, input_file_path);
    cpu_execute_program(&cpu, db, limit, stk);

    cpu_clean_program(&cpu);
}
