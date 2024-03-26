#include "sv.h"

String_View sv_from_cstr(char *cstr)
{
    return (String_View) {
        .count = strlen(cstr),
        .data = cstr
    };
}

String_View sv_trim_left(String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i += 1;
    }

    return (String_View) {
        .count = sv.count - i,
        .data = sv.data + i
    };
}

String_View sv_trim_right(String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - i - 1])) {
        i += 1;
    }

    return (String_View) {
        .count = sv.count - i,
        .data = sv.data
    };
}

String_View sv_trim(String_View sv)
{
    return sv_trim_right(sv_trim_left(sv));
}

String_View sv_div_by_delim(String_View *sv, char delim)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }

    String_View result = {
        .count = i,
        .data = sv->data
    };

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data += i + 1;
    } else {
        sv->count -= i;
        sv->data += i;
    }

    return result;
}

int sv_cmp(String_View sv1, String_View sv2)
{
    if (sv1.count != sv2.count) {
        return 0;
    } else {
        return memcmp(sv1.data, sv2.data, sv1.count) == 0;
    }
}

int sv_to_int(String_View sv)
{
    int result = 0;
    for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); ++i) {
        result = result * 10 + sv.data[i] - '0'; 
    }
    
    return result;
}

int sv_is_float(String_View sv)
{
    int res = 0;
    for (size_t i = 0; i < sv.count; ++i) {
        if (sv.data[i] == '.') {
            res = 1;
            break;
        }
    }
    return res;
}

String_View sv_div_by_next_symbol(String_View *sv)
{
    String_View result;

    size_t i = 0;
    size_t count = 0;
    while (i < sv->count) {
        if (!isspace(sv->data[i])) {
            count++;
        }

        if (count == 2) { 
            break;
        }

        ++i;
    }

    if (i == 1) result.count = i;
    else result.count = (i - 1); 

    result.data = sv->data;
    
    sv->count -= i;
    sv->data += i;

    return result;
}

int char_in_sv(String_View sv, char c)
{   
    int result = 0;
    for (size_t i = 0; i < sv.count; ++i) {
        if (sv.data[i] == c) {
            result = 1;
            break;
        }
    }
    return result;
}

void sv_cut_space_left(String_View *sv)
{
    size_t i = 0;
    while (i < sv->count && isspace(sv->data[i])) {
        i++;
    }

    sv->count -= i;
    sv->data += i;
}

void sv_cut_space_right(String_View *sv) 
{
    size_t i = 0;
    while (i < sv->count && isspace(sv->data[sv->count - i])) {
        i++;
    }

    sv->count -= i;
}

void sv_cut_left(String_View *sv, int step)
{
    sv->count -= step;
    sv->data += step;
}

void sv_cut_right(String_View *sv, int step)
{
    sv->count -= step;
}

void sv_cut_while_char(String_View *sv, char sym)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] == sym) {
        i++;
    }
    
    sv->count -= i;
    sv->data += i;
}

String_View sv_cut_value(String_View *sv)
{
    String_View result;
    size_t i = 0;
    while (i < sv->count && (isdigit(sv->data[i]) || sv->data[i] == '.')) {
        i++;
    }

    result.data = sv->data;
    result.count = i;

    sv->count -= i;
    sv->data += i;

    return result;
}

String_View sv_cut_alpha(String_View *sv)
{
    String_View result;
    size_t i = 0;
    while (i < sv->count && isalpha(sv->data[i])) {
        i++;
    }

    result.count = i;
    result.data = sv->data;

    sv->count -= i;
    sv->data += i;

    return result;
}

void sv_append_nul(String_View *sv)
{
    sv->data[sv->count] = '\0';
}

String_View sv_cut_txt(String_View *sv, String_View special)
{
    String_View result;
    size_t i = 0;
    while (i < sv->count) {
        if (char_in_sv(special, sv->data[i]) || isspace(sv->data[i])) {
            break;
        } else {
            i++; 
        }
    }

    result.count = i;
    result.data = sv->data;

    sv->count -= i;
    sv->data += i;

    return result;
}

// return null terminated c-string
char *sv_to_cstr(String_View sv)
{
    size_t sv_size = sizeof(*sv.data) * sv.count;
    char *cstr = malloc(sv_size + 1);
    memcpy(cstr, sv.data, sv_size);
    memset(cstr + sv.count, '\0', 1);
    return cstr;
}

// needs free sv data after using
void sv_append_sv(String_View *dst, String_View src)
{
    dst->data = realloc(dst->data, sizeof(*src.data) * src.count + sizeof(*dst->data) * dst->count);
    memcpy(dst->data + dst->count, src.data, sizeof(*src.data) * src.count);
    dst->count = dst->count + src.count;
}