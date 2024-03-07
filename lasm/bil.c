#define BIL_IMPLEMENTATION
#include "../common/bil.h"

int main(int argc, char **argv)
{
    BIL_REBUILD(argv);
    Bil_Cmd cmd = {0};

    bil_cmd_append(&cmd, "gcc");
    bil_cmd_append(&cmd, "../cpu/src/cpu.c", "./src/lasm.c");
    bil_cmd_append(&cmd, "../common/ht.c", "../common/sv.c");
    bil_cmd_append(&cmd, "./src/translate/translate.c");
    bil_cmd_append(&cmd, "-o", "./src/tsl");

    if (!bil_cmd_build_sync(&cmd)) return 1;

    return 0;
}