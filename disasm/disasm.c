#include "../lasm/lasm.h"

#define USAGE(program) \
    fprintf(stderr, "Usage: %s <input.ven>\n", (program))

static CPU cpu = {0};

int main(int argc, char **argv) 
{
    const char *program = luna_shift_args(&argc, &argv);

    if (argc < 1) {
        USAGE(program);
        fprintf(stderr, "Error: %s expected input\n", program);
        exit(1);
    }

    const char *input_file_path = luna_shift_args(&argc, &argv);
    load_program_from_file(&cpu, input_file_path);
    
    for (size_t i = 0; i < cpu.program_size; ) {
        Inst inst = cpu.program[i].inst;
        if (inst_has_2_regs(inst)) {
            printf("%s %s, %s\n", inst_as_cstr(inst), 
                                  reg_as_cstr(cpu.program[i + 1].reg), 
                                  reg_as_cstr(cpu.program[i + 2].reg));
            i += 3;       

        } else if (inst_has_1_op(inst)) {
            printf("%s %lu\n", inst_as_cstr(inst), cpu.program[i + 1].u64);
            i += 2;

        } else if (inst_has_no_ops(inst)) {
            printf("%s\n", inst_as_cstr(inst));
            i += 1;

        } else {
            printf("%s %s, %li\n", inst_as_cstr(inst),
                                 reg_as_cstr(cpu.program[i + 1].reg),
                                 cpu.program[i + 2].i64);
            i += 3;
        }
    }
}