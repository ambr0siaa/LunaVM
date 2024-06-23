#ifndef SV_H_
#define SV_H_

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef HT_H_
#   include "ht.h"
#endif

typedef struct {
    char *data;
    size_t count;
} String_View;

/*
* Usage for printing sv:
*   printf(""SV_Fmt"\n", SV_Args(sv));
*/
#define SV_Fmt "%.*s"
#define SV_Args(sv) (int) (sv).count, (sv).data

String_View sv_div_by_next_symbol(String_View *sv);
String_View sv_from_cstr(char *cstr);
String_View sv_from_parts(char *data, size_t count);
String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_div_by_delim(String_View *sv, char delim);

int sv_to_int(String_View sv);
double sv_to_flt(String_View sv);
char *sv_to_cstr(String_View sv);
int char_in_sv(String_View sv, char c);
int sv_in_sv(String_View sv1, String_View sv2);
int sv_cmp(String_View sv1, String_View sv2);
int sv_is_float(String_View sv);

void sv_cut_space_left(String_View *sv);
void sv_cut_space_right(String_View *sv);
void sv_cut_left(String_View *sv, int step);
void sv_cut_right(String_View *sv, int step);
void sv_cut_while_char(String_View *sv, char sym);
void sv_append_sv(String_View *dst, String_View src);

String_View sv_cut_value(String_View *sv);
String_View sv_cut_txt(String_View *sv);

String_View sv_read_file(const char *file_path);

#endif // SV_H_
