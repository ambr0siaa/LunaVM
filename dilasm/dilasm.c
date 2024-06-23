#include "../luna/src/luna.h"

#define USAGE(program) \
    fprintf(stderr, "Usage: %s <input.ln>\n", (program))

static Arena arena = {0};
static Luna L = {0};

int main(int argc, char **argv)
{
    fprintf(stdout, "Note: disassembler don't work normaly on this Luna's version\n\n");
    
    const char *program = luna_shift_args(&argc, &argv);

    if (argc < 1) {
        USAGE(program);
        fprintf(stderr, "Error: %s no input file\n", program);
        return EXIT_FAILURE;
    }

    const char *input_file_path = luna_shift_args(&argc, &argv);
    load_program_from_file(&arena, &L, input_file_path);

    for (size_t i = 0; i < L.program_size; ) {
        Inst inst = L.program[i].inst;
        if (inst_has_2_regs(inst)) {
            printf("%s %s, %s\n", inst_as_cstr(inst),
                                  reg_as_cstr(L.program[i + 1].reg),
                                  reg_as_cstr(L.program[i + 2].reg));
            i += 3;

        } else if (inst_has_1_op(inst)) {
            printf("%s %"PRIu64"\n", inst_as_cstr(inst), L.program[i + 1].u64);
            i += 2;

        } else if (inst_has_no_ops(inst)) {
            printf("%s\n", inst_as_cstr(inst));
            i += 1;

        } else {
            printf("%s %s, %"PRIi64"\n", inst_as_cstr(inst),
                                 reg_as_cstr(L.program[i + 1].reg),
                                 L.program[i + 2].i64);
            i += 3;
        }
    }

    arena_free(&arena);
    return EXIT_SUCCESS;
}
