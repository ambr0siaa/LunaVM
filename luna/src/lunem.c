#include <ctype.h>

#include "luna.h"

// [-l limit] [-db debug registers] [-h help] [-stk debug stack]

#define USAGE(program) \
    fprintf(stderr, "Usage: %s -i [input.ln] [options]\n", (program)); \
    fprintf(stderr, "options:\n"); \
    fprintf(stderr, "-l    limit of instruction execution. By default no limit\n"); \
    fprintf(stderr, "-db   debug register state on each cycle of execution. By default flag is off\n"); \
    fprintf(stderr, "-stk  debug stack state on each cycle of execution. By default flag is off\n"); \
    fprintf(stderr, "-h    prints this usage\n")

static Arena arena = {0};
static Luna L = {0};

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
            return EXIT_SUCCESS;
            
        } else {
            fprintf(stderr, "Error: unknown flag `%s`\n", flag);
            USAGE(program);
            return EXIT_FAILURE;
        }
    }

    load_program_from_file(&arena, &L, input_file_path);
    luna_execute_program(&L, db, limit, stk);

    arena_free(&arena);
    return EXIT_SUCCESS;
}
