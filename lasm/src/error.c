#include "../include/error.h"

size_t err_pos(String_View where, String_View what)
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

void pr_error(error_level level, String_View_Array *info, Token tk, const char *fmt, ...)
{
    String_View line = info->items[tk.line];
    size_t pos = err_pos(line, tk.txt);

    fprintf(stderr, "At line %u in position %zu\n", tk.location, pos + 1);
    switch (level) {
        case LEXICAL_ERR:
            fprintf(stderr, "Lexical Error: ");
            break;
        default:
            fprintf(stderr, "Error: unknown case\n");
            exit(1);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "|\n");
    va_end(args);

    fprintf(stderr, "|    "SV_Fmt"\n", SV_Args(line));
    fprintf(stderr, "|    ");
    for (size_t i = 0; i < pos; ++i)
        fprintf(stderr, " ");
    fprintf(stderr, "^\n");

    exit(1);
}