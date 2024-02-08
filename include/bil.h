// This custom build system like MakeFile, Cmake and etc
// The main idea was stolen from `https://github.com/tsoding/nobuild`
// BIL compiles on Windows and Linux

#ifndef BIL_H_
#define BIL_H_

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#   include <windows.h>
#   include <dirent.h>
#   include <direct.h>
#   include <shellapi.h>
#else
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <sys/stat.h>
#    include <unistd.h>
#    include <fcntl.h>
#endif

#define BIL_CMD_INIT_CAPACITY 256
#define BIL_ASSERT assert
#define BIL_FREE free

#ifdef _WIN32
typedef HANDLE Bil_Proc;
#define BIL_INVALID_PROC INVALID_HANDLE_VALUE
#else
typedef int Bil_Proc;
#define BIL_INVALID_PROC (-1)
#endif 

typedef enum {
    BIL_INFO = 0,
    BIL_ERROR,
    BIL_WARNING
} bil_log_flags;

#ifndef BIL_REBUILD_COMMAND
#  if _WIN32
#    if defined(__GNUC__)
#       define BIL_REBUILD_COMMAND(source_path, output_path) "gcc", "-o", (output_path), (source_path)
#    elif defined(__clang__)
#       define BIL_REBUILD_COMMAND(source_path, output_path) "clang", "-o", (output_path), (source_path)
#    endif
#  else
#       define BIL_REBUILD_COMMAND(source_path, output_path) "cc", "-o", (output_path), (source_path)
#  endif
#endif // BIL_REBUILD_COMMAND

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Bil_Cmd;

#define bil_cmd_append(cmd, ...) cmd_append_many(cmd, __VA_ARGS__, NULL)
#define BIL_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define bil_cmd_clean(cmd) (cmd)->count = 0; (cmd)->capacity = 0; BIL_FREE((cmd)->items);

#define bil_da_append(da, new_item)                                                             \
    do {                                                                                        \
        if ((da)->count >= (da)->capacity) {                                                    \
            (da)->capacity = (da)->capacity > 0 ? (da)->capacity * 2 : BIL_CMD_INIT_CAPACITY;   \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items));          \
            assert((da)->items != NULL);                                                        \
        }                                                                                       \
                                                                                                \
        (da)->items[(da)->count++] = (new_item);                                                \
    } while(0)

void cmd_append_many(Bil_Cmd *cmd, ...);
void cmd_append_cmd(Bil_Cmd *dst, Bil_Cmd *src);
void cmd_append_one(Bil_Cmd *cmd, char *new_item);

void bil_log(bil_log_flags flag, char *fmt, ...);
void bil_delete_file(const char *deleted_file);

bool bil_check_for_rebuild(const char *output_file_path, const char *source_file_path);
bool bil_rename_file(const char *file_path, const char *new_path);
bool bil_cmd_await(Bil_Proc proc);

bool bil_cmd_build_sync(Bil_Cmd *cmd);
Bil_Proc bil_cmd_build_async(Bil_Cmd *cmd);

#define BIL_SB_INIT_CAPACITY 128
#define SB_Args(sb) (int) (sb).count, (sb).str
#define SB_Fmt "%.*s"

typedef struct {
    char *str;
    size_t count;
    size_t capacity;
} Bil_String_Builder;

void sb_join_cstr(Bil_String_Builder *sb, const char *cstr);
void sb_join_sb(Bil_String_Builder *dst, const Bil_String_Builder *src);
void sb_join_many(Bil_String_Builder *sb, ...);

Bil_String_Builder sb_from_cstr(char *cstr);
Bil_String_Builder bil_cmd_create(Bil_Cmd *cmd);

#define sb_join_nul(sb) { (sb)->str[(sb)->count] = '\0'; } 
#define SB_JOIN(sb, ...) sb_join_many((sb), __VA_ARGS__, NULL)
#define sb_clean(sb) { (sb)->count = 0; (sb)->capacity = 0; BIL_FREE((sb)->str); }

// After rebuilding file old file renames to `DELETME`
#define BIL_REBUILD(argv)                                                                           \
    do {                                                                                            \
        const char *output_file_path = (argv[0]);                                                   \
        const char *source_file_path = __FILE__;                                                    \
                                                                                                    \
        bool rebuild_is_need = bil_check_for_rebuild(output_file_path, source_file_path);           \
                                                                                                    \
        if (rebuild_is_need) {                                                                      \
            bil_log(BIL_INFO, "start rebuild file `%s`", source_file_path);                         \
            if (!bil_rename_file(output_file_path, "DELETEME")) exit(1);                            \
                                                                                                    \
            Bil_Cmd rebuild_cmd = {0};                                                              \
            bil_cmd_append(&rebuild_cmd, BIL_REBUILD_COMMAND(source_file_path, output_file_path));  \
                                                                                                    \
            bil_cmd_build_sync(&rebuild_cmd);                                                       \
            bil_log(BIL_INFO, "rebuild complete");                                                  \
                                                                                                    \
            Bil_Cmd cmd = {0};                                                                      \
            bil_cmd_append(&cmd, output_file_path);                                                 \
            bil_cmd_build_sync(&cmd);                                                               \
            exit(0);                                                                                \
        }                                                                                           \
    } while (0)

int bil_file_exist(const char *file_path);
void bil_delete_file(const char *file_path); 
char *bil_shift_args(int *argc, char ***argv);

#endif // BIL_H_

#ifdef BIL_IMPLEMENTATION

void cmd_append_one(Bil_Cmd *cmd, char *new_item)
{
    bil_da_append(cmd, new_item);
}

void cmd_append_many(Bil_Cmd *cmd, ...)
{
    va_list args;
    va_start(args, cmd);

    char* arg = va_arg(args, char*);
    while (arg != NULL) {
        cmd_append_one(cmd, arg);
        arg = va_arg(args, char*);
    }
    va_end(args);
}

void cmd_append_cmd(Bil_Cmd *dst, Bil_Cmd *src) 
{
    if (dst->count + src->count >= dst->capacity) {
        if (dst->capacity == 0) {
            dst->capacity = BIL_CMD_INIT_CAPACITY;
        }

        do { dst->capacity *= 2;} while (dst->count + src->count >= dst->capacity);
        dst->items = realloc(dst->items, dst->capacity * sizeof(*dst->items));
        assert(dst->items != NULL);
    }

    memcpy(dst->items + dst->count, src->items, src->count * sizeof(*src->items));
    dst->count += src->count;
}

void bil_log(bil_log_flags flag, char *fmt, ...)
{
    switch (flag) {
    case BIL_INFO:
        fprintf(stderr, "[Info] ");
        break;
    case BIL_ERROR:
        fprintf(stderr, "[Error] ");
        break;
    case BIL_WARNING:
        fprintf(stderr, "[Warning] ");
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
    size_t capacity = BIL_SB_INIT_CAPACITY;

    if (len > capacity) {
        do {
            capacity *= 2;
        } while (len > capacity);
    }

    char *buf = (char*) malloc(capacity);
    memcpy(buf, cstr, len);

    return (Bil_String_Builder) {
        .capacity = capacity,
        .count = len,
        .str = buf  
    };
}

void sb_join_cstr(Bil_String_Builder *sb, const char *cstr)
{
    if (sb->count == 0) {
        sb->str = (char*) malloc(BIL_CMD_INIT_CAPACITY);
        sb->capacity = BIL_CMD_INIT_CAPACITY;
    }

    size_t cstr_len = strlen(cstr);

    if (sb->count + cstr_len > sb->capacity) {
        do {
            sb->capacity *= 2;
        } while (sb->count + cstr_len > sb->capacity);

        sb->str = realloc(sb->str, sb->capacity);
    }

    memcpy(sb->str + sb->count, cstr, cstr_len);
    sb->count += cstr_len;
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

    for (size_t i = 0; i < cmd->count; ++i) {
        SB_JOIN(&sb, cmd->items[i], " ");
    }

    return sb;
}

Bil_Proc bil_cmd_build_async(Bil_Cmd *cmd)
{
    if (cmd->count < 1) {
        bil_log(BIL_ERROR, "Could not execute empty command\n");
        return BIL_INVALID_PROC;
    }

#ifdef _WIN32

    Bil_String_Builder command = bil_cmd_create(cmd);
    sb_join_nul(&command); 
    bil_log(BIL_INFO, "Command: "SB_Fmt"", SB_Args(command));

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
        exit(1);
    } else {
        bil_log(BIL_INFO, "Command was successfuly execute");
    }

    CloseHandle(pi.hThread);
    sb_clean(&command);
    return pi.hProcess;
#else
    pid_t cpid = fork();

    if (cpid < 0) {
        bil_log(BIL_ERROR, "Could not fork child process: %s", strerror(errno));
        return BIL_INVALID_PROC;
    }

    if (cpid == 0) {
        Bil_Cmd cmd_null = {0};
        cmd_append_cmd(&cmd_null, cmd);
        bil_cmd_append(&cmd_null, NULL);

        Bil_String_Builder sb = bil_cmd_create(cmd);
        sb_join_nul(&sb);

        bil_log(BIL_INFO, "Command: %s", sb.str);
        sb_clean(&sb);

        if (execvp(cmd->items[0], (char * const*) cmd_null.items) < 0) {
            bil_log(BIL_ERROR, "Could not execute child process: %s", strerror(errno));
            exit(1);
        }
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

bool bil_cmd_await(Bil_Proc proc)
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
    return true;
#else
    while(1) {
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

    return true;
#endif // _WIN32
}

bool bil_cmd_build_sync(Bil_Cmd *cmd)
{
    Bil_Proc proc = bil_cmd_build_async(cmd);
    if (proc == BIL_INVALID_PROC) return false;
    return bil_cmd_await(proc);
}

bool bil_check_for_rebuild(const char *output_file_path, const char *source_file_path) 
{
#ifdef _WIN32
    BOOL Proc;

    HANDLE src_file = CreateFile(source_file_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (src_file == INVALID_HANDLE_VALUE) {
        bil_log(BIL_ERROR, "cannot open `%s`: %lu", source_file_path, GetLastError()); 
    }

    FILETIME src_time;
    Proc = GetFileTime(src_file, NULL, NULL, &src_time);
    CloseHandle(src_file);

    if (!Proc) {
        bil_log(BIL_ERROR, "cannot get `%s` time: %lu", source_file_path, GetLastError()); 
    }

    HANDLE output_file = CreateFile(output_file_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (output_file == INVALID_HANDLE_VALUE) {
        bil_log(BIL_ERROR, "cannot open `%s`: %lu", output_file_path, GetLastError()); 
    }

    FILETIME output_time;
    Proc = GetFileTime(output_file, NULL, NULL, &output_time);
    CloseHandle(output_file);

    if (!Proc) {
        bil_log(BIL_ERROR, "cannot get `%s` time: %lu", output_file_path, GetLastError()); 
    }    

    if (CompareFileTime(&src_time, &output_time) == 1)
        return true;
    
    return false;
#else
    struct stat statbuf = {0};

    if (stat(source_file_path, &statbuf) < 0) {
        bil_log(BIL_ERROR, "could not get info about file `%s`: %s", source_file_path, strerror(errno));
        exit(1);
    }

    time_t src_file_time = statbuf.st_mtime;

    if (stat(output_file_path, &statbuf) < 0) {
        bil_log(BIL_ERROR, "could not get info about file `%s`: %s", output_file_path, strerror(errno));
        exit(1);
    }

    time_t output_file_time = statbuf.st_mtime;

    if (src_file_time > output_file_time) return true;

    return false;
#endif
}

bool bil_rename_file(const char *file_name, const char *new_name)
{
    bil_log(BIL_INFO, "renaming %s -> %s", file_name, new_name);
#ifdef _WIN32
    if (!MoveFileEx(file_name, new_name, MOVEFILE_REPLACE_EXISTING)) {
        bil_log(BIL_ERROR, "could not rename %s to %s: %lu", file_name, new_name, GetLastError());
        return false;
    }
    return true;
#else
    if (rename(file_name, new_name) < 0) {
        bil_log(BIL_ERROR, "could not rename %s to %s: %s", file_name, new_name, strerror(errno));
        return false;
    }
    return true;
#endif
}

void bil_delete_file(const char *file_path) 
{
#ifdef _WIN32
    BOOL fSuccess = DeleteFile(file_path);
    if (!fSuccess) {
        bil_log(BIL_ERROR, "Cannot delete file by path `%s`\n", file_path);
    }
#else
    if (remove(file_path) != 0) {
        bil_log(BIL_ERROR, "Cannot delete file by path `%s`\n", file_path);
    }
#endif
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
    if (access(file_path, F_OK) == 0) {
        return 1;
    }
    return 0;    
#endif 
}

#endif // BIL_IMPLEMENTATION 