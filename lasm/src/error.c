#include "error.h"

Luna_Error err_global;

void printse(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

size_t err_pos(String_View where, String_View what)
{
    size_t i = 0;
    while (i < where.count) {
        String_View tmp = sv_from_parts((char*)(where.data + i), what.count);
        if (sv_cmp(tmp, what)) break;
        i++;
    }

    return i;
}

String_View err_line(size_t line_ptr)
{
    if (err_global.defined) return err_global.items[line_ptr];
    else return (String_View) {
        .count = 0
    };
}

void pr_error(error_level level, Token tk, const char *fmt, ...)
{
    String_View line;
    size_t pos;

    if (err_global.defined && tk.type != TK_NONE) {
        line = err_line(tk.line);
        pos = err_pos(line, tk.txt);
        printse("In \"%s\" at line %u in position %zu\n",
                err_global.program, tk.location, pos + 1);
    }

    switch (level) {
        case INPUT_ERR:   printse("Input Error: ");   break;
        case SYNTAX_ERR:  printse("Syntax Error: ");  break;
        case LEXICAL_ERR: printse("Lexical Error: "); break;
        case PROGRAM_ERR: printse("Program Error: "); break;
        default:
            printse("Error: unknown case in `pr_error`\n");
            exit(1);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    if (!err_global.defined && tk.type != TK_NONE)
        printse(", at line %u", tk.location);
    printse("\n");
    va_end(args);

    if (err_global.defined && tk.type != TK_NONE) {
        line = sv_trim(line);
        printse("|\n");
        printse("|    "SV_Fmt"\n", SV_Args(line));
        printse("|    ");
        for (size_t i = 0; i < pos; ++i) printse(" ");
        printse("^");
        for (size_t i = 0; i < tk.txt.count - 1; ++i)
            printse("~");
        printse("\n");
    }

    if (err_global.a != NULL)
        arena_free(err_global.a);
    exit(1);
}
