// NOTE(vanya): STL abstraction
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

// NOTE(vanya): Helper types
typedef uint8_t bool;
#define true (uint8_t)1
#define false (uint8_t)0

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

// NOTE(vanya): Memory functions
#define P_memset(dest, fill_byte, size) memset(dest, fill_byte, size)
#define P_memcpy(dest, src, size) memcpy(dest, src, size)
#define P_malloc(size) malloc(size)
#define P_calloc(num, size) calloc(num, size)
#define P_realloc(old_ptr, size) realloc(old_ptr, size)
#define P_free(ptr) free(ptr)

//NOTE(vanya): String functions 
#define P_strlen(str) strlen(str)

#define P_strcmp(str_a, str_b) strcmp(str_a, str_b)
#define P_strncmp(str_a, str_b, size) strncmp(str_a, str_b, size)

#define P_strdup(str) strdup(str)

#define P_snprintf(buffer, size, format, ...) snprintf(buffer, size, format, __VA_ARGS__)
#define P_vsnprintf(buffer, size, format, ...) vsnprintf(buffer, size, format, __VA_ARGS__)

// NOTE(vanya): Debugging macros
#define P_assert(exp, msg) \
    { if (!(exp)) { fprintf(stderr, "Assetion failed! - " msg); exit(1); } }

// NOTE(vanya): Logging macros
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_DEBUG(format_string, ...) \
    fprintf(stdout, "debug:    %s:%d %s - " format_string "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_INFO(format_string, ...) \
    fprintf(stdout, "info:     %s:%d %s - " format_string "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_WARNING(format_string, ...) \
    fprintf(stdout, "warning:  %s:%d %s - " format_string "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_ERROR(format_string, ...) \
    fprintf(stderr, "error:    %s:%d %s - " format_string "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_CRITICAL(format_string, ...) \
    fprintf(stderr, "critical: %s:%d %s - " format_string "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

// NOTE(vanya): File IO functions
#define P_MAX_PATH_LENGTH PATH_MAX
#define P_SEEK_SET SEEK_SET
#define P_SEEK_CUR SEEK_CUR
#define P_SEEK_END SEEK_END

#define P_fopen(path, mode) fopen(path, mode)
#define P_fclose(io) fclose(io)
#define P_fflush(io) fflush(io)

#define P_fread(io, buf , size) fread(buf, size, 1, io)
#define P_fwrite(io, buf, size) fwrite(buf, size, 1, io)

#define P_fgetc(io) fgetc(io)
#define P_fputc(io, c) fputc(c, io)

#define P_ftell(io) ftell(io)
#define P_fseek(io, offset, origin) fseek(io, offset, origin)
#define P_frewind(io) frewind(io)

#define P_ferror(io) ferror(io)
#define P_fremove(path) remove(path)
#define P_frename(path_old, path_new) rename(path_old, path_new)

#define P_print_os_error(prefix) perror(prefix)

// NOTE(vanya): Math helper functions
#define P_round_i32(f) (i32)round(f)
#define P_floor_i32(f) (i32)floor(f)
#define P_round_f32(f) (f32)round(f)
#define P_floor_f32(f) (f32)floor(f)

#define P_TEMPLATE_MIN_MAX_CLAMP(T) \
    T P_min_##T(T a, T b) \
    { \
        return a < b ? a : b; \
    } \
    \
    T P_max_##T(T a, T b) \
    { \
        return a > b ? a : b; \
    } \
    \
    T P_clamp_##T(T v, T from, T to) \
    { \
        return P_min_##T(P_max_##T(from, v), to); \
    } \

P_TEMPLATE_MIN_MAX_CLAMP(i64)
P_TEMPLATE_MIN_MAX_CLAMP(i32)
P_TEMPLATE_MIN_MAX_CLAMP(i16)
P_TEMPLATE_MIN_MAX_CLAMP(i8)
P_TEMPLATE_MIN_MAX_CLAMP(u64)
P_TEMPLATE_MIN_MAX_CLAMP(u32)
P_TEMPLATE_MIN_MAX_CLAMP(u16)
P_TEMPLATE_MIN_MAX_CLAMP(u8)
P_TEMPLATE_MIN_MAX_CLAMP(f64)
P_TEMPLATE_MIN_MAX_CLAMP(f32)

// NOTE(vanya): File IO functions - Extras
void P_walk_dir(const char* path)
{
    DIR *dir = opendir(path);
    if (!dir) {
        P_print_os_error(path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (
                P_strcmp(entry->d_name, ".") == 0
                || P_strcmp(entry->d_name, "..") == 0
        ) {
            continue;
        }

        char full_path[P_MAX_PATH_LENGTH];
        P_snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (lstat(full_path, &st) == -1) {
            P_print_os_error(full_path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            LOG_INFO("DIR : %s", full_path);
            P_walk_dir(full_path);
        }
        if (S_ISREG(st.st_mode)) {
            LOG_INFO("FILE: %s", full_path);
        }
    }
}
