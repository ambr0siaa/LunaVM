// This file build all project

#define BIL_IMPLEMENTATION
#include "./common/bil.h"

#define CC "gcc"
#define SRC "./lasm/src/lasm.c", "./cpu/src/cpu.c", "sv.c", "./common/ht.c"

#ifdef _Win32
#   define CFLAGS " "
#else 
#   define CFLAGS "-Wall", "-Wextra"
#endif

/*
* This `bil` file will build:
* in ./transalte `tsl` - translate assembly code to vm bytecode
* in ./emulate `eml` - emulate program from bytecode of vm
* in ./disasm `dis` - disassembly bytecode and print in console
* All usages in `.c` files
*/
Bil_Cmd tsl  = {0};
Bil_Cmd eml = {0};
Bil_Cmd dis = {0};

const char *sources[] = { "./lasm/translate/translate.c", "./cpu/emulate/emulate.c", "./disasm/disasm.c" };
const char *outputs[] = { "./lasm/src/translate/tsl", "./cpu/src/emulate/eml" , "./disasm/dis"};

int main(int argc, char **argv)
{
    BIL_REBUILD(argv);
    Bil_Cmd commands[] = { tsl, eml , dis };

    for (size_t i = 0; i < BIL_ARRAY_SIZE(commands); ++i) {
        bil_cmd_append(&commands[i], CC);
        bil_cmd_append(&commands[i], CFLAGS);
        bil_cmd_append(&commands[i], "-I ./common");
        bil_cmd_append(&commands[i], SRC, sources[i]);
        bil_cmd_append(&commands[i], "-o");
        bil_cmd_append(&commands[i], outputs[i]);

        if (!bil_cmd_build_sync(&commands[i])) return 1;
    }

    bil_cmd_clean(&tsl);
    bil_cmd_clean(&eml);
    bil_cmd_clean(&dis);
}