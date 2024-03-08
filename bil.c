/*
* This `bil` file build:
*   in ./disasm `dis`           - disassembly bytecode and print in console
*   in ./kernel/lasm/src `tsl`  - translate assembly code to vm bytecode
*   in ./kernel/cpu/src `lunem` - emulate program from bytecode of vm
*   All usages in `.c` files
*/

#define BIL_IMPLEMENTATION
#include "bil.h"

#define CC "gcc"
#define DEBUG_MODE "-g", "-ggdb"
#define SRC "./kernel/lasm/src/lasm.c", "./kernel/cpu/src/cpu.c", "./kernel/common/sv.c", "./kernel/common/ht.c"

#ifdef _Win32
#   define CFLAGS " "
#else
#   define CFLAGS "-Wall", "-Wextra"
#endif

const char *sources[] = {
    "./kernel/lasm/src/translate.c",
    "./kernel/cpu/src/lunem.c",
    "./disasm/disasm.c"
};

const char *outputs[] = {
    "./kernel/lasm/src/tsl",
    "./kernel/cpu/src/lunem",
    "./disasm/dis"
};

#define PREF_DOT "."
#define PREF_CPU "cpu"
#define PREF_SRC "src"
#define PREF_LASM "lasm"
#define PREF_KERNEL "kernel"
#define PREF_EXAMPLES "examples"

#define FROM_KERNEL PREF_DOT, PREF_KERNEL

void mk_path_to_example(Bil_String_Builder *sb, int *argc, char ***argv)
{
    const char *target = bil_shift_args(argc, argv);
    *sb = PATH(FROM_KERNEL, PREF_LASM, PREF_EXAMPLES, target);
    sb_join_nul(sb);
}

int main(int argc, char **argv)
{
    BIL_REBUILD(argv);

    const char *flag = NULL;
    int debug = 0;
    int hdlr = 0;

    while (argc > 0) {
        flag = bil_shift_args(&argc, &argv);
        
        if (!strcmp("-db", flag)) {
            debug = 1;

        } else if (!strcmp("-hdlr", flag)) {
            hdlr = 1;
            break;
        }
    }

    if (hdlr) {
        Bil_Cmd handler = {0};
        const char *target = bil_shift_args(&argc, &argv);
        Bil_String_Builder target_path = {0};
     
        if (!strcmp("tsl", target)) {
            target_path = PATH(FROM_KERNEL, PREF_LASM, PREF_SRC, target);
            sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.str);

            Bil_String_Builder input_path = {0};
            Bil_String_Builder output_path = {0};

            mk_path_to_example(&input_path, &argc, &argv);
            mk_path_to_example(&output_path, &argc, &argv);

            bil_cmd_append(&handler, input_path.str, output_path.str);
            if (!bil_cmd_build_sync(&handler)) return BIL_EXIT_FAILURE;

            sb_clean(&input_path);
            sb_clean(&output_path);

        } else if (!strcmp("lunem", target)) {
            target_path = PATH(FROM_KERNEL, PREF_CPU, PREF_SRC, target);
            sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.str);

            Bil_String_Builder lunem_tar_path = {0};

            while (argc > 0) {
                const char *arg = bil_shift_args(&argc, &argv);
                bil_cmd_append(&handler, arg);
                if (!strcmp(arg, "-i")) {
                    mk_path_to_example(&lunem_tar_path, &argc, &argv);
                    bil_cmd_append(&handler, lunem_tar_path.str);
                }
            }

            if (!bil_cmd_build_sync(&handler)) return BIL_ERROR;
            sb_clean(&lunem_tar_path);

        } else if (!strcmp("dis", target)) {
            target_path = PATH(PREF_DOT, "disasm", target);
            sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.str);

            Bil_String_Builder input_path = {0};
            mk_path_to_example(&input_path, &argc, &argv);

            bil_cmd_append(&handler, input_path.str);
            if (!bil_cmd_build_sync(&handler)) return BIL_EXIT_FAILURE;
        }

        sb_clean(&target_path);
        bil_cmd_clean(&handler);

    } else {
        Bil_Cmd tsl = {0};
        Bil_Cmd eml = {0};
        Bil_Cmd dis = {0};

        Bil_Cmd commands[] = { tsl, eml, dis };

        for (size_t i = 0; i < BIL_ARRAY_SIZE(commands); ++i) {
            bil_cmd_append(&commands[i], CC);
            bil_cmd_append(&commands[i], CFLAGS);
            bil_cmd_append(&commands[i], SRC, sources[i]);
            bil_cmd_append(&commands[i], "-o");
            bil_cmd_append(&commands[i], outputs[i]);

            if (debug) {
                bil_cmd_append(&commands[i], DEBUG_MODE);
            }
                
            if (!bil_cmd_build_sync(&commands[i])) return BIL_EXIT_FAILURE;
        }

        bil_cmd_clean(&tsl);
        bil_cmd_clean(&eml);
        bil_cmd_clean(&dis);
    }

    return BIL_EXIT_SUCCESS;
}