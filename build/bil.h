/*
*  This custom build system is like MakeFile, Cmake and etc
*  but implemented as single header style C library (stb-style).
*  Library can by used on Linux and Windows.
*  Project has inspired by `https://github.com/tsoding/nobuild`
* 
*  Copyright (c) 2024 Matthew Sokolkin <sokolkin227@gmail.com>
* 
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
* 
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.
* 
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
*/

#ifndef BIL_H_
#define BIL_H_

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <fcntl.h>
#    include <unistd.h>
#    include <sys/wait.h>
#    include <sys/stat.h>
#    include <sys/time.h>
#    include <sys/types.h>
#endif

#define BIL_DA_INIT_CAPACITY 256

#define BIL_ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define BIL_DA_SIZE(da) (da)->capacity * sizeof(*(da)->items)

#define BIL_ASSERT assert
#define BIL_FREE free
#define BIL_EXIT exit
#define BIL_REALLOC bil_context_realloc

#define BIL_INFO_COLOR "\x1b[1m"
#define BIL_ERROR_COLOR "\x1b[38;5;9m"
#define BIL_NORMAL_COLOR "\x1b[0m"
#define BIL_WARNING_COLOR "\x1b[1m"

#define BIL_EXIT_SUCCESS 0
#define BIL_EXIT_FAILURE 1

#ifndef BIL_REBUILD_COMMAND
#  define BIL_REBUILD_CFLAGS "-Wall", "-Wextra", "-flto", "-O3", "-fPIE", "-pipe"
#  if _WIN32
#    if defined(__GNUC__)
#       define BIL_REBUILD_COMMAND(source_path, output_path) "gcc", "-o", (output_path), (source_path), BIL_REBUILD_CFLAGS
#    elif defined(__clang__)
#       define BIL_REBUILD_COMMAND(source_path, output_path) "clang", "-o", (output_path), (source_path), BIL_REBUILD_CFLAGS
#    endif
#  else
#       define BIL_REBUILD_COMMAND(source_path, output_path) "gcc", "-o", (output_path), (source_path), BIL_REBUILD_CFLAGS
#  endif
#endif /* BIL_REBUILD_COMMAND */

#ifdef _WIN32
typedef HANDLE Bil_Proc;
#define BIL_INVALID_PROC INVALID_HANDLE_VALUE
#else
typedef int Bil_Proc;
#define BIL_INVALID_PROC (-1)
#endif

#define bil_da_append(da, new_item)                                                        \
    do {                                                                                   \
        if ((da)->count + 1 >= (da)->capacity) {                                           \
            size_t old_size = BIL_DA_SIZE((da));                                           \
            (da)->capacity = (da)->capacity > 0 ? (da)->capacity*2 : BIL_DA_INIT_CAPACITY; \
            (da)->items = BIL_REALLOC((da)->items, old_size, BIL_DA_SIZE((da)));           \
            BIL_ASSERT((da)->items != NULL);                                               \
        }                                                                                  \
        (da)->items[(da)->count++] = (new_item);                                           \
    } while (0)

#define bil_da_append_many(da, new_items, items_count)                                      \
    do {                                                                                    \
        if ((da)->count + (items_count) >= (da)->capacity) {                                \
            size_t old_size = BIL_DA_SIZE((da));                                            \
            if ((da)->capacity == 0) (da)->capacity = BIL_DA_INIT_CAPACITY;                 \
            while ((da)->count + (items_count) >= (da)->capacity) { (da)->capacity *= 2; }  \
            (da)->items = BIL_REALLOC((da)->items, old_size, BIL_DA_SIZE((da)));            \
            BIL_ASSERT((da)->items != NULL);                                                \
        }                                                                                   \
        memcpy((da)->items + (da)->count, (new_items), (items_count)*sizeof(*(da)->items)); \
        (da)->count += (items_count);                                                       \
    } while (0)


#define bil_da_append_da(dst, src)                                                               \
    do {                                                                                         \
        if ((dst)->count + (src)->count >= (dst)->capacity) {                                    \
            size_t old_size = BIL_DA_SIZE((dst));                                                \
            if ((dst)->capacity == 0) (dst)->capacity = BIL_DA_INIT_CAPACITY;                    \
            while ((dst)->count + (src)->count >= (dst)->capacity) { (dst)->capacity *= 2; }     \
            (dst)->items = BIL_REALLOC((dst)->items, old_size, BIL_DA_SIZE((dst)));              \
            BIL_ASSERT((dst)->items != NULL);                                                    \
        }                                                                                        \
        memcpy((dst)->items + (dst)->count, (src)->items, (src)->count * sizeof(*(src)->items)); \
        (dst)->count += (src)->count;                                                            \
    } while (0)

#define bil_da_clean(da)                              \
    do {                                              \
        if ((da)->items != NULL && bil_alloc_flag) {  \
            BIL_FREE((da)->items);                    \
            (da)->items = NULL;                       \
            (da)->capacity = 0;                       \
            (da)->count = 0;                          \
        }                                             \
    } while (0)

typedef enum {
    BIL_INFO = 0,
    BIL_ERROR,
    BIL_WARNING
} bil_report_flags;

void bil_report(bil_report_flags flag, char *fmt, ...);

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Bil_Cstr_Array;

void bil_cstr_arr_clean(Bil_Cstr_Array *arr);
void bil_cstr_arr_append(Bil_Cstr_Array *arr, char *item);                                  /* Appends only one item */
void bil_cstr_arr_append_many(Bil_Cstr_Array *arr, const char **items, size_t items_count); /* Appends fixed count of items */

/* Appends variable count of items */
#define bil_cstr_array_append(arr, ...) \
    bil_da_append_many((arr), ((const char *[]){__VA_ARGS__}), sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char*))

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} Bil_String_Builder;

#define SB_Args(sb) (int) (sb).count, (sb).items
#define SB_Fmt "%.*s"

void bil_sb_join_many(Bil_String_Builder *sb, ...);
void bil_sb_join_cstr(Bil_String_Builder *sb, const char *cstr);

Bil_String_Builder bil_sb_from_cstr(char *cstr);

#define bil_sb_clean(sb)     bil_da_clean(sb);
#define bil_sb_join_nul(sb)  bil_da_append_many(sb, "", 1)
#define SB_JOIN(sb,...)      bil_sb_join_many((sb), __VA_ARGS__, NULL)

typedef struct {
    char *data;
    size_t count;
} Bil_String_View;

#define SV_Fmt SB_Fmt
#define SV_Args(sv) (int) (sv).count, (sv).data

Bil_String_View bil_sv_from_cstr(char *cstr);
Bil_String_View bil_sv_cut_value(Bil_String_View *sv);
Bil_String_View bil_sv_chop_by_space(Bil_String_View *sv);

void bil_sv_cut_space_left(Bil_String_View *sv);

int bil_char_in_sv(Bil_String_View sv, char c);
int bil_sv_cmp(Bil_String_View s1, Bil_String_View s2);

uint32_t bil_sv_to_u32(Bil_String_View sv);
char *bil_sv_to_cstr(Bil_String_View sv);

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Bil_Cmd;

/* Single call of command, without providing `Bil_Cmd` structure */
#define CMD(...) \
    cmd_single_run(((const char *[]){__VA_ARGS__}), sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char*))

#define bil_cmd_append(cmd, ...) \
    bil_da_append_many(cmd, ((const char *[]){__VA_ARGS__}), sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char*))

#define bil_cmd_clean(cmd) bil_da_clean(cmd)
#define bil_cmd_reset(cmd) do (cmd).count = 0; while(0)
#define bil_cmd_append_cmd(dst, src) bil_da_append_da(dst, src)

void cmd_single_run(const char **args, size_t args_count);

Bil_String_Builder bil_cmd_create(Bil_Cmd *cmd);

Bil_Proc bil_cmd_run_async(Bil_Cmd *cmd);
bool bil_cmd_run_sync(Bil_Cmd *cmd);
bool bil_proc_await(Bil_Proc proc);

#ifdef _WIN32
#define Bil_FileTime FILETIME
/* convert FILETIME to long int */ 
#define bil_ft2li(t, dst) \
    do { \
        (dst) = (long int)((uint64_t)((t).dwHighDateTime) << 32 | (t).dwLowDateTime); \
        (dst) = (dst) >= 0 ? (dst) : (-1)*(dst); \
    } while (0)
#else
#define Bil_FileTime time_t
#endif

struct bil_dep_info {
    uint32_t id;
    Bil_FileTime t;
#ifdef _WIN32
    long int ltime;     /* load time from parser */
#endif
};

/* TODO: binary search */
typedef struct {
    struct bil_dep_info *items;
    size_t capacity;
    size_t count;
} Bil_Deps_Info;

/*
*  Bil_Dep - bil's dependence. Uses for keep tracking of dependent files.
*  Takes many dependences and 1 output file where writes info about depenences.
*  Output file I prefer name's with extention `.bil` but you can also use diffrent extention.
*
*  Usage for dependences:
*      Bil_Dep dep = {0};
*      bil_dep_init(&dep, <output file path>, <dependeces>);
*/
typedef struct {
    Bil_Cstr_Array deps;
    Bil_Cstr_Array changed;
    const char *output_file;
} Bil_Dep;

bool bil_dep_ischange(Bil_Dep *dep);                         /* the main workhorse function for dependencies */
int *bil_deps_ischange(const Bil_Dep *deps, size_t count);   /* checks many dependeces */

/* returns array of ints with `bil_dep_ischange` result respectively provided dependences */
#define bil_check_deps(...) \
    bil_deps_ischange(((const Bil_Dep[]){__VA_ARGS__}), sizeof((const Bil_Dep[]){__VA_ARGS__}) / sizeof(const Bil_Dep))

uint32_t bil_file_id(char *file_path);
Bil_FileTime bil_file_last_update(const char *path);

Bil_Deps_Info bil_parse_dep(char *src);
bool bil_dep_write(const char *path, Bil_String_Builder *dep);

Bil_String_Builder bil_mk_dependence_file(Bil_Cstr_Array deps);
Bil_String_Builder bil_mk_dependence_from_info(Bil_Deps_Info *info);

void bil_change_dep_info(Bil_Deps_Info *info, struct bil_dep_info new_info);
bool bil_deps_info_search(Bil_Deps_Info *info, uint32_t id, struct bil_dep_info *target); 

#define bil_dep_append(dep, ...)                                                    \
    bil_cstr_arr_append_many(&(dep)->deps, ((const char*[]){__VA_ARGS__}),          \
                             sizeof((const char*[]){__VA_ARGS__})/sizeof(char*))

#define bil_dep_init(dep, output_file_path, ...)                                  \
    do {                                                                          \
        (dep)->output_file = output_file_path;                                    \
        bil_cstr_arr_append_many(&(dep)->deps,                                    \
                             ((const char*[]){__VA_ARGS__}),                      \
                             sizeof((const char*[]){__VA_ARGS__})/sizeof(char*)); \
    } while (0)


#define bil_dep_clean(dep) bil_cstr_arr_clean(&(dep)->deps)

#define DELETEME_FILE "DELETEME"
#define BIL_CURRENT_DIR "cur"

bool bil_check_for_rebuild(const char *output_file_path, const char *source_file_path);

/*
*  Macro for rebuilding building executable file when it provided.
*  After rebuilding old file will rename to `DELETME` and run.
*/
#define BIL_REBUILD(argc, argv, deleteme_dir)                                                           \
    do {                                                                                                \
        bil_workflow_begin();                                                                           \
            int status = -1;                                                                            \
            const char *output_file_path = (argv[0]);                                                   \
            const char *source_file_path = __FILE__;                                                    \
            bool rebuild_is_need = bil_check_for_rebuild(output_file_path, source_file_path);           \
            if (rebuild_is_need) {                                                                      \
                Bil_String_Builder deleteme_bil_sb_path = {0};                                          \
                const char *deleteme_path;                                                              \
                int is_current = strcmp(deleteme_dir, BIL_CURRENT_DIR);                                 \
                if (is_current) {                                                                       \
                    deleteme_bil_sb_path = BIL_PATH(".", deleteme_dir, DELETEME_FILE);                  \
                    deleteme_path = deleteme_bil_sb_path.items;                                         \
                } else {                                                                                \
                    deleteme_path = DELETEME_FILE;                                                      \
                }                                                                                       \
                if (!bil_rename_file(output_file_path, deleteme_path)) extra_exit();                    \
                if (is_current) bil_sb_clean(&deleteme_bil_sb_path);                                    \
                Bil_Cmd rebuild_cmd = {0};                                                              \
                bil_cmd_append(&rebuild_cmd, BIL_REBUILD_COMMAND(source_file_path, output_file_path));  \
                if (!bil_cmd_run_sync(&rebuild_cmd)) bil_defer_status(BIL_EXIT_FAILURE);                \
                bil_report(BIL_INFO, "rebuild complete");                                               \
                bil_cmd_clean(&rebuild_cmd);                                                            \
                Bil_Cmd cmd = {0};                                                                      \
                bil_da_append_many(&cmd, argv, argc);                                                   \
                status = bil_cmd_run_sync(&cmd);                                                        \
                bil_cmd_clean(&cmd);                                                                    \
                bil_defer_status(!status);                                                              \
            }                                                                                           \
    defer:                                                                                              \
        bil_workflow_end(WORKFLOW_NO_TIME);                                                             \
        if (status != -1) BIL_EXIT(status);                                                             \
    } while (0)

/* Using for cutting file extension */
#define BIL_DOT_NEED    1
#define BIL_DOT_NOTNEED 0

void bil_mkdir(const char *name);
bool bil_dir_exist(const char *dir_path);

bool bil_file_exist(const char *file_path);
bool bil_delete_file(const char *file_path);
bool bil_read_file(const char *file_path, char **dst);
bool bil_read_entire_file(const char *file_path, Bil_String_View *dst);
bool bil_rename_file(const char *file_path, const char *new_path);
void bil_cut_file_extension(Bil_String_Builder *file, bool dot);
void bil_replace_file_extension(Bil_String_Builder *file, const char *ext);

char *bil_shift_args(int *argc, char ***argv);

Bil_String_Builder bil_mk_path(char *file, ...);
#define BIL_PATH(file, ...) bil_mk_path(file, __VA_ARGS__, NULL)

/*
*  Using for workflow context.
*  When provided `bil_workflow_begin` build system use's context alloc
*  if not then build system use's default malloc
*/
static int bil_alloc_flag;

/* Memmory region for context alloc (its like arenas) */
typedef struct Bil_Region Bil_Region;

#define BIL_ALIGNMENT sizeof(void*)
#define BIL_REGION_DEFAULT_CAPACITY (1024)
#define BIL_ASIZE_CMP(size) ((size) < BIL_REGION_DEFAULT_CAPACITY ? BIL_REGION_DEFAULT_CAPACITY : (size))

struct Bil_Region {
    size_t capacity;
    size_t alloc_pos;
    Bil_Region *next;
    char *data;
};

Bil_Region *bil_region_create(size_t region_size);

void *bil_context_alloc(size_t size);
void *bil_context_realloc(void *ptr, size_t old_size, size_t new_size);

typedef struct WfContext WfContext; /* Workflow Context */

struct WfContext {
    struct timeval begin;
    struct timeval end;
    Bil_Region *r_head;
    Bil_Region *r_tail;
    WfContext *next;
};

#ifdef _WIN32
int gettimeofday(struct timeval * tp);
#endif

#define bil_defer_status(s) do { status = (s); goto defer; } while(0)
#define WORKFLOW_NO_TIME 1

typedef struct {
    WfContext *head;
    WfContext *tail;
} Bil_Workflow;

/*
*  Workflow is special memmory region which simplify working with memmory allocations.
*  You also don't have to use workflow but if don't use it, you should clean everything up by hands.
*
*  For using workflow context put `bil_workflow_begin` (start of workflow context) next put `bil_workflow_end` (end of workflow context).
*  In bitween of this functions do some stuff and don't care about memmory leaks current workflow context will care about it.
*  
*  Workflow contexts can be define in other workflow context.
* 
*  Example:
*        void somefunc() {
*            ...
*            bil_workflow_begin();
*                
*                some building job
*
*            bil_workflow_end();
*            ...
*        }
*/

static Bil_Workflow *workflow = NULL;

double workflow_time_taken(struct timeval end, struct timeval begin);

void bil_workflow_begin();
void workflow_end(const int *args, int argc);
void extra_exit();

#define bil_workflow_end(...) \
    workflow_end(((const int[]){__VA_ARGS__}), sizeof((const int[]){__VA_ARGS__}) / sizeof(const int));

#endif /* BIL_H_ */

#ifdef BIL_IMPLEMENTATION

void bil_cut_file_extension(Bil_String_Builder *file, bool dot)
{
    size_t i = 0;
    while (i < file->count && file->items[file->count - i - 1] != '.') ++i;
    if (!dot) --i;
    file->count -= i;
}

void bil_replace_file_extension(Bil_String_Builder *file, const char *ext)
{
    bil_cut_file_extension(file, BIL_DOT_NEED);
    bil_sb_join_cstr(file, ext);
}

void extra_exit()
{
    if (workflow != NULL) {
        WfContext *wfc = workflow->head;
        while (wfc != NULL) {
            Bil_Region *r = wfc->r_head;
            while (r != NULL) {
                Bil_Region *next = r->next;
                free(r->data);
                free(r);
                r = next;
            }
            WfContext *next = wfc->next;
            free(wfc);
            wfc = next;
        } 
    } else
        bil_report(BIL_WARNING, "Some allocations may not cleaned");
    BIL_EXIT(BIL_EXIT_FAILURE);
}

#ifdef _WIN32
/* Stolen from https://stackoverflow.com/questions/10905892/equivalent-of-gettimeofday-for-windows */
int gettimeofday(struct timeval *tp)
{
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);
    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;
    GetSystemTime(&system_time);
    SystemTimeToFileTime( &system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;
    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

Bil_Region *bil_region_create(size_t region_size)
{
    Bil_Region *r = malloc(sizeof(Bil_Region));
    r->data = malloc(region_size);
    r->capacity = region_size;
    r->alloc_pos = 0;
    r->next = NULL;
    return r;
}

void *bil_context_alloc(size_t size)
{
    if (workflow == NULL) {
        bil_alloc_flag = 1;
        bil_report(BIL_WARNING, "Default malloc is used. Keep memory clean!");
        return malloc(size);
    }
    bil_alloc_flag = 0;
    WfContext *actual = workflow->tail;
    Bil_Region *cur = actual->r_head;
    size_t align_size = (size + (BIL_ALIGNMENT - 1)) & ~(BIL_ALIGNMENT - 1);
    for(;;) {
        if (cur == NULL) {
            Bil_Region* r = bil_region_create((size) < BIL_REGION_DEFAULT_CAPACITY ?
                                              BIL_REGION_DEFAULT_CAPACITY : (size));
            actual->r_tail->next = r;
            actual->r_tail = r;
            cur = actual->r_tail;
        }
        if (cur->alloc_pos + align_size <= cur->capacity) {
            char *ptr = (char*)(cur->data + cur->alloc_pos);
            memset(ptr, 0, align_size);
            cur->alloc_pos += align_size;
            return ptr;
        } else {
            cur = cur->next;
            continue;
        }
    }
}

void *bil_context_realloc(void *ptr, size_t old_size, size_t new_size)
{
    if (new_size > old_size) {
        void *new_ptr = bil_context_alloc(new_size);
        memcpy(new_ptr, ptr, old_size);
        return new_ptr;
    } else {
        return ptr;
    }
}

void bil_workflow_begin()
{
    WfContext *wfc = malloc(sizeof(WfContext));
    wfc->end = (struct timeval) {0};
#ifdef _WIN32
    gettimeofday(&wfc->begin);
#else
    gettimeofday(&wfc->begin, NULL);
#endif
    Bil_Region *r = bil_region_create(BIL_REGION_DEFAULT_CAPACITY);
    wfc->r_head = r;
    wfc->r_tail = r;
    wfc->next = NULL;
    if (workflow == NULL) {
        workflow = malloc(sizeof(Bil_Workflow));
        workflow->head = wfc;
        workflow->tail = wfc;
    } else {
        workflow->tail->next = wfc;
        workflow->tail = wfc;
    }
}

double workflow_time_taken(struct timeval end, struct timeval begin)
{
    double time_taken = (end.tv_sec - begin.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec - begin.tv_usec)) * 1e-6;
    return time_taken;
}

void workflow_end(const int *args, int argc)
{
    if (workflow == NULL) {
        bil_report(BIL_WARNING, "Cannot clear workflow that not exist. All workflows have already done or haven't begun\n");
        return;
    }
    WfContext *wfc = workflow->tail;
    if (argc < 1) {
    workflow_time:
    #ifdef _WIN32
        gettimeofday(&wfc->end);
    #else
        gettimeofday(&wfc->end, NULL);
    #endif
        double time_taken = workflow_time_taken(wfc->end, wfc->begin);
        bil_report(BIL_INFO, "Workflow has taken %lf sec", time_taken);
    } else if (argc > 0 && args[0] != WORKFLOW_NO_TIME) {
        goto workflow_time;
    }
    Bil_Region *r = wfc->r_head;
    while(r != NULL) {
        Bil_Region *r_next = r->next;
        free(r->data);
        free(r);
        r = r_next;
    }
    WfContext *cur = workflow->head;
    if (cur == wfc) {
        free(wfc);
        free(workflow);
        workflow = NULL;
        return;
    }
    while (cur->next != wfc) cur = cur->next;
    free(wfc);
    cur->next = NULL;
    workflow->tail = cur;
}

uint32_t bil_sv_to_u32(Bil_String_View sv)
{
    uint32_t result = 0;
    for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); ++i)
        result = result * 10 + sv.data[i] - '0';
    return result;
}

char *bil_sv_to_cstr(Bil_String_View sv)
{
    char *cstr = bil_context_alloc(sv.count + 1);
    memcpy(cstr, sv.data, sv.count);
    cstr[sv.count] = '\0';
    return cstr;
}

Bil_String_View bil_sv_from_cstr(char *cstr)
{
    return (Bil_String_View) {
        .count = strlen(cstr),
        .data = cstr
    };
}

Bil_String_View bil_sv_cut_value(Bil_String_View *sv)
{
    Bil_String_View result;
    size_t i = 0;
    while (i < sv->count && (isdigit(sv->data[i]) || sv->data[i] == '.')) ++i;
    result.data = sv->data;
    result.count = i;
    sv->count -= i;
    sv->data += i;
    return result;
}

int bil_char_in_sv(Bil_String_View sv, char c)
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

void bil_sv_cut_space_left(Bil_String_View *sv)
{
    size_t i = 0;
    while (i < sv->count && isspace(sv->data[i])) ++i;
    sv->count -= i;
    sv->data += i;
}

int bil_sv_cmp(Bil_String_View s1, Bil_String_View s2)
{
    if (s1.count == s2.count) {
        return memcmp(s1.data, s2.data, s1.count) == 0;
    } else {
        return 0;
    }
}

Bil_String_View bil_sv_chop_by_space(Bil_String_View *sv)
{
    Bil_String_View result = {0};
    size_t i = 0;
    while (i < sv->count && !isspace(sv->data[i])) ++i;
    result.data = sv->data;
    result.count = i;
    sv->count -= i;
    sv->data += i;
    return result;
}

void bil_sv_cut_left(Bil_String_View *sv, int shift)
{
    sv->data += shift;
    sv->count -= shift;
}

Bil_Deps_Info bil_parse_dep(char *src)
{
    Bil_Deps_Info info = {0};
    Bil_String_View source = bil_sv_from_cstr(src);
    while (source.count > 0 && source.data[0] != '\0') {
        struct bil_dep_info i = {0};
        if (isdigit(source.data[0])) {
            i.id = bil_sv_to_u32(bil_sv_cut_value(&source));
            bil_sv_cut_left(&source, 1);
            if (source.data[0] == '-')
                bil_sv_cut_left(&source, 1);
            #ifdef _WIN32
            i.ltime = bil_sv_to_u32(bil_sv_cut_value(&source));
            #else
            i.t = bil_sv_to_u32(bil_sv_cut_value(&source));
            #endif
            bil_sv_cut_left(&source, 1);
        } else if (source.data[0] == '-') {
            bil_sv_cut_left(&source, 1);
        } else {
            bil_report(BIL_ERROR, "unknown character `%c`", source.data[0]);
            extra_exit();
        }
        bil_da_append(&info, i);
    }
    return info;
}

void bil_cstr_arr_append_many(Bil_Cstr_Array *arr, const char **items, size_t items_count)
{
    bil_da_append_many(arr, items, items_count);
}

void bil_cstr_arr_append(Bil_Cstr_Array *arr, char *item)
{
    bil_da_append(arr, item);
}

void bil_cstr_arr_clean(Bil_Cstr_Array *arr)
{
    bil_da_clean(arr);
}

void bil_mkdir(const char *dir_path)
{
    bil_report(BIL_INFO, "Make directory %s", dir_path);
#ifdef _WIN32
    if (CreateDirectoryA(dir_path, NULL) == 0) {
        bil_report(BIL_ERROR, "cannot create directory `%s`: %lu",
                dir_path, GetLastError());
        extra_exit();
    }
#else
    if (mkdir(dir_path, 0777) < 0) {
        bil_report(BIL_ERROR, "cannot create directory `%s`: %s\n",
                dir_path, strerror(errno));
        extra_exit();
    }
#endif
}

bool bil_dir_exist(const char *dir_path)
{
    bool result = true;
    DIR *dir = opendir(dir_path);
    if (dir) {
        result = true;
    } else if (ENOENT == errno) {
        result = false;
    } else {
        bil_report(BIL_ERROR, "file `%s` not a directory\n", dir_path);
        result = false;
    }
    closedir(dir);
    return result;
}

/* TODO: not implemented */
void bil_wildcard()
{
    DIR *d;
    struct dirent *dir;
    
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
}

bool bil_read_file(const char *file_path, char **dst)
{
    bool status = true;
    FILE *f = fopen(file_path, "r");
    if (!f) bil_defer_status(false);
    if (fseek(f, 0, SEEK_END) < 0)
        bil_defer_status(false); 
    long int file_size = ftell(f);
    if (file_size < 0)
        bil_defer_status(false);
    if (fseek(f, 0, SEEK_SET) < 0)
        bil_defer_status(false);
    char *buf = bil_context_alloc(file_size + 1);
    if (!buf) bil_defer_status(false);
    size_t buf_len = fread(buf, 1, file_size, f);
    buf[buf_len] = '\0';
    if (ferror(f))
        bil_defer_status(false);
    *dst = buf;
defer:
    fclose(f);
    if (status != true)
        bil_report(BIL_ERROR, "cannot read from `%s`: %s",
                file_path, strerror(errno));
    return status;
}

bool bil_read_entire_file(const char *file_path, Bil_String_View *dst)
{
    char *content;
    if (!bil_read_file(file_path, &content))
        return false;
    *dst = bil_sv_from_cstr(content);
    return true;
}

Bil_FileTime bil_file_last_update(const char *path)
{
    Bil_FileTime result;
    if (!bil_file_exist(path)) {
        bil_report(BIL_ERROR, "cannot find dependence by path `%s`", path);
        extra_exit();
    }
#ifdef _WIN32
    BOOL Proc;
    HANDLE file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (file == BIL_INVALID_PROC) {
        bil_report(BIL_ERROR, "cannot open `%s`: %lu", path, GetLastError());
        extra_exit();
    }
    Bil_FileTime file_time;
    Proc = GetFileTime(file, NULL, NULL, &file_time);
    CloseHandle(file);
    if (!Proc) {
        bil_report(BIL_ERROR, "cannot get `%s` time: %lu", path, GetLastError());
        extra_exit();
    }
    result = file_time;
#else
    struct stat statbuf = {0};
    if (stat(path, &statbuf) < 0) {
        bil_report(BIL_ERROR, "cannot get info about `%s`: %s", strerror(errno));
        extra_exit();
    }
    result = statbuf.st_ctime;
#endif
    return result;
}

bool bil_dep_write(const char *path, Bil_String_Builder *dep)
{
    bool status = true;
    FILE *f = fopen(path, "w");
    if (!f) {
        bil_report(BIL_ERROR, "cannot open file by path `%s`", path);
        bil_defer_status(false);
    }
    fwrite(dep->items, sizeof(dep->items[0]), sizeof(dep->items[0]) * dep->count, f);
    if (ferror(f)) {
        bil_report(BIL_ERROR, "cannot write to `%s` file", path);
        bil_defer_status(false);
    }
defer:
    fclose(f);
    return status;
}

/* Check summ CRC32B */
uint32_t bil_file_id(char *file_path)
{
    size_t i = 0;
    uint32_t id = 0xFFFFFFFF, mask;
    while (file_path[i]) {
        id ^= (uint32_t)(file_path[i]);
        for (int j = 7; j >= 0; --j) {
            mask = -(id & 1);
            id = (id >> 1) ^ (0xEDB88320 & mask);
        }
        ++i;
    }
    return ~id;
}

Bil_String_Builder bil_mk_dependence_file(Bil_Cstr_Array deps)
{
    Bil_String_Builder sb = {0};
    for (size_t i = 0; i < deps.count; ++i) {
        char buf[256];
        char *file_path = deps.items[i];
        Bil_FileTime t = bil_file_last_update(file_path);
        long int time;
        #ifdef _WIN32
        bil_ft2li(t, time);
        #else
        time = t;
        #endif
        uint32_t id = bil_file_id(file_path);
        snprintf(buf, sizeof(buf), "%u %li\n", id, time);
        bil_sb_join_cstr(&sb, buf);
    }
    bil_sb_join_nul(&sb);
    return sb;
}

bool bil_deps_info_search(Bil_Deps_Info *info, uint32_t id, struct bil_dep_info *target)
{
    bool result = false;
    for (size_t i = 0; i < info->count; ++i) {
        if (info->items[i].id == id) {
            *target = info->items[i];
            result = true;
            break;
        }
    }
    return result;
}

void bil_change_dep_info(Bil_Deps_Info *info, struct bil_dep_info new_info)
{
    for (size_t i = 0; i < info->count; ++i) {
        if (new_info.id == info->items[i].id) {
            info->items[i].t = new_info.t;
            #ifdef _WIN32
            bil_ft2li(new_info.t, info->items[i].ltime);
            #endif
            break;
        }
    }
    info = info;
}

Bil_String_Builder bil_mk_dependence_from_info(Bil_Deps_Info *info)
{
    Bil_String_Builder sb = {0};
    for (size_t i = 0; i < info->count; ++i) {
        char buf[256];
        struct bil_dep_info new = info->items[i];
        long int time;
        #ifdef _WIN32
        time = new.ltime;
        time = time >= 0 ? time : (-1) * time;
        #else
        time = new.t;
        #endif
        snprintf(buf, sizeof(buf), "%u %li\n", info->items[i].id, time);
        bil_sb_join_cstr(&sb, buf);
    }
    bil_sb_join_nul(&sb);
    return sb;
}

bool bil_dep_ischange(Bil_Dep *dep)
{
    bool result = false;
    const char *update = NULL;
    if (!bil_file_exist(dep->output_file)) {
    dep_write:
        Bil_String_Builder out = bil_mk_dependence_file(dep->deps);
        bil_dep_write(dep->output_file, &out);
        if (update) {
            bil_report(BIL_INFO, "Updated dependence output `%s`", dep->output_file);
        } else {
            bil_report(BIL_INFO, "Created dependece output `%s`", dep->output_file);
        }
        bil_sb_clean(&out);
        return false;
    }
    char *buf;
    if (!bil_read_file(dep->output_file, &buf)) return false;
    Bil_Deps_Info info = bil_parse_dep(buf);
    for (size_t i = 0; i < dep->deps.count; ++i) {
        bool changed = false;
        char *dependence = dep->deps.items[i];
        struct bil_dep_info dependence_info = { 
            .id = bil_file_id(dependence),
            .t  = bil_file_last_update(dependence)
        };
        struct bil_dep_info target = {0};
        bool found = bil_deps_info_search(&info, dependence_info.id, &target);
        if (!found) {
            update = dependence;
            goto dep_write;
        }
    #ifdef _WIN32
        long int dependece_time;
        bil_ft2li(dependence_info.t, dependece_time);
        if (target.ltime != dependece_time)
    #else 
        if (dependence_info.t != target.t)
    #endif
        {
            result = true;
            changed = true;
        }
        if (changed == true) {
            bil_cstr_array_append(&dep->changed, dependence);
            bil_change_dep_info(&info, dependence_info);
            bil_report(BIL_INFO, ": %sFILE%s %s : dependence %s%s%s has changed",
                       BIL_INFO_COLOR, BIL_NORMAL_COLOR, dep->output_file,
                       BIL_INFO_COLOR, dependence, BIL_NORMAL_COLOR);
        }
    }
    if (result == true) {
        Bil_String_Builder out = bil_mk_dependence_from_info(&info);
        bil_dep_write(dep->output_file, &out);
        bil_sb_clean(&out);
    }
    if (bil_alloc_flag) free(buf);
    bil_da_clean(&info);
    return result;
}

int *bil_deps_ischange(const Bil_Dep *deps, size_t count)
{
    int *changes = bil_context_alloc(sizeof(int)*count);
    for (size_t i = 0; i < count; ++i)
        changes[i] = bil_dep_ischange((Bil_Dep*)(&deps[i]));
    return changes;
}

void cmd_single_run(const char **args, size_t args_count)
{
    Bil_Cmd cmd = {0};
    bil_da_append_many(&cmd, args, args_count);
    if (!bil_cmd_run_sync(&cmd)) extra_exit();
    bil_cmd_clean(&cmd);
}

Bil_String_Builder bil_mk_path(char *file, ...)
{
    Bil_String_Builder sb = {0};
    bil_sb_join_cstr(&sb, file);
    bil_sb_join_cstr(&sb, "/");
    va_list args;
    va_start(args, file);
    char *arg = va_arg(args, char*);
    while (1) {
        bil_sb_join_cstr(&sb, arg);
        arg = va_arg(args, char*);
        if (arg == NULL) break;
        bil_sb_join_cstr(&sb, "/");
    }
    va_end(args);
    return sb;
}

void bil_report(bil_report_flags flag, char *fmt, ...)
{
    fprintf(stderr, ":: ");
    switch (flag) {
    case BIL_INFO:
        fprintf(stderr, "%sINFO%s ", 
                BIL_INFO_COLOR, BIL_NORMAL_COLOR);
        break;
    case BIL_ERROR:
        fprintf(stderr, "%sERROR%s ", 
                BIL_ERROR_COLOR, BIL_NORMAL_COLOR);
        break;
    case BIL_WARNING:
        fprintf(stderr, "%sWARNING%s ",
                BIL_WARNING_COLOR, BIL_NORMAL_COLOR);
        break;
    default:
        BIL_ASSERT(0 && "Unknown flag");
        break;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    printf("\n");
}

Bil_String_Builder bil_sb_from_cstr(char *cstr)
{
    size_t len = strlen(cstr);
    size_t capacity = BIL_DA_INIT_CAPACITY;
    if (len > capacity) {
        do { capacity *= 2; } while (len > capacity);
    }
    char *buf = bil_context_alloc(capacity * sizeof(char));
    memcpy(buf, cstr, len);
    return (Bil_String_Builder) {
        .capacity = capacity,
        .count = len,
        .items = buf
    };
}

void bil_sb_join_cstr(Bil_String_Builder *sb, const char *cstr)
{
    size_t len = strlen(cstr);
    bil_da_append_many(sb, cstr, len);
}

void bil_sb_join_many(Bil_String_Builder *sb, ...)
{
    va_list args;
    va_start(args, sb);
    char *arg = va_arg(args, char*);
    while (arg != NULL) {
        bil_sb_join_cstr(sb, arg);
        arg = va_arg(args, char*);
    }
    va_end(args);
}

Bil_String_Builder bil_cmd_create(Bil_Cmd *cmd)
{
    Bil_String_Builder sb = {0};
    for (size_t i = 0; i < cmd->count; ++i) {
        SB_JOIN(&sb, cmd->items[i]);
        if (i + 1 != cmd->count) SB_JOIN(&sb, " ");
    }
    return sb;
}

Bil_Proc bil_cmd_run_async(Bil_Cmd *cmd)
{
    if (cmd->count < 1) {
        bil_report(BIL_ERROR, "Could not execute empty command\n");
        return BIL_INVALID_PROC;
    }
    Bil_String_Builder command = bil_cmd_create(cmd);
    bil_sb_join_nul(&command);
    bil_report(BIL_INFO, ": %sCMD%s [%s]",
            BIL_INFO_COLOR, BIL_NORMAL_COLOR,command.items);
#ifdef _WIN32
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.dwFlags |= STARTF_USESTDHANDLES;
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    BOOL proc = CreateProcessA(NULL, command.items, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    if (!proc) {
        bil_report(BIL_ERROR, "Could not create child process: %lu", GetLastError());
        return BIL_INVALID_PROC;
    }
    CloseHandle(pi.hThread);
    bil_sb_clean(&command);
    return pi.hProcess;
#else
    bil_sb_clean(&command);
    pid_t cpid = fork();
    if (cpid < 0) {
        bil_report(BIL_ERROR, "Could not fork child process: %s", strerror(errno));
        return BIL_INVALID_PROC;
    }
    if (cpid == 0) {
        Bil_Cmd cmd_null = {0};
        bil_cmd_append_cmd(&cmd_null, cmd);
        bil_cmd_append(&cmd_null, NULL);
        if (execvp(cmd->items[0], (char * const*) cmd_null.items) < 0) {
            bil_report(BIL_ERROR, "Could not execute child process: %s", strerror(errno));
        }
        bil_cmd_clean(&cmd_null);
    }
    return cpid;
#endif
}

char *bil_shift_args(int *argc, char ***argv)
{
    BIL_ASSERT(*argc >= 0);
    char *result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

bool bil_proc_await(Bil_Proc proc)
{
#ifdef _WIN32
    DWORD result = WaitForSingleObject(proc, INFINITE);

    if (result == WAIT_FAILED) {
        bil_report(BIL_ERROR, "Could not wait on child process: %lu", GetLastError());
        return false;
    }

    DWORD exit_status;
    if (!GetExitCodeProcess(proc, &exit_status)) {
        bil_report(BIL_ERROR, "Could not get process exit code: %lu", GetLastError());
        return false;
    }

    if (exit_status != 0) {
        bil_report(BIL_ERROR, "Command exited with exit code %lu", exit_status);
        return false;
    }

    CloseHandle(proc);
#else
    for(;;) {
        int wstatus = 0;
        if (waitpid(proc, &wstatus, 0) < 0) {
            bil_report(BIL_ERROR, "could not wait on command (pid %d): %s", proc, strerror(errno));
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                bil_report(BIL_ERROR, "command exited with exit code %d", exit_status);
                return false;
            }
            break;
        }

        if (WIFSIGNALED(wstatus)) {
            bil_report(BIL_ERROR, "command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
            return false;
        }
    }
#endif /* _WIN32 */
    return true;
}

bool bil_cmd_run_sync(Bil_Cmd *cmd)
{
    Bil_Proc proc = bil_cmd_run_async(cmd);
    if (proc == BIL_INVALID_PROC) return false;
    return bil_proc_await(proc);
}

bool bil_check_for_rebuild(const char *output_file_path, const char *source_file_path)
{
    Bil_FileTime src_file_time = bil_file_last_update(source_file_path);
    Bil_FileTime output_file_time = bil_file_last_update(output_file_path);

    #ifdef _WIN32
    if (CompareFileTime(&src_file_time, &output_file_time) == 1) return true;
    #else
    if (src_file_time > output_file_time) return true;
    #endif

    return false;
}

bool bil_rename_file(const char *file_name, const char *new_name)
{
    bil_report(BIL_INFO, "renaming %s -> %s", file_name, new_name);
#ifdef _WIN32
    if (!MoveFileEx(file_name, new_name, MOVEFILE_REPLACE_EXISTING)) {
        bil_report(BIL_ERROR, "could not rename %s to %s: %lu", file_name, new_name, GetLastError());
        return false;
    }
#else
    if (rename(file_name, new_name) < 0) {
        bil_report(BIL_ERROR, "could not rename %s to %s: %s", file_name, new_name, strerror(errno));
        return false;
    }
#endif
    return true;
}

bool bil_delete_file(const char *file_path)
{
#ifdef _WIN32
    BOOL fSuccess = DeleteFile(file_path);
    if (!fSuccess) {
        bil_report(BIL_ERROR, "Cannot delete file by path `%s`", file_path);
        return false;
    }
#else
    if (remove(file_path) != 0) {
        bil_report(BIL_ERROR, "Cannot delete file by path `%s`", file_path);
        return false;
    }
#endif
    bil_report(BIL_INFO, "file %s%s%s was deleted",
               BIL_INFO_COLOR, file_path, BIL_NORMAL_COLOR);
    return true;
}

bool bil_file_exist(const char *file_path)
{
#ifdef _WIN32
    DWORD ret = GetFileAttributes(file_path);
    return ((ret != INVALID_FILE_ATTRIBUTES) && !(ret & FILE_ATTRIBUTE_DIRECTORY));
#else
    if (access(file_path, F_OK) == 0)
        return true;
    return false;
#endif
}

#endif /* BIL_IMPLEMENTATION */
