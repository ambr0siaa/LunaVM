/*
* This `bil` file builds:
*   in dilasm `dilasm.c` - disassembly bytecode and print it in console
*   in lasm/src `lasm.c` - translate assembly code to Luna's bytecode
*   in cpu/src `lunem.c` - emulate program the Luna's bytecode
*
* All usages in `.c` files and in README
*
* Flags:
*    -d     delete file in working dir `DELETME`. In macro `BIL_REBUILD` provided path to working dir where `bil` and `DELETEME` binaries are
*           By default it `/bin`.
*
*    -b     if flag was provided it expexted target for building (lasm, lunem, dilasm) and build only 1 provided target. 
*           By default `bil` builds all targets.
*
*    -db    if flag was provided current targets builds with `-g` and `-ggdb` for debug.
*
*    -hdlr  bil's handler flag. It simplify working with main targets (lasm, lunem, dilasm) by making path's to examples 
*           Example: type in terminal [ bin/bil -hdlr lasm -i call.asm -o call.ln ]
*/

#define BIL_IMPLEMENTATION
#include "bil.h"

#define CC "gcc"
#define DEBUG_MODE "-g3", "-ggdb"

#define SRC_CPU      \
    "cpu/src/cpu.c", \
    "common/arena.c"

#define SRC_COMMON \
    "common/sv.c", \
    "common/ht.c"

#define SRC_LASM           \
    "lasm/src/compiler.c", \
    "lasm/src/parser.c",   \
    "lasm/src/lexer.c",    \
    "lasm/src/linizer.c",  \
    "lasm/src/consts.c",   \
    "lasm/src/eval.c"

static int debug = 0;
static int hdlr = 0;
static char *binary_dir_path = "bin";

const char *dep_dir_path = "bin/dep";
const char *lasm_dep_path = "bin/dep/lasm.bil";
const char *lunem_dep_path = "bin/dep/lunem.bil";
const char *dilasm_dep_path = "bin/dep/dilasm.bil";

#ifdef _Win32
#   define CFLAGS " "
#else
#   define CFLAGS "-Wall", "-Wextra", "-flto"
#endif

char *targets[] = {
    "lasm/src/lasm.c",
    "cpu/src/lunem.c",
    "dilasm/dilasm.c"
};

#define TARGET_LASM 0 
#define TARGET_LUNEM 1
#define TARGET_DILASM 2

char *outputs[] = {
    "lasm/src/lasm",
    "cpu/src/lunem",
    "dilasm/dilasm"
};

#define PREF_DOT "."
#define PREF_CPU "cpu"
#define PREF_SRC "src"
#define PREF_LASM "lasm"
#define PREF_EXAMPLES "examples"
#define PREF_BYTECODE "examples/bytecode"

void mk_path_to_example(Bil_String_Builder *sb, char *where, int *argc, char ***argv)
{
    const char *target = bil_shift_args(argc, argv);
    *sb = PATH(PREF_LASM, where, target);
    sb_join_nul(sb);
}

void cc(Bil_Cmd *cmd)
{
    bil_cmd_append(cmd, CC);
    bil_cmd_append(cmd, CFLAGS);
    bil_cmd_append(cmd, "-o");
}

int build_lasm(Bil_Cmd *cmd)
{
    cc(cmd);
    bil_cmd_append(cmd, outputs[TARGET_LASM]);
    bil_cmd_append(cmd, targets[TARGET_LASM]);
    bil_cmd_append(cmd, SRC_LASM, SRC_CPU, SRC_COMMON);
    if (!bil_cmd_run_sync(cmd)) return BIL_EXIT_FAILURE;
    cmd->count = 0;
    return BIL_EXIT_SUCCESS;
}

int build_lunem(Bil_Cmd *cmd)
{
    cc(cmd);
    bil_cmd_append(cmd, outputs[TARGET_LUNEM]);
    bil_cmd_append(cmd, targets[TARGET_LUNEM]);
    bil_cmd_append(cmd, SRC_CPU);
    if (!bil_cmd_run_sync(cmd)) return BIL_EXIT_FAILURE;
    cmd->count = 0;
    return BIL_EXIT_SUCCESS;
}

int build_dilasm(Bil_Cmd *cmd)
{
    cc(cmd);
    bil_cmd_append(cmd, outputs[TARGET_DILASM]);
    bil_cmd_append(cmd, targets[TARGET_DILASM]);
    bil_cmd_append(cmd, SRC_CPU);
    if (!bil_cmd_run_sync(cmd)) return BIL_EXIT_FAILURE;
    cmd->count = 0;
    return BIL_EXIT_SUCCESS;
}

void cmd_args(int *argc, char ***argv)
{
    while (*argc > 0) {
        const char *flag = bil_shift_args(argc, argv);
        if (!strcmp("-db", flag)) {
            debug = 1;

        } else if (!strcmp("-hdlr", flag)) {
            hdlr = 1;
            break;

        } else if (!strcmp("-d", flag)) {
            if (!strcmp(binary_dir_path, BIL_CURRENT_DIR)) {
                bil_delete_file(DELETEME_FILE);
            } else {
                Bil_String_Builder sb = PATH(PREF_DOT, binary_dir_path, DELETEME_FILE);
                sb_join_nul(&sb);
                bil_delete_file(sb.items);
                sb_clean(&sb);
            }
            if (*argc < 1)
                BIL_EXIT(0);

        } else if (!strcmp("-b", flag)) {
            Bil_Cmd cmd = {0};
            int status = BIL_EXIT_SUCCESS; 
            for (size_t i = 0; i < 3; ++i) {
                switch (i) {
                    case TARGET_LASM: status = build_lasm(&cmd); break;
                    case TARGET_LUNEM: status = build_lunem(&cmd); break;
                    case TARGET_DILASM: status = build_dilasm(&cmd); break;
                }
            }
            bil_cmd_clean(&cmd);
            BIL_EXIT(status);
        }
    }
}

void cmd_handler(int *argc, char ***argv)
{
    if (hdlr) {
        if (*argc < 1) {
            bil_log(BIL_ERROR, "expected commands for handler");
            BIL_EXIT(BIL_EXIT_FAILURE);
        }

        Bil_Cmd handler = {0};
        const char *target = bil_shift_args(argc, argv);
        Bil_String_Builder target_path = {0};
     
        if (!strcmp("lasm", target)) {
            if (*argc < 1) {
                lasm_error:
                bil_log(BIL_ERROR, "expected commands for `lasm`");
                CMD("lasm/src/lasm", "-h");
                BIL_EXIT(BIL_EXIT_FAILURE);
            }

            target_path = PATH(PREF_LASM, PREF_SRC, target);
            sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.items);

            Bil_String_Builder output_path = {0};
            Bil_String_Builder input_path = {0};

            while (*argc > 0) {
                const char *flag = bil_shift_args(argc, argv);
                bil_cmd_append(&handler, flag);
                if (*argc < 1) {
                    goto lasm_error;
                }

                if (!strcmp("-o", flag)) {
                    mk_path_to_example(&output_path, PREF_BYTECODE, argc, argv);
                    bil_cmd_append(&handler, output_path.items);
                } else if (!strcmp("-i", flag)) {
                    mk_path_to_example(&input_path, PREF_EXAMPLES, argc, argv);
                    bil_cmd_append(&handler, input_path.items);
                }
            }

            if (!bil_cmd_run_sync(&handler)) BIL_EXIT(BIL_EXIT_FAILURE);

            sb_clean(&input_path);
            sb_clean(&output_path);

        } else if (!strcmp("lunem", target)) {
            if (*argc < 1) {
                bil_log(BIL_ERROR, "expected commands for `lunem`");
                CMD("lasm/src/lunem", "-h");
                BIL_EXIT(BIL_EXIT_FAILURE);
            }

            target_path = PATH(PREF_CPU, PREF_SRC, target);
            sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.items);

            Bil_String_Builder lunem_tar_path = {0};

            while (*argc > 0) {
                const char *flag = bil_shift_args(argc, argv);
                bil_cmd_append(&handler, flag);
                if (!strcmp(flag, "-i")) {
                    mk_path_to_example(&lunem_tar_path, PREF_BYTECODE, argc, argv);
                    bil_cmd_append(&handler, lunem_tar_path.items);
                }
            }

            if (!bil_cmd_run_sync(&handler)) BIL_EXIT(BIL_EXIT_FAILURE);
            sb_clean(&lunem_tar_path);

        } else if (!strcmp("dilasm", target)) {
            if (*argc < 1) {
                bil_log(BIL_ERROR, "expected commands for `dilasm`");
                BIL_EXIT(BIL_EXIT_FAILURE);
            }

            target_path = PATH(PREF_DOT, "dilasm", target);
            sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.items);

            Bil_String_Builder input_path = {0};
            mk_path_to_example(&input_path, PREF_BYTECODE, argc, argv);

            bil_cmd_append(&handler, input_path.items);
            if (!bil_cmd_run_sync(&handler)) BIL_EXIT(BIL_EXIT_FAILURE);
        }

        sb_clean(&target_path);
        bil_cmd_clean(&handler);
        BIL_EXIT(BIL_EXIT_SUCCESS);
    }
}

int main(int argc, char **argv)
{
    BIL_REBUILD(argv, binary_dir_path);

    if (!bil_dir_exist(dep_dir_path)) 
                bil_mkdir(dep_dir_path);

    cmd_args(&argc, &argv);
    cmd_handler(&argc, &argv);

    int status = -1;
    Bil_Cmd cmd = {0};

    Bil_Dep lasm = {0};
    Bil_Dep lunem = {0};
    Bil_Dep dilasm = {0};

    bil_dep_init(&lasm, lasm_dep_path, 
                 targets[TARGET_LASM], SRC_LASM, 
                 SRC_CPU, SRC_COMMON);
    
    bil_dep_init(&lunem, lunem_dep_path,
                 targets[TARGET_LUNEM], SRC_CPU);
    
    bil_dep_init(&dilasm, dilasm_dep_path,
                 targets[TARGET_DILASM], SRC_CPU);

    if (bil_dep_ischange(&lasm)) status = build_lasm(&cmd);
    if (bil_dep_ischange(&lunem)) status = build_lunem(&cmd);
    if (bil_dep_ischange(&dilasm)) status = build_dilasm(&cmd);

    if (status == -1) {
        status = BIL_EXIT_SUCCESS;
        bil_log(BIL_INFO, "No changes");
    }

    bil_cmd_clean(&cmd);
    bil_dep_clean(&lasm);
    bil_dep_clean(&lunem);
    bil_dep_clean(&dilasm);

    return status;
}