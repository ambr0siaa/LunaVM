#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include <stdarg.h>

#ifndef LEXER_H_
#include "lexer.h"
#endif

typedef struct {
    Arena *a;
    uint8_t defined;
    const char *program;
    String_View *items;
    size_t capacity;
    size_t count;
} Luna_Error;

extern Luna_Error err_global;

typedef enum {
    LEXICAL_ERR = 0,
    PROGRAM_ERR,
    INPUT_ERR,
} error_level;

#define ERROR PROGRAM_ERR, TOKEN_NONE
#define ERRI INPUT_ERR, TOKEN_NONE

String_View err_line(size_t line_ptr);
void pr_error(error_level level, Token tk, const char *fmt, ...);

#endif // ERROR_H_