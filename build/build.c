#define BIL_IMPLEMENTATION
#include "bil.h"

#define CC      "gcc"
#define DBFLAGS "-ggdb" /* Debug flags */
#define TARGET  "bin/luna" /* Path for output executable */
#define CFLAGS  "-Wall", "-Wextra", "-flto" /* Common flags */
#define OFLAGS  "-O3", "-pipe", "-fPIE" /* Omptimizes */
#define LFLAGS  "-march=native", "-mtune=native" /* For specific machine cpu, not mendatory*/

#define DEP_PATH      "bin/dep"
#define EXAMPLE_PATH  "examples"
#define BYTECODE_PATH "examples/bytecode"

static int debug_flag; /* Debug option */
static int build_flag; /* Build option */

/* 
 * Macros LUNA_SRC and LUNA_OBJ must have the same order of files
 */
#define LUNA_SRC                                              \
    "src/luna.c", "src/arena.c", "src/core.c", "src/lexer.c", \
    "src/common.c", "src/parser.c", "src/table.c", "src/sv.c"

#define LUNA_OBJ                                              \
    "src/luna.o", "src/arena.o", "src/core.o", "src/lexer.o", \
    "src/common.o", "src/parser.o", "src/table.o", "src/sv.o"

#define LUNA_HDRS                                             \
    "src/luna.h", "src/arena.h", "src/core.h", "src/lexer.h", \
    "src/common.h", "src/parser.h", "src/table.h", "src/sv.h"

static inline void mk_important_dirs()
{
    Bil_Cstr_Array dirs = {0};
    bil_workflow_begin();
        bil_cstr_array_append(&dirs, DEP_PATH, EXAMPLE_PATH, BYTECODE_PATH);
        for (size_t i = 0; i < dirs.count; ++i) {
            if (!bil_dir_exist(dirs.items[i]))
                bil_mkdir(dirs.items[i]);
        }
    bil_workflow_end(WORKFLOW_NO_TIME);
}

/* Commong options for cmd */
static inline void cmd_build_options(Bil_Cmd *cmd)
{
    bil_cmd_append(cmd, CC);
    bil_cmd_append(cmd, CFLAGS, LFLAGS);
    if (debug_flag) bil_cmd_append(cmd, DBFLAGS);
    else bil_cmd_append(cmd, OFLAGS);
}

static inline int build_object_file(Bil_String_Builder *source, Bil_String_Builder *output)
{
    Bil_Cmd cmd = {0};
    bil_workflow_begin();
        cmd_build_options(&cmd);
        bil_cmd_append(&cmd, "-c", source->items);
        bil_cmd_append(&cmd, "-o", output->items);
        if (!bil_cmd_run_sync(&cmd)) {
            return false;
        }
    bil_workflow_end(WORKFLOW_NO_TIME);
    return true;
}

static inline void clean_targets(void)
{
    Bil_Cstr_Array obj = {0};
    bil_workflow_begin();
        bil_cstr_array_append(&obj, LUNA_OBJ);
        for (size_t i = 0; i < obj.count; ++i)
            bil_delete_file(obj.items[i]);
        bil_delete_file(TARGET);
    bil_workflow_end(WORKFLOW_NO_TIME);
    BIL_EXIT(BIL_EXIT_SUCCESS);
}

static inline void cmd_args(int *argc, char ***argv) {
    while (*argc > 0) {
        char *arg = bil_shift_args(argc, argv);
        if (arg[0] == '-') {
            switch (arg[1]) {
                case 'b': build_flag = 1; break; /* Build project whihout checking dependences */
                case 'g': debug_flag = 1; break; /* Add debug information to building */
                case 'c': clean_targets(); break; /* Clean all object files and executable */
                default: { /* Just ignore this case */
                    bil_report(BIL_WARNING, "Unknown build option '%c'", arg[1]);
                    break;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    BIL_REBUILD(argc, argv, "bin");
    cmd_args(&argc, &argv);
    bool build = false;
    mk_important_dirs(); /* Must have */
    bil_workflow_begin();
        Bil_Cmd cmd = {0};
        Bil_Dep luna = {0};
        Bil_Cstr_Array src = {0};
        Bil_Cstr_Array obj = {0};
        bil_cstr_array_append(&src, LUNA_SRC);
        bil_cstr_array_append(&obj, LUNA_OBJ);
        bil_dep_init(&luna, "bin/dep/luna.bil", LUNA_SRC, LUNA_HDRS);
        for (size_t i = 0; i < obj.count; ++i) { /* Make object file if not exist or `build` flag provided*/
            if (!bil_file_exist(obj.items[i]) || build_flag) {
                Bil_String_Builder object = bil_sb_from_cstr(obj.items[i]);
                Bil_String_Builder source = bil_sb_from_cstr(src.items[i]);
                if (!build_object_file(&source, &object)) return BIL_EXIT_FAILURE;
                build = true; /* Must rebuild project if even one object file is missing */
            }
        }
        if (build || build_flag) goto build; /* If no object files exists or `build` flag provided */
        else if (bil_dep_ischange(&luna)) { /* If some dependence is changed */
            for (size_t i = 0; i < luna.changed.count; ++i) { /* Going through all changed */
                Bil_String_Builder source = bil_sb_from_cstr(luna.changed.items[i]);
                if (source.items[source.count - 1] != 'c') continue; /* Must be .c file */
                bil_sb_join_nul(&source); /* Make terminated string */
                Bil_String_Builder object = bil_sb_from_cstr(source.items);
                bil_replace_file_extension(&object, "o");
                build_object_file(&source, &object);
                build = true;
            }
            build: /* Main build */
                cmd_build_options(&cmd);
                bil_cmd_append(&cmd, "-lm"); /* Link with math-lib */
                bil_cmd_append(&cmd, LUNA_OBJ);
                bil_cmd_append(&cmd, "-o", TARGET);
                if (!bil_cmd_run_sync(&cmd)) return BIL_EXIT_FAILURE;
        }
    if (!build) {
        bil_report(BIL_INFO, "No changes");
    }
    bil_workflow_end();
    return BIL_EXIT_SUCCESS;
}
