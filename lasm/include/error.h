#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include <stdarg.h>

#ifndef LEXER_H_
#include "lexer.h"
#endif

// TODO: better compiler error when somewhere error occured
//       remove this file to common and add errors for cpu and dilasm

typedef enum {
    LEXICAL_ERR = 0,
    PROGRAM_ERR,
    INPUT_ERR,
} error_level;

typedef struct {
    Arena *a;
    uint8_t defined;
    const char *program;
    String_View *items;
    size_t capacity;
    size_t count;
} Luna_Error;

void printse(const char *fmt, ...); // se - standart error (like stderr)

extern Luna_Error err_global;

#define ERROR PROGRAM_ERR, TOKEN_NONE
#define ERRI INPUT_ERR, TOKEN_NONE

String_View err_line(size_t line_ptr);
size_t err_pos(String_View where, String_View what);
void pr_error(error_level level, Token tk, const char *fmt, ...);


#endif // ERROR_H_