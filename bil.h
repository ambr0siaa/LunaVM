// This custom build system like MakeFile, Cmake and etc
// But as C single header style library (stb style)
// bil is crose platform (well, work on windows and linux)
// This tiny build system has inspired by `https://github.com/tsoding/nobuild`

// Note: probably new fitures not working on windows

#ifndef BIL_H_
#define BIL_H_

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <sys/stat.h>
#    include <unistd.h>
#    include <fcntl.h>
#endif

#define BIL_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define BIL_DA_INIT_CAPACITY 256
#define BIL_ASSERT assert
#define BIL_FREE free
#define BIL_EXIT exit
#define BIL_REALLOC realloc

#define BIL_INFO_COLOR "\033[0;36m"
#define BIL_ERROR_COLOR "\033[0;31m"
#define BIL_NORMAL_COLOR "\x1B[0m"
#define BIL_WARNING_COLOR "\x1b[36m"

#ifdef _WIN32
typedef HANDLE Bil_Proc;
#define BIL_INVALID_PROC INVALID_HANDLE_VALUE
#else
typedef int Bil_Proc;
#define BIL_INVALID_PROC (-1)
#endif

#define bil_da_append(da, new_item)                                                              \
    do {                                                                                         \
        if ((da)->count >= (da)->capacity) {                                                     \
            (da)->capacity = (da)->capacity > 0 ? (da)->capacity * 2 : BIL_DA_INIT_CAPACITY;     \
            (da)->items = BIL_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items));       \
            BIL_ASSERT((da)->items != NULL);                                                     \
        }                                                                                        \
        (da)->items[(da)->count++] = (new_item);                                                 \
    } while(0)

#define bil_da_append_many(da, new_items, items_count)                                           \
    do {                                                                                         \
        if ((da)->count + (items_count) >= (da)->capacity) {                                     \
            if ((da)->capacity == 0) (da)->capacity = BIL_DA_INIT_CAPACITY;                      \
            while ((da)->count + (items_count) >= (da)->capacity) { (da)->capacity *= 2; }       \
            (da)->items = BIL_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items));       \
            BIL_ASSERT((da)->items != NULL);                                                     \
        }                                                                                        \
        memcpy((da)->items + (da)->count, (new_items), (items_count) * sizeof(*(da)->items));    \
        (da)->count += (items_count);                                                            \
    } while (0)

#define bil_da_append_da(dst, src)                                                               \
    do {                                                                                         \
        if ((dst)->count + (src)->count >= (dst)->capacity) {                                    \
            if ((dst)->capacity == 0) (dst)->capacity = BIL_DA_INIT_CAPACITY;                    \
            while ((dst)->count + (src)->count >= (dst)->capacity) { (dst)->capacity *= 2; }     \
            (dst)->items = BIL_REALLOC((dst)->items, (dst)->capacity * sizeof(*(dst)->items));   \
            BIL_ASSERT((dst)->items != NULL);                                                    \
        }                                                                                        \
        memcpy((dst)->items + (dst)->count, (src)->items, (src)->count * sizeof(*(src)->items)); \
        (dst)->count += (src)->count;                                                            \
    } while (0)

#define bil_da_clean(da)            \
    do {                            \
        if ((da)->items != NULL) {  \
            BIL_FREE((da)->items);  \
            (da)->capacity = 0;     \
            (da)->count = 0;        \
        }                           \
    } while (0)

typedef enum {
    BIL_INFO = 0,
    BIL_ERROR,
    BIL_WARNING
} bil_log_flags;

#define BIL_EXIT_SUCCESS 0
#define BIL_EXIT_FAILURE 1

#ifndef BIL_REBUILD_COMMAND
#  if _WIN32
#    if defined(__GNUC__)
#       define BIL_REBUILD_COMMAND(source_path, output_path) "gcc", "-o", (output_path), (source_path)
#    elif defined(__clang__)
#       define BIL_REBUILD_COMMAND(source_path, output_path) "clang", "-o", (output_path), (source_path)
#    endif
#  else
#       define BIL_REBUILD_COMMAND(source_path, output_path) "gcc", "-o", (output_path), (source_path)
#  endif
#endif // BIL_REBUILD_COMMAND

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Bil_Cmd;

#define CMD(...) \
    cmd_single_run(((const char *[]){__VA_ARGS__}), sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char*))

#define bil_cmd_append(cmd, ...) \
    bil_da_append_many(cmd, ((const char *[]){__VA_ARGS__}), sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char*))

#define bil_cmd_clean(cmd) bil_da_clean(cmd)
#define bil_cmd_append_cmd(dst, src) bil_da_append_da(dst, src)

void cmd_single_run(const char **args, size_t args_count);

bool bil_cmd_run_sync(Bil_Cmd *cmd);
Bil_Proc bil_cmd_run_async(Bil_Cmd *cmd);

bool bil_proc_await(Bil_Proc proc);

void bil_log(bil_log_flags flag, char *fmt, ...);

bool bil_rename_file(const char *file_path, const char *new_path);
void bil_delete_file(const char *deleted_file);

bool bil_check_for_rebuild(const char *output_file_path, const char *source_file_path);

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Bil_Cstr_Array;

void cstr_arr_clean(Bil_Cstr_Array *arr);
void cstr_arr_append(Bil_Cstr_Array *arr, char *item);
void cstr_arr_append_many(Bil_Cstr_Array *arr, const char **items, size_t items_count);

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} Bil_String_Builder;

#define SB_Args(sb) (int) (sb).count, (sb).items
#define SB_Fmt "%.*s"

void sb_join_many(Bil_String_Builder *sb, ...);
void sb_join_cstr(Bil_String_Builder *sb, const char *cstr);

Bil_String_Builder sb_from_cstr(char *cstr);
Bil_String_Builder bil_cmd_create(Bil_Cmd *cmd);

#define sb_clean(sb)     bil_da_clean(sb);
#define sb_join_nul(sb)  bil_da_append_many(sb, "", 1)
#define SB_JOIN(sb,...)  sb_join_many((sb), __VA_ARGS__, NULL)

typedef struct {
    char *data;
    size_t count;
} Bil_String_View;

#define SV_Fmt SB_Fmt
#define SV_Args(sv) (int) (sv).count, (sv).data

Bil_String_View sv_from_cstr(char *cstr);
Bil_String_View sv_cut_value(Bil_String_View *sv);
uint32_t sv_to_u32(Bil_String_View sv);

#ifdef _Win32
#   #define Bil_FileTime FILETIME
#else
#   define Bil_FileTime time_t
#endif

struct bil_dep_info {
    uint32_t id;
    Bil_FileTime t;
};

// TODO: binary search
typedef struct {
    struct bil_dep_info *items;
    size_t capacity;
    size_t count;
} Bil_Deps_Info;


typedef struct {
    Bil_Cstr_Array deps;    
    const char *output_file;
} Bil_Dep;

/*
*  Usage for dependences:
*      Bil_Dep dep = {0};
*      bil_dep_init(
*             1. &dep, 
*             2. path for output dependence file.
*                Well, I named this files with extention `.bil`,
*             3. file paths that is dependence
*                this file bil will keep an eye on
*           );
*/

bool bil_dep_ischange(Bil_Dep *dep);    // the main workhorse function for dependencies

// some stuffs for bil's dependences 
uint32_t bil_file_id(char *file_path);
Bil_Deps_Info bil_parse_dep(char *src);
Bil_FileTime bil_file_last_update(const char *path);
void bil_dep_write(const char *path, Bil_String_Builder *dep);
Bil_String_Builder bil_mk_dependence_file(Bil_Cstr_Array deps);
Bil_String_Builder bil_mk_dependence_from_info(Bil_Deps_Info *info);
void bil_change_dep_info(Bil_Deps_Info *info, struct bil_dep_info new_info);
bool bil_deps_info_search(Bil_Deps_Info *info, uint32_t id, struct bil_dep_info *target); 

#define bil_dep_init(dep, output_file_path, ...)                                  \
    do {                                                                          \
        (dep)->output_file = output_file_path;                                    \
        cstr_arr_append_many(&(dep)->deps,                                        \
                             ((const char*[]){__VA_ARGS__}),                      \
                             sizeof((const char*[]){__VA_ARGS__})/sizeof(char*)); \
    } while (0)

#define bil_dep_clean(dep) cstr_arr_clean(&(dep)->deps)

void bil_mkdir(const char *name);
bool bil_dir_exist(const char *dir_path);
char *bil_read_file(const char *file_path);

#define DELETEME_FILE "DELETEME"
#define BIL_CURRENT_DIR "current"

// After rebuilding old file renames to `DELETME`
#define BIL_REBUILD(argv, deleteme_dir)                                                             \
    do {                                                                                            \
        const char *output_file_path = (argv[0]);                                                   \
        const char *source_file_path = __FILE__;                                                    \
                                                                                                    \
        bool rebuild_is_need = bil_check_for_rebuild(output_file_path, source_file_path);           \
                                                                                                    \
        if (rebuild_is_need) {                                                                      \
            bil_log(BIL_INFO, "start rebuild file `%s`", source_file_path);                         \
            Bil_String_Builder deleteme_sb_path = {0};                                              \
            const char *deleteme_path;                                                              \
                                                                                                    \
            int is_current = strcmp(deleteme_dir, BIL_CURRENT_DIR);                                 \
                                                                                                    \
            if (is_current) {                                                                       \
                deleteme_sb_path = PATH(".", deleteme_dir, DELETEME_FILE);                          \
                deleteme_path = deleteme_sb_path.items;                                             \
            } else {                                                                                \
                deleteme_path = DELETEME_FILE;                                                      \
            }                                                                                       \
                                                                                                    \
            if (!bil_rename_file(output_file_path, deleteme_path)) BIL_EXIT(1);                     \
            if (is_current) sb_clean(&deleteme_sb_path);                                            \
                                                                                                    \
            Bil_Cmd rebuild_cmd = {0};                                                              \
            bil_cmd_append(&rebuild_cmd, BIL_REBUILD_COMMAND(source_file_path, output_file_path));  \
            if (!bil_cmd_run_sync(&rebuild_cmd)) BIL_EXIT(1);                                       \
            bil_cmd_clean(&rebuild_cmd);                                                            \
            bil_log(BIL_INFO, "rebuild complete");                                                  \
                                                                                                    \
            Bil_Cmd cmd = {0};                                                                      \
            bil_cmd_append(&cmd, output_file_path);                                                 \
            bil_cmd_run_async(&cmd);                                                                \
            bil_cmd_clean(&cmd);                                                                    \
            BIL_EXIT(0);                                                                            \
        }                                                                                           \
    } while (0)

int bil_file_exist(const char *file_path);
void bil_delete_file(const char *file_path);
char *bil_shift_args(int *argc, char ***argv);

Bil_String_Builder bil_mk_path(char *file, ...);
#define PATH(file, ...) bil_mk_path(file, __VA_ARGS__, NULL)

#endif // BIL_H_

#ifdef BIL_IMPLEMENTATION

uint32_t sv_to_u32(Bil_String_View sv)
{
    uint32_t result = 0;
    for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); ++i)
        result = result * 10 + sv.data[i] - '0'; 
    return result;
}

Bil_String_View sv_from_cstr(char *cstr)
{
    return (Bil_String_View) {
        .count = strlen(cstr),
        .data = cstr
    };
}

Bil_String_View sv_cut_value(Bil_String_View *sv)
{
    Bil_String_View result;
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

Bil_Deps_Info bil_parse_dep(char *src)
{
    Bil_Deps_Info info = {0};
    Bil_String_View source = sv_from_cstr(src);
    while (source.count > 0) {
        struct bil_dep_info i = {0};
        if (isdigit(source.data[0])) {
            i.id = sv_to_u32(sv_cut_value(&source));
            
            source.data += 1;
            source.count -= 1;

            i.t = sv_to_u32(sv_cut_value(&source));
            
            source.data += 1;
            source.count -= 1;
        } else {
            bil_log(BIL_ERROR, "unknown character `%c`", source.data[0]);
            BIL_EXIT(1);
        }
        bil_da_append(&info, i);
    }
    return info;
}

void cstr_arr_append_many(Bil_Cstr_Array *arr, const char **items, size_t items_count)
{
    bil_da_append_many(arr, items, items_count);
}

void cstr_arr_append(Bil_Cstr_Array *arr, char *item)
{
    bil_da_append(arr, item);
}

void cstr_arr_clean(Bil_Cstr_Array *arr)
{
    bil_da_clean(arr);
}

void bil_mkdir(const char *dir_path)
{
    bil_log(BIL_INFO, "Make directory %s", dir_path);
#ifdef _Win32
    if (CreateDirectoryA(dir_path, NULL) == 0) {
        bil_log(BIL_ERROR, "cannot create directory `%s`: %lu",
                dir_path, GetLastError());
        BIL_EXIT(1);
    }
#else
    if (mkdir(dir_path, 0777) < 0) {
        bil_log(BIL_ERROR, "cannot create directory `%s`: %s\n",
                dir_path, strerror(errno));
        BIL_EXIT(1);
    }
#endif
}

bool bil_dir_exist(const char *dir_path)
{
    bool result;
    DIR *dir = opendir(dir_path);
    if (dir) {
        result = true;
    } else if (ENOENT == errno) {
        result = false;
    } else {
        bil_log(BIL_ERROR, "file `%s` not a directory\n", dir_path);
    }
    closedir(dir);
    return result;
}

// TODO: not implemented
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

char *bil_read_file(const char *file_path)
{
    FILE *f = fopen(file_path, "r");
    if (!f) {
        bil_log(BIL_ERROR, "cannot open file `%s`", file_path);
        BIL_EXIT(1);
    }

    if (fseek(f, 0, SEEK_END) < 0)
        goto error_reading; 

    long int file_size = ftell(f);
    if (file_size < 0)
        goto error_reading;

    if (fseek(f, 0, SEEK_SET) < 0) 
        goto error_reading;

    char *buf = malloc(file_size + 1);
    if (!buf) {
        bil_log(BIL_ERROR, "cannot allocate size `%li`\n", file_size);
        BIL_EXIT(1);
    }

    size_t buf_len = fread(buf, 1, file_size, f);
    buf[buf_len] = '\0';

    if (ferror(f))
        goto error_reading; 

    fclose(f);
    return buf;

    error_reading:
        fclose(f);
        bil_log(BIL_ERROR, "cannot read from `%s`: %s", file_path, strerror(errno));
        BIL_EXIT(1);
}

Bil_FileTime bil_file_last_update(const char *path)
{
    if (!bil_file_exist(path)) {
        bil_log(BIL_ERROR, "cannot find dependence by path `%s`", path);
        BIL_EXIT(1);
    }
#ifdef _Win32
    BOOL Proc;

    HANDLE file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (file == BIL_INVALID_PROC) {
        bil_log(BIL_ERROR, "cannot open `%s`: %lu", path, GetLastError());
        BIL_EXIT(1);
    }

    Bil_FileTime file_time;
    Proc = GetFileTime(file, NULL, NULL, &file_time);
    CloseHandle(file);

    if (!Proc) {
        bil_log(BIL_ERROR, "cannot get `%s` time: %lu", path, GetLastError());
        BIL_EXIT(1);
    }

    return file_time;
#else
    struct stat statbuf = {0};
    if (stat(path, &statbuf) < 0) {
        bil_log(BIL_ERROR, "cannot get info about `%s`: %s", strerror(errno));
        BIL_EXIT(1);
    }
    return statbuf.st_ctime;
#endif
}

void bil_dep_write(const char *path, Bil_String_Builder *dep)
{
    FILE *f = fopen(path, "w");
    if (!f) {
        bil_log(BIL_ERROR, "cannot open file by path `%s`", path);
        BIL_EXIT(1);
    }

    fwrite(dep->items, sizeof(dep->items[0]), sizeof(dep->items[0]) * dep->count, f);

    if (ferror(f)) {
        bil_log(BIL_ERROR, "cannot write to `%s` file", path);
        BIL_EXIT(1);
    }

    fclose(f);
}

uint32_t bil_file_id(char *file_path)
{
    int p = 0x0;
    int m = 0xfff;
    uint32_t id = 0x0;
    const int magic = 0x428a2f98; // cubic root of 2
    const size_t len = strlen(file_path);
    for (size_t i = len; i > 0; --i) {
        char c = file_path[i - 1];
        if (c == '\\' || c == '/') break;
        id += (int)(c) * (p ^ magic);
        p = (p << 2) % m;
    }
    return id;
}

Bil_String_Builder bil_mk_dependence_file(Bil_Cstr_Array deps)
{
    Bil_String_Builder sb = {0};
    for (size_t i = 0; i < deps.count; ++i) {
        char buf[256];
        char *file_path = deps.items[i];
        Bil_FileTime t = bil_file_last_update(file_path);
        #ifdef _Win32
        t = (long int)((t.dwHighDateTime) << 32 | t.dwLowDateTime);
        #endif
        uint32_t id = bil_file_id(file_path);
        snprintf(buf, sizeof(buf), "%u %li\n", id, t);
        sb_join_cstr(&sb, buf);
    }
    sb_join_nul(&sb);
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
        Bil_FileTime t = info->items[i].t;
        #ifdef _Win32
        t = (long int)((t.dwHighDateTime) << 32 | t.dwLowDateTime);
        #endif
        snprintf(buf, sizeof(buf), "%u %li\n", info->items[i].id, t);
        sb_join_cstr(&sb, buf);
    }
    sb_join_nul(&sb);
    return sb;
}

bool bil_dep_ischange(Bil_Dep *dep)
{
    bool result = false;
    if (!bil_file_exist(dep->output_file)) {
        Bil_String_Builder out = bil_mk_dependence_file(dep->deps);
        bil_dep_write(dep->output_file, &out);
        bil_log(BIL_INFO, "Created dependece output `%s`",
                dep->output_file);
        sb_clean(&out);
        return false;
    }

    char *buf = bil_read_file(dep->output_file);
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
        
        if (found == false)
            bil_log(BIL_WARNING, "cannot find dependence for `%s`",
                    dependence);

        #ifdef _Win32
        if (CompareFileTime(target.t, dependence_info.t) == 0)
        #else 
        if (dependence_info.t != target.t)
        #endif
        {
            result = true;
            changed = true;
        }

        if (changed == true) {
            bil_log(BIL_INFO, "Changed dependece `%s`", dependence);
            bil_change_dep_info(&info, dependence_info);
        }
    }

    if (result == true) {
        Bil_String_Builder out = bil_mk_dependence_from_info(&info);
        bil_dep_write(dep->output_file, &out);
        sb_clean(&out);
    }

    free(buf);
    bil_da_clean(&info);
    return result;
}

void cmd_single_run(const char **args, size_t args_count)
{
    Bil_Cmd cmd = {0};
    bil_da_append_many(&cmd, args, args_count);
    if (!bil_cmd_run_sync(&cmd)) BIL_EXIT(1);
    bil_cmd_clean(&cmd);
}

Bil_String_Builder bil_mk_path(char *file, ...)
{
    Bil_String_Builder sb = {0};
    sb_join_cstr(&sb, file);
    sb_join_cstr(&sb, "/");

    va_list args;
    va_start(args, file);

    char *arg = va_arg(args, char*);
    while (1) {
        sb_join_cstr(&sb, arg);
        arg = va_arg(args, char*);
        if (arg == NULL) break;
        sb_join_cstr(&sb, "/");
    }
    va_end(args);
    
    return sb;
}

void bil_log(bil_log_flags flag, char *fmt, ...)
{
    switch (flag) {
    case BIL_INFO:
        fprintf(stderr, "%sINFO%s: ", 
                BIL_INFO_COLOR, BIL_NORMAL_COLOR);
        break;
    case BIL_ERROR:
        fprintf(stderr, "%sERROR%s: ", 
                BIL_ERROR_COLOR, BIL_NORMAL_COLOR);
        break;
    case BIL_WARNING:
        fprintf(stderr, "%sWARNING%s: ",
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

Bil_String_Builder sb_from_cstr(char *cstr)
{
    size_t len = strlen(cstr);
    size_t capacity = BIL_DA_INIT_CAPACITY;

    if (len > capacity) {
        do { capacity *= 2; } while (len > capacity);
    }

    char *buf = malloc(capacity * sizeof(char));
    memcpy(buf, cstr, len);

    return (Bil_String_Builder) {
        .capacity = capacity,
        .count = len,
        .items = buf
    };
}

void sb_join_cstr(Bil_String_Builder *sb, const char *cstr)
{
    size_t len = strlen(cstr);
    bil_da_append_many(sb, cstr, len);
}

void sb_join_many(Bil_String_Builder *sb, ...)
{
    va_list args;
    va_start(args, sb);
    char *arg = va_arg(args, char*);
    while (arg != NULL) {
        sb_join_cstr(sb, arg);
        arg = va_arg(args, char*);
    }
    va_end(args);
}

Bil_String_Builder bil_cmd_create(Bil_Cmd *cmd)
{
    Bil_String_Builder sb = {0};
    for (size_t i = 0; i < cmd->count; ++i)
        SB_JOIN(&sb, cmd->items[i], " ");
    return sb;
}

Bil_Proc bil_cmd_run_async(Bil_Cmd *cmd)
{
    if (cmd->count < 1) {
        bil_log(BIL_ERROR, "Could not execute empty command\n");
        return BIL_INVALID_PROC;
    }

    Bil_String_Builder command = bil_cmd_create(cmd);
    sb_join_nul(&command);

    bil_log(BIL_INFO, "CMD: %s", command.items);

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

    BOOL proc = CreateProcessA(NULL, command.str, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

    if (!proc) {
        bil_log(BIL_ERROR, "Could not create child process: %lu", GetLastError());
        BIL_EXIT(1);
    } else {
        bil_log(BIL_INFO, "Command was successfuly execute");
    }

    CloseHandle(pi.hThread);
    sb_clean(&command);
    return pi.hProcess;
#else
    sb_clean(&command);
    pid_t cpid = fork();

    if (cpid < 0) {
        bil_log(BIL_ERROR, "Could not fork child process: %s", strerror(errno));
        return BIL_INVALID_PROC;
    }

    if (cpid == 0) {
        Bil_Cmd cmd_null = {0};
        bil_cmd_append_cmd(&cmd_null, cmd);
        bil_cmd_append(&cmd_null, NULL);

        if (execvp(cmd->items[0], (char * const*) cmd_null.items) < 0) {
            bil_log(BIL_ERROR, "Could not execute child process: %s", strerror(errno));
        }

        bil_cmd_clean(&cmd_null);
    }

    return cpid;
#endif
}

char *bil_shift_args(int *argc, char ***argv)
{
    assert(argc >= 0);
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
        bil_log(BIL_ERROR, "Could not wait on child process: %lu", GetLastError());
        return false;
    }

    DWORD exit_status;
    if (!GetExitCodeProcess(proc, &exit_status)) {
        bil_log(BIL_ERROR, "Could not get process exit code: %lu", GetLastError());
        return false;
    }

    if (exit_status != 0) {
        bil_log(BIL_ERROR, "Command exited with exit code %lu", exit_status);
        return false;
    }

    CloseHandle(proc);
#else
    for(;;) {
        int wstatus = 0;
        if (waitpid(proc, &wstatus, 0) < 0) {
            bil_log(BIL_ERROR, "could not wait on command (pid %d): %s", proc, strerror(errno));
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                bil_log(BIL_ERROR, "command exited with exit code %d", exit_status);
                return false;
            }
            break;
        }

        if (WIFSIGNALED(wstatus)) {
            bil_log(BIL_ERROR, "command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
            return false;
        }
    }

#endif // _WIN32
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
    if (CompareFileTime(&src_time, &output_time) == 1) return true;
    #else
    if (src_file_time > output_file_time) return true;
    #endif
    return false;
}

bool bil_rename_file(const char *file_name, const char *new_name)
{
    bil_log(BIL_INFO, "renaming %s -> %s", file_name, new_name);
#ifdef _WIN32
    if (!MoveFileEx(file_name, new_name, MOVEFILE_REPLACE_EXISTING)) {
        bil_log(BIL_ERROR, "could not rename %s to %s: %lu", file_name, new_name, GetLastError());
        return false;
    }
#else
    if (rename(file_name, new_name) < 0) {
        bil_log(BIL_ERROR, "could not rename %s to %s: %s", file_name, new_name, strerror(errno));
        return false;
    }
#endif
    return true;
}

void bil_delete_file(const char *file_path)
{
#ifdef _WIN32
    BOOL fSuccess = DeleteFile(file_path);
    if (!fSuccess) {
        bil_log(BIL_ERROR, "Cannot delete file by path `%s`", file_path);
        BIL_EXIT(BIL_EXIT_FAILURE);
    }
#else
    if (remove(file_path) != 0) {
        bil_log(BIL_ERROR, "Cannot delete file by path `%s`", file_path);
        BIL_EXIT(BIL_EXIT_FAILURE);
    }
#endif
    bil_log(BIL_INFO, "File `%s` was deleted\n", file_path);
}

int bil_file_exist(const char *file_path)
{
#ifdef _WIN32
    WIN32_FIND_DATA FindFileData;
    HANDLE handle = FindFirstFile(file_path, &FindFileData) ;
    int found = handle != INVALID_HANDLE_VALUE;
    if (found) {
        FindClose(handle);
    }
    return found;
#else
    if (access(file_path, F_OK) == 0) return 1;
    return 0;
#endif
}

#endif // BIL_IMPLEMENTATION