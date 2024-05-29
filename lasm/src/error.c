#include "../include/error.h"

Luna_Error err_global;

static size_t err_pos(String_View where, String_View what)
{
    size_t i = 0;
    while (i < where.count) {
        String_View tmp = sv_from_parts((char*)(where.data + i), what.count);
        if (sv_cmp(tmp, what))
            break;
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
        fprintf(stderr, "In \"%s\" at line %u in position %zu\n",
                err_global.program, tk.location, pos + 1);
    }

    switch (level) {
        case INPUT_ERR:   fprintf(stderr, "Input Error: ");   break;
        case LEXICAL_ERR: fprintf(stderr, "Lexical Error: "); break;
        case PROGRAM_ERR: fprintf(stderr, "Program Error: "); break;
        default:
            fprintf(stderr, "Error: unknown case\n");
            exit(1);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    if (!err_global.defined && tk.type != TK_NONE)
        fprintf(stderr, ", at line %u", tk.location);
    fprintf(stderr, "\n");
    va_end(args);

    if (err_global.defined && tk.type != TK_NONE) {
        fprintf(stderr, "|\n");
        fprintf(stderr, "|    "SV_Fmt"\n", SV_Args(line));
        fprintf(stderr, "|    ");
        for (size_t i = 0; i < pos; ++i)
            fprintf(stderr, " ");
        fprintf(stderr, "^\n");
    }

    if (err_global.a != NULL)
        arena_free(err_global.a);
    exit(1);
}