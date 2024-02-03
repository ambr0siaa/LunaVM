#ifndef SV_H_
#define SV_H_

#include <string.h>
#include <ctype.h>

#define SV_Fmt "%.*s"
#define SV_Args(sv) (int) (sv).count, (sv).data

typedef struct {
    char *data;
    size_t count;
} String_View;

extern String_View sv_from_cstr(char *cstr);
extern String_View sv_trim_left(String_View sv);
extern String_View sv_trim_right(String_View sv);
extern String_View sv_trim(String_View sv);
extern String_View sv_div_by_delim(String_View *sv, char delim);
extern int sv_cmp(String_View sv1, String_View sv2);
extern int sv_to_int(String_View sv);
extern void sv_append_nul(String_View *sv);

#endif // SV_H_