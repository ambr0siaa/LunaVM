#include "cpu.h"

#define USAGE(program) \
    fprintf(stderr, "Usage: %s -i <input.ln> [-l limit] [-db debug registers] [-h help] [-stk debug stack]\n", (program))

static Arena arena = {0};
static CPU cpu = {0};

void lunem_help();

int main(int argc, char **argv)
{
    const char *flag = NULL;
    const char *input_file_path = NULL;
    int limit = -1;
    int db = 0;
    int stk = 0;

    const char *program = luna_shift_args(&argc, &argv);

    if (argc < 1) {
        USAGE(program);
        fprintf(stderr, "Error: not enough args\n");
        return EXIT_FAILURE;
    }

    while (argc != 0) {
        flag = luna_shift_args(&argc, &argv);

        if (!strcmp(flag, "-i")) {
            if (argc < 1) {
                USAGE(program);
                fprintf(stderr, "Error: after `-i` flag needs input file\n");
                return EXIT_FAILURE;
            }

            input_file_path = luna_shift_args(&argc, &argv);

        } else if (!strcmp(flag, "-l")) {
            if (argc < 1) {
                USAGE(program);
                fprintf(stderr,"Error: after `-l` must be limit\n");
                return EXIT_FAILURE;
            }

            const char *limit_cstr = luna_shift_args(&argc, &argv);

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
            lunem_help();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Error: unknown flag `%s`\n", flag);
            USAGE(program);
            return EXIT_FAILURE;
        }
    }

    load_program_from_file(&arena, &cpu, input_file_path);
    cpu_set_entry_ip(&cpu);
    cpu_execute_program(&cpu, db, limit, stk);

    arena_free(&arena);
    
    return EXIT_SUCCESS;
}

void lunem_help()
{
    printf("\n--------------------------------------------------- Lunem Usage ---------------------------------------------------\n\n");
    fprintf(stdout, "-i    input file name to execute. Expects files with `.ln` extension\n");
    fprintf(stdout, "-l    limit of executing program. Default is -1\n");
    fprintf(stdout, "-db   if flag was provided, emulator will print info about all registers state. Not enabled by default\n");
    fprintf(stdout, "-stk  flag is the same as -db, but debug stack state. Not enabled by default");
    fprintf(stdout, "-h    print this text and exit\n");
    printf("\n----------------------------------------------------------------------------------------------------------------\n");
}