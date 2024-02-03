#define BIL_IMPLEMENTATION
#include "./include/bil.h"

#define CC "gcc"
#define SRC "./src/asm.c", "./src/sv.c", "./src/cpu.c", "./src/ht.c"

// This `bil` file will build:
//      in ./transalte `tsl` - translate assembly code to vm bytecode
//      in ./emulate `eml` - emulate program from bytecode of vm
//      in ./disasm `dis` - disassembly bytecode and print in console
//
// All usages in `.c` files

Bil_Cmd tsl  = {0};
Bil_Cmd eml = {0};
Bil_Cmd dis = {0};

const char *sources[] = { "./translate/translate.c", "./emulate/emulate.c", "./disasm/disasm.c" };
const char *outputs[] = { "./translate/tsl", "./emulate/eml" , "./disasm/dis"};

int main(int argc, char **argv)
{
    BIL_REBUILD(argv);
    Bil_Cmd commands[] = { tsl, eml , dis};

    for (size_t i = 0; i < BIL_ARRAY_SIZE(commands); ++i) {
        bil_cmd_append(&commands[i], CC);
        bil_cmd_append(&commands[i], SRC, sources[i]);
        bil_cmd_append(&commands[i], "-o");
        bil_cmd_append(&commands[i], outputs[i]);

        if (!bil_cmd_build_sync(&commands[i])) return 1;
    }

    bil_cmd_clean(&tsl);
    bil_cmd_clean(&eml);
    bil_cmd_clean(&dis);
}