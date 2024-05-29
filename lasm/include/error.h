#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include <stdarg.h>

#ifndef LEXER_H_
#include "lexer.h"
#endif

typedef struct {
    uint8_t defined;
    const char * program;
    String_View *items;
    size_t capacity;
    size_t count;
} Program_Error;

extern Program_Error err_global;

typedef enum {
    LEXICAL_ERR = 0,
} error_level;

String_View err_line(size_t line_ptr);
void pr_error(error_level level, Token tk, const char *fmt, ...);

#endif // ERROR_H_