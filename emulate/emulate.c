#include "../include/asm.h"

#define USAGE(program)                                                                              \
    fprintf(stderr, "Usage: %s <input.ven> -l <limit> -db <debug registers> -h <help>\n", (program))

CPU cpu = {0};

int main(int argc, char **argv)
{
    const char *flag;
    const char *input_file_path;
    int limit = -1;
    int db = 0;

    const char *program = asm_shift_args(&argc, &argv);

    while (argc != 0) {
        flag = asm_shift_args(&argc, &argv);
        
        if (!strcmp(flag, "-i")) {
            if (argc < 1) {
                USAGE(program);
                fprintf(stderr, "Error: after `-i` flag needs input file\n");
                exit(1);
            }

            input_file_path = asm_shift_args(&argc, &argv);

        } else if (!strcmp(flag, "-l")) {
            if (argc < 1) {
                USAGE(program);
                fprintf(stderr,"Error: after `-l` must be limit\n");
                exit(1);
            }

            const char *limit_cstr = asm_shift_args(&argc, &argv);

            if (!isdigit(*limit_cstr)) {
                fprintf(stderr, "Error: limit must be integer!\n");
                exit(1);
            }

            limit = atoi(limit_cstr); 

        } else if (!strcmp(flag, "-db")) { 
            db = 1;

        } else if (!strcmp(flag, "-h")) {
            USAGE(program);
            printf("\n------------------------------------------- VM Uasge -------------------------------------------\n\n");
            fprintf(stderr, "-i    get input file path and executing program. Flag expected `.ven` files\n");
            fprintf(stderr, "-l    limit of executing program. By default limit = -1\n");
            fprintf(stderr, "-db   if flag was provided, emulater will print info about registers state (all registers). By default db = 0\n");
            fprintf(stderr, "-h    print this usage\n");
            printf("\n-------------------------------------------------------------------------------------------------\n");
            exit(0);
        }
    }

    load_program_from_file(&cpu, input_file_path);
    cpu_execute_program(&cpu, db, limit);
}