/*
* This `bil` file builds:
*   in dilasm `dilasm.c` - disassembly bytecode and print it in console
*   in lasm/src `lasm.c` - translate assembly code to Luna's bytecode
*   in luna/src `lunem.c` - emulate program the Luna's bytecode
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
*    -db    if flag was provided current targets builds with `-g3` and `-ggdb` for debug.
*
*    -hdlr  bil's handler flag. It simplify working with main targets (lasm, lunem, dilasm) by making path's to examples 
*           Example: type in terminal [ bin/build -hdlr lasm -i e.asm -o e.ln ]
*
*/

#define BIL_IMPLEMENTATION
#include "bil.h"

#define CC "gcc"
#define DEBUG_MODE "-g3", "-ggdb"
#define CFLAGS "-Wall", "-Wextra", "-flto"

#define CPU_INCLUDE_BIL_PATH "-Iluna/src/"
#define COMMON_INCLUDE_BIL_PATH "-Icommon/"

#define SRC_CPU      \
    "luna/src/luna.c", \
    "common/arena.c"

#define SRC_COMMON \
    "common/sv.c", \
    "common/ht.c"

#define SRC_LASM            \
    "lasm/src/compiler.c",  \
    "lasm/src/parser.c",    \
    "lasm/src/lexer.c",     \
    "lasm/src/linizer.c",   \
    "lasm/src/consts.c",    \
    "lasm/src/expr.c",      \
    "lasm/src/statement.c", \
    "lasm/src/error.c"

#define SRC_EXAMPLES             \
    "lasm/examples/e.asm",       \
    "lasm/examples/call.asm",    \
    "lasm/examples/bitwise.asm", \
    "lasm/examples/const.asm",   \
    "lasm/examples/fib.asm",     \
    "lasm/examples/ret.asm",     \
    "lasm/examples/vald.asm",    \
    "lasm/examples/stack.asm",   \
    "lasm/examples/float.asm"

#define HDRS_COMMON \
    "common/arena.h", \
    "common/sv.h", \
    "common/ht.h", \
    "common/types.h"

#define HDRS_LASM \
    "lasm/src/compiler.h", \
    "lasm/src/consts.h", \
    "lasm/src/error.h", \
    "lasm/src/expr.h", \
    "lasm/src/lexer.h", \
    "lasm/src/linizer.h", \
    "lasm/src/parser.h", \
    "lasm/src/statement.h"

#define HDRS_LUNA \
    "luna/src/luna.h"

static int debug = 0;
static int hdlr = 0;
static char *binary_dir_path = "bin";

const char *dep_dir_path = "bin/dep";
const char *bytecode_dir_path = "lasm/examples/bytecode";

const char *lasm_dep_path = "bin/dep/lasm.bil";
const char *lunem_dep_path = "bin/dep/lunem.bil";
const char *dilasm_dep_path = "bin/dep/dilasm.bil";

char *targets[] = {
    "lasm/src/lasm.c",
    "luna/src/lunem.c",
    "dilasm/dilasm.c"
};

enum {
    TARGET_LASM = 0,
    TARGET_LUNEM,
    TARGET_DILASM,
    TARGET_AMOUNT
};

char *outputs[] = {
    "bin/lasm",
    "bin/lunem",
    "bin/dilasm"
};

#define PREF_DOT "."
#define PREF_CPU "luna"
#define PREF_SRC "src"
#define PREF_LASM "lasm"
#define PREF_EXAMPLES "examples"
#define PREF_BYTECODE "examples/bytecode"

void mk_path_to_example(Bil_String_Builder *sb, char *where, int *argc, char ***argv)
{
    const char *target = bil_shift_args(argc, argv);
    *sb = BIL_PATH(PREF_LASM, where, target);
    bil_sb_join_nul(sb);
}

#define BUILD_TARGET(tar, cmd, ...)                                                     \
    do {                                                                                \
        bil_cmd_append(cmd, CC);                                                        \
        bil_cmd_append(cmd, CFLAGS);                                                    \
        bil_cmd_append(cmd, CPU_INCLUDE_BIL_PATH);                                          \
        bil_cmd_append(cmd, COMMON_INCLUDE_BIL_PATH);                                       \
        if (debug) bil_cmd_append(cmd, DEBUG_MODE);                                     \
        bil_cmd_append(cmd, "-o");                                                      \
        bil_cmd_append(cmd, outputs[tar]);                                              \
        bil_cmd_append(cmd, targets[tar]);                                              \
        bil_da_append_many(cmd,                                                         \
                           ((const char*[]){__VA_ARGS__}),                              \
                           sizeof((const char*[]){__VA_ARGS__}) / sizeof(const char*)); \
                                                                                        \
        if (!bil_cmd_run_sync(cmd)) return BIL_EXIT_FAILURE;                            \
                                                                                        \
        cmd->count = 0;                                                                 \
        return BIL_EXIT_SUCCESS;                                                        \
    } while (0)

int build_lasm(Bil_Cmd *cmd)
{
    BUILD_TARGET(TARGET_LASM, cmd, SRC_LASM, SRC_CPU, SRC_COMMON);
}

int build_lunem(Bil_Cmd *cmd)
{
    BUILD_TARGET(TARGET_LUNEM, cmd, SRC_CPU);
}

int build_dilasm(Bil_Cmd *cmd)
{
    BUILD_TARGET(TARGET_DILASM, cmd, SRC_CPU);
}

int build_examples(void)
{
    Bil_Cmd cmd = {0};
    int status = BIL_EXIT_SUCCESS;

    bil_workflow_begin();
        
        Bil_Cstr_Array examples = {0};
        bil_cstr_array_append(&examples, SRC_EXAMPLES);

        for (size_t i = 0; i < examples.count; ++i) {
            bil_cmd_append(&cmd, "bin/lasm");
            bil_cmd_append(&cmd, "-i",  examples.items[i]);

            Bil_String_Builder output = bil_sb_from_cstr(examples.items[i]);
            bil_replace_file_extension(&output, "ln");
            bil_sb_join_nul(&output);

            bil_cmd_append(&cmd, "-o", output.items);
            
            if (!bil_cmd_run_sync(&cmd))
                bil_defer_status(BIL_EXIT_FAILURE);
            
            cmd.count = 0;
        }
        
defer:
    bil_workflow_end();
    return status;
}

int cmd_args(int *argc, char ***argv)
{
    int status = -1;
    bil_workflow_begin();

        while (*argc > 0) {
            const char *flag = bil_shift_args(argc, argv);
            if (!strcmp("-db", flag)) {
                debug = 1;

            } else if (!strcmp("-hdlr", flag)) {
                hdlr = 1;
                break;

            } else if (!strcmp("-d", flag)) {
                if (strcmp(binary_dir_path, BIL_CURRENT_DIR)) {
                    Bil_String_Builder sb = BIL_PATH(PREF_DOT, binary_dir_path, DELETEME_FILE);
                    bil_sb_join_nul(&sb);
                    if (!bil_delete_file(sb.items))
                        bil_defer_status(BIL_EXIT_FAILURE);
                } else
                    if (!bil_delete_file(DELETEME_FILE))
                        bil_defer_status(BIL_EXIT_FAILURE);

                if (*argc < 1)
                    bil_defer_status(BIL_EXIT_SUCCESS);

            } else if (!strcmp("-b", flag)) {
                
                if (*argc > 0) {
                    flag = bil_shift_args(argc, argv);
                
                    if (!strcmp("only", flag)) {
                        status = 2;
                    } else if (!strcmp("examples", flag)) {
                        status = build_examples();
                        bil_defer_status(status);
                    }
                }

                size_t target = 0;
                size_t build_amount = 3;

                if (*argc > 0) {
                    flag = bil_shift_args(argc, argv);

                    if (!strcmp(flag, "lasm")) target = TARGET_LASM;
                    else if (!strcmp(flag, "lunem")) target = TARGET_LUNEM;
                    else if (!strcmp(flag, "dilasm")) target = TARGET_DILASM;
                    else {
                        bil_report(BIL_ERROR, "Unknown target for building `%s`",flag);
                        bil_defer_status(BIL_EXIT_FAILURE);
                    }

                    build_amount = target + 1;
                }

                Bil_Cmd cmd = {0};

                for (size_t i = target; i < build_amount; ++i) {
                    switch (i) {
                        case TARGET_LASM: status = build_lasm(&cmd); break;
                        case TARGET_LUNEM: status = build_lunem(&cmd); break;
                        case TARGET_DILASM: status = build_dilasm(&cmd); break;
                    }
                    if (status == BIL_EXIT_FAILURE)
                        bil_defer_status(status);
                }

                status = 2;
                bil_defer_status(status);
            }
        }

defer:
    bil_workflow_end(WORKFLOW_NO_TIME);
    if (status == 2) BIL_EXIT(BIL_EXIT_SUCCESS);
    return status;
}

void cmd_handler(int *argc, char ***argv)
{
    if (hdlr) {
        int status = BIL_EXIT_SUCCESS;
        bil_workflow_begin();

            if (*argc < 1) {
                bil_report(BIL_ERROR, "expected commands for handler");
                bil_defer_status(BIL_EXIT_FAILURE);
            }

            Bil_Cmd handler = {0};
            const char *target = bil_shift_args(argc, argv);
            Bil_String_Builder target_path = BIL_PATH(binary_dir_path, target);
            bil_sb_join_nul(&target_path);
            bil_cmd_append(&handler, target_path.items);

            if (!strcmp("lasm", target)) {
                if (*argc < 1) {
                    lasm_error:
                    bil_report(BIL_ERROR, "expected commands for `lasm`");
                    CMD("lasm/src/lasm", "-h");
                    bil_defer_status(BIL_EXIT_FAILURE);
                }

                Bil_String_Builder output_path = {0};
                Bil_String_Builder input_path = {0};

                while (*argc > 0) {
                    const char *flag = bil_shift_args(argc, argv);
                    bil_cmd_append(&handler, flag);
                    if (*argc < 1)
                        goto lasm_error;

                    if (!strcmp("-o", flag)) {
                        mk_path_to_example(&output_path, PREF_BYTECODE, argc, argv);
                        bil_cmd_append(&handler, output_path.items);

                    } else if (!strcmp("-i", flag)) {
                        mk_path_to_example(&input_path, PREF_EXAMPLES, argc, argv);
                        bil_cmd_append(&handler, input_path.items);
                    }
                }

                if (!bil_cmd_run_sync(&handler))
                    bil_defer_status(BIL_EXIT_FAILURE);

            } else if (!strcmp("lunem", target)) {
                if (*argc < 1) {
                    bil_report(BIL_ERROR, "expected commands for `lunem`");
                    CMD("lasm/src/lunem", "-h");
                    bil_defer_status(BIL_EXIT_FAILURE);
                }

                Bil_String_Builder lunem_tar_path = {0};

                while (*argc > 0) {
                    const char *flag = bil_shift_args(argc, argv);
                    bil_cmd_append(&handler, flag);
                    if (!strcmp(flag, "-i")) {
                        mk_path_to_example(&lunem_tar_path, PREF_BYTECODE, argc, argv);
                        bil_cmd_append(&handler, lunem_tar_path.items);
                    }
                }

                if (!bil_cmd_run_sync(&handler)) 
                    bil_defer_status(BIL_EXIT_FAILURE);

            } else if (!strcmp("dilasm", target)) {
                if (*argc < 1) {
                    bil_report(BIL_ERROR, "expected commands for `dilasm`");
                    bil_defer_status(BIL_EXIT_FAILURE);
                }

                Bil_String_Builder input_path = {0};
                mk_path_to_example(&input_path, PREF_BYTECODE, argc, argv);

                bil_cmd_append(&handler, input_path.items);
                if (!bil_cmd_run_sync(&handler))
                    bil_defer_status(BIL_EXIT_FAILURE);
            } else
                bil_defer_status(BIL_EXIT_FAILURE);

    defer:
        bil_workflow_end(WORKFLOW_NO_TIME);
        BIL_EXIT(status);
    }
}

void mk_important_dirs()
{
    if (!bil_dir_exist(dep_dir_path)) 
        bil_mkdir(dep_dir_path);

    if (!bil_dir_exist(bytecode_dir_path))
        bil_mkdir(bytecode_dir_path);
}

int main(int argc, char **argv)
{
    BIL_REBUILD(argc, argv, binary_dir_path);
    mk_important_dirs();

    if (cmd_args(&argc, &argv) == 1)
            BIL_EXIT(BIL_EXIT_FAILURE);

    cmd_handler(&argc, &argv);

    int status = -1;
    Bil_Cmd cmd = {0};
    Bil_Dep lasm = {0}, lunem = {0}, dilasm = {0};

    bil_workflow_begin();

        bil_dep_init(&lasm, lasm_dep_path, targets[TARGET_LASM],
                     SRC_LASM, SRC_CPU, SRC_COMMON, HDRS_COMMON, HDRS_LASM, HDRS_LUNA);

        bil_dep_init(&lunem, lunem_dep_path,
                     targets[TARGET_LUNEM], SRC_CPU, HDRS_COMMON, HDRS_LUNA);

        bil_dep_init(&dilasm, dilasm_dep_path,
                     targets[TARGET_DILASM], SRC_CPU, HDRS_COMMON, HDRS_LUNA);

        int *changes = bil_check_deps(lasm, lunem, dilasm);

        for (size_t target = 0; target < TARGET_AMOUNT; ++target) {
            if (changes[target]) {
                switch (target) {
                    case TARGET_LASM:   status = build_lasm(&cmd); break;
                    case TARGET_LUNEM:  status = build_lunem(&cmd); break;
                    case TARGET_DILASM: status = build_dilasm(&cmd); break;
                }
            }
        }

        if (status == -1) {
            status = BIL_EXIT_SUCCESS;
            bil_report(BIL_INFO, "No changes");
        }

    bil_workflow_end();
    return status;
}