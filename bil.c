/*
* This `bil` file builds:
*   in ./dilasm `dilasm.c`        - disassembly bytecode and print it in console
*   in ./kernel/lasm/src `lasm.c` - translate assembly code to Luna's bytecode
*   in ./kernel/cpu/src `lunem.c` - emulate program the Luna's bytecode
*
* All usages in `.c` files and in README
*
* Flags:
*    -d     delete file in working dir `DELETME`. In macro `BIL_REBUILD` provided path to working dir where `bil` and `DELETEME` binaries are
*           By default it `/bin`.
*
*    -b     if flag was provided it expexted target for building (lasm, lunem, dis) and build only 1 provided target. 
*           By default `bil` builds all targets.
*
*    -db    if flag was provided current targets builds with `-g` and `-ggdb` for debug.
*
*    -hdlr  bil's handler flag. It simplify working with main targets (lasm, lunem, dis) by making path's to examples 
*           Example: type in terminal [ ./bin/bil -hdlr lasm -i call.asm -o call.ln ]
*/

#define BIL_IMPLEMENTATION
#include "bil.h"

#define CC "gcc"
#define DEBUG_MODE "-g", "-ggdb"

#define SRC_CPU               \
    "./kernel/cpu/src/cpu.c", \
    "./kernel/common/arena.c"

#define SRC_COMMON          \
    "./kernel/common/sv.c", \
    "./kernel/common/ht.c"  \

#define SRC_LASM                    \
    "./kernel/lasm/src/compiler.c", \
    "./kernel/lasm/src/parser.c",   \
    "./kernel/lasm/src/lexer.c",    \
    "./kernel/lasm/src/linizer.c",  \
    "./kernel/lasm/src/consts.c",   \
    "./kernel/lasm/src/eval.c"

char *binary_dir_path = "bin";

#ifdef _Win32
#   define CFLAGS " "
#else
#   define CFLAGS "-Wall", "-Wextra", "-flto"
#endif

const char *targets[] = {
    "./kernel/lasm/src/lasm.c",
    "./kernel/cpu/src/lunem.c",
    "./dilasm/dilasm.c"
};

#define TARGET_LASM 0 
#define TARGET_LUNEM 1
#define TARGET_DILASM 2

const char *outputs[] = {
    "./kernel/lasm/src/lasm",
    "./kernel/cpu/src/lunem",
    "./dilasm/dilasm"
};

#define PREF_DOT "."
#define PREF_CPU "cpu"
#define PREF_SRC "src"
#define PREF_LASM "lasm"
#define PREF_KERNEL "kernel"
#define PREF_EXAMPLES "examples"

#define TO_KERNEL PREF_DOT, PREF_KERNEL

void mk_path_to_example(Bil_String_Builder *sb, int *argc, char ***argv)
{
    const char *target = bil_shift_args(argc, argv);
    *sb = PATH(TO_KERNEL, PREF_LASM, PREF_EXAMPLES, target);
    sb_join_nul(sb);
}

int main(int argc, char **argv)
{
    BIL_REBUILD(argv, binary_dir_path);

    const char *flag = NULL;
    const char *build_file = NULL;

    int debug = 0;
    int hdlr = 0;

    while (argc > 0) {
        flag = bil_shift_args(&argc, &argv);
        
        if (!strcmp("-db", flag)) {
            debug = 1;

        } else if (!strcmp("-hdlr", flag)) {
            hdlr = 1;
            break;

        } else if (!strcmp("-b", flag)){
            if (argc < 1) {
                bil_log(BIL_ERROR, "flag `-b` expcted target for build");
                return BIL_EXIT_FAILURE;
            }
            build_file = bil_shift_args(&argc, &argv);

        } else if (!strcmp("-d", flag)) {
            if (!strcmp(binary_dir_path, BIL_DIR_CURRENT)) {
                bil_delete_file(DELETEME);
            } else {
                Bil_String_Builder sb = PATH(PREF_DOT, binary_dir_path, DELETEME);
                sb_join_nul(&sb);
                bil_delete_file(sb.str);
                sb_clean(&sb);
            }
            if (argc < 1) return BIL_EXIT_SUCCESS;
        }
    }

    if (hdlr) {
        Bil_Cmd handler = {0};
        const char *target = bil_shift_args(&argc, &argv);
        Bil_String_Builder target_path = {0};
     
        if (!strcmp("lasm", target)) {
            target_path = PATH(TO_KERNEL, PREF_LASM, PREF_SRC, target);
            sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.str);

            Bil_String_Builder output_path = {0};
            Bil_String_Builder input_path = {0};

            while (argc > 0) {
                flag = bil_shift_args(&argc, &argv);
                bil_cmd_append(&handler, flag);
                if (!strcmp("-o", flag)) {
                    if (argc > 0) {
                        mk_path_to_example(&output_path, &argc, &argv);
                        bil_cmd_append(&handler, output_path.str);
                    }
                } else if (!strcmp("-i", flag)) {
                    if (argc > 0) {
                        mk_path_to_example(&input_path, &argc, &argv);
                        bil_cmd_append(&handler, input_path.str);
                    }
                }
            }

            if (!bil_cmd_build_sync(&handler)) return BIL_EXIT_FAILURE;

            sb_clean(&input_path);
            sb_clean(&output_path);

        } else if (!strcmp("lunem", target)) {
            target_path = PATH(TO_KERNEL, PREF_CPU, PREF_SRC, target);
            sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.str);

            Bil_String_Builder lunem_tar_path = {0};

            while (argc > 0) {
                flag = bil_shift_args(&argc, &argv);
                bil_cmd_append(&handler, flag);
                if (!strcmp(flag, "-i")) {
                    mk_path_to_example(&lunem_tar_path, &argc, &argv);
                    bil_cmd_append(&handler, lunem_tar_path.str);
                }
            }

            if (!bil_cmd_build_sync(&handler)) return BIL_ERROR;
            sb_clean(&lunem_tar_path);

        } else if (!strcmp("dilasm", target)) {
            target_path = PATH(PREF_DOT, "dilasm", target);
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
        Bil_Cmd dilasm = {0};
        Bil_Cmd lasm = {0};
        Bil_Cmd lunem = {0};

        Bil_Cmd commands[] = { lasm, lunem, dilasm };
        size_t builds_count = 0;
        size_t target = 0;

        if (build_file != NULL) {
            builds_count = 2;
            if (!strcmp("lasm", build_file)) {
                target = TARGET_LASM;

            } else if (!strcmp("lunem", build_file)) {
                target = TARGET_LUNEM;

            } else if (!strcmp("dilasm", build_file)) {
                target = TARGET_DILASM;

            } else {
                bil_log(BIL_ERROR, "unknown target `%s`", build_file);
                return BIL_EXIT_FAILURE;
            }
        }
        
        bil_log(BIL_INFO, "Start building targets");

        for (size_t i = builds_count; i < BIL_ARRAY_SIZE(commands); ++i) {
            bil_cmd_append(&commands[target], CC);
            bil_cmd_append(&commands[target], CFLAGS);
            
            if (target == TARGET_LASM) {
                bil_cmd_append(&commands[target], SRC_LASM, SRC_CPU, SRC_COMMON);
            } else if (target == TARGET_LUNEM || target == TARGET_DILASM) {
                bil_cmd_append(&commands[target], SRC_CPU);
            }

            bil_cmd_append(&commands[target], targets[target]);
            bil_cmd_append(&commands[target], "-o");
            bil_cmd_append(&commands[target], outputs[target]);

            if (debug) bil_cmd_append(&commands[target], DEBUG_MODE);
            if (!bil_cmd_build_sync(&commands[target])) return BIL_EXIT_FAILURE;
            if (builds_count == 0) target++;
        }

        bil_cmd_clean(&lasm);
        bil_cmd_clean(&lunem);
        bil_cmd_clean(&dilasm);
    }

    return BIL_EXIT_SUCCESS;
}