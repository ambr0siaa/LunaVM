#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include <stdarg.h>

#ifndef LEXER_H_
#include "lexer.h"
#endif

typedef enum {
    LEXICAL_ERR =0,
} error_level;

void pr_error(error_level level, String_View_Array *info, Token tk, const char *fmt, ...);

#endif // ERROR_H_