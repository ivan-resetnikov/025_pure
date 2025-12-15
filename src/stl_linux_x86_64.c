#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>


// NOTE(vanya): Platform types
#define P_NULL NULL

typedef uint8_t P_bool;
#define P_true (uint8_t)1
#define P_false (uint8_t)0

typedef int8_t P_i8;
typedef int16_t P_i16;
typedef int32_t P_i32;
typedef int64_t P_i64;

typedef uint8_t P_u8;
typedef uint16_t P_u16;
typedef uint32_t P_u32;
typedef uint64_t P_u64;

typedef float P_f32;
typedef double P_f64;

typedef size_t P_size;


#ifdef P_DEFINE_TYPE_SHORTHANDS
#define bool P_bool
#define true P_true
#define false P_false

#define i8 P_i8
#define i16 P_i16
#define i32 P_i32
#define i64 P_i64

#define u8 P_u8
#define u16 P_u16
#define u32 P_u32
#define u64 P_u64

#define f32 P_f32
#define f64 P_f64
#endif


typedef enum {
    P_ERROR_OK = 0,

    P_ERROR_INVALID_PARAMETER,

    P_ERROR_FAILED_TO_OPEN_FILE,
    P_ERROR_FAILED_TO_CLOSE_FILE,
    P_ERROR_FAILED_TO_FLUSH,
    P_ERROR_FAILED_TO_READ_FILE,
    P_ERROR_FAILED_TO_WRITE_FILE,
    P_ERROR_FAILED_TO_SEEK,
    P_ERROR_FAILED_TO_TELL,
} P_Error;


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


// NOTE(vanya): Logging macros
#define P_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define TERM_COL_BLACK_FG "\x1b[0;30m"
#define TERM_COL_RED_FG "\x1b[0;31m"
#define TERM_COL_GREEN_FG "\x1b[0;32m"
#define TERM_COL_YELLOW_FG "\x1b[0;33m"
#define TERM_COL_BLUE_FG "\x1b[0;34m"
#define TERM_COL_MAGENTA_FG "\x1b[0;35m"
#define TERM_COL_CYAN_FG "\x1b[0;36m"
#define TERM_COL_WHITE_FG "\x1b[0;37m"

#define TERM_COL_BLACK_BG "\x1b[0;40m"
#define TERM_COL_RED_BG "\x1b[0;41m"
#define TERM_COL_GREEN_BG "\x1b[0;42m"
#define TERM_COL_YELLOW_BG "\x1b[0;43m"
#define TERM_COL_BLUE_BG "\x1b[0;44m"
#define TERM_COL_MAGENTA_BG "\x1b[0;45m"
#define TERM_COL_CYAN_BG "\x1b[0;46m"
#define TERM_COL_WHITE_BG "\x1b[0;47m"

#define TERM_COL_RESET "\x1b[0;39;49m"

#define LOG_DEBUG(format_string, ...) \
    fprintf(stdout, "debug:    %s:%d %s - " format_string "\n", P_FILENAME, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_INFO(format_string, ...) \
    fprintf(stdout, "info:     %s:%d %s - " format_string "\n", P_FILENAME, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_WARNING(format_string, ...) \
    fprintf(stdout, TERM_COL_YELLOW "warning:  %s:%d %s - " format_string TERM_COL_RESET "\n", P_FILENAME, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_ERROR(format_string, ...) \
    fprintf(stderr, TERM_COL_RED_FG "error:    %s:%d %s - " format_string TERM_COL_RESET "\n", P_FILENAME, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_CRITICAL(format_string, ...) \
    fprintf(stderr, TERM_COL_WHITE_FG TERM_COL_RED_BG "critical: %s:%d %s - " format_string TERM_COL_RESET "\n", P_FILENAME, __LINE__, __func__, ##__VA_ARGS__)


// NOTE(vanya): Debugging macros
#define P_assert(exp, msg) \
    { if (!(exp)) { LOG_CRITICAL("Assetion failed! - " msg); exit(1); } }


// NOTE(vanya): File IO
#define P_MAX_PATH_LENGTH PATH_MAX


typedef enum {
    P_IO_STREAM_SEEK_MODE_SET,
    P_IO_STREAM_SEEK_MODE_CUR,
    P_IO_STREAM_SEEK_MODE_END
} P_IOStream_SeekMode;


typedef struct {
    FILE* stream;
} P_IOStream;


P_Error P_IOStream_FromFile(P_IOStream* p_dest, const char* p_path, const char* p_modes);

P_Error P_IOStream_Close(P_IOStream* p_stream);

P_Error P_IOStream_Flush(P_IOStream* p_stream);

P_Error P_IOStream_Read(P_IOStream* p_stream, void* p_dest, size_t p_size, P_size* out_bytes_read);

P_Error P_IOStream_Write(P_IOStream* p_stream, void* p_src, size_t p_size, P_size* p_out_bytes_written);


P_Error P_IOStream_FromFile(P_IOStream* p_dest, const char* p_path, const char* p_modes)
{
    P_assert(p_dest != P_NULL, "p_dest is null!");

    p_dest->stream = fopen(p_path, p_modes);

    if (p_dest->stream == P_NULL) {
        return P_ERROR_FAILED_TO_OPEN_FILE;
    }

    return P_ERROR_OK;
}


P_Error P_IOStream_Close(P_IOStream* p_stream)
{
    P_assert(p_stream != P_NULL, "p_target is null!");

    if (fclose(p_stream->stream) != 0) {
        return P_ERROR_FAILED_TO_CLOSE_FILE;
    }

    p_stream->stream = P_NULL;

    return P_ERROR_OK;
}


P_Error P_IOStream_Flush(P_IOStream* p_stream)
{
    P_assert(p_stream != P_NULL, "P_IO_Flush: stream is null!");

    if (fflush(p_stream->stream) != 0) {
        return P_ERROR_FAILED_TO_FLUSH;
    }

    return P_ERROR_OK;
}


P_Error P_IOStream_Read(P_IOStream* p_src, void* p_dest, size_t p_size, P_size* out_bytes_read)
{
    P_assert(p_src != P_NULL, "p_src is null!");
    P_assert(p_dest != P_NULL, "p_dest is null!");

    P_size bytes_read = fread(p_dest, 1, p_size, p_src->stream);

    if (bytes_read < p_size) {
        return P_ERROR_FAILED_TO_READ_FILE;
    }

    if (out_bytes_read != P_NULL) *out_bytes_read = bytes_read;

    return P_ERROR_OK;
}


P_Error P_IOStream_Write(P_IOStream* p_dest, void* p_src, size_t p_size, P_size* out_bytes_written)
{
    P_assert(p_dest != P_NULL, "p_dest is null!");
    P_assert(p_src != P_NULL, "p_src is null!");

    P_size bytes_written = fwrite(p_src, 1, p_size, p_dest->stream);

    if (out_bytes_written != P_NULL) *out_bytes_written = bytes_written;

    if (bytes_written < p_size) {
        return P_ERROR_FAILED_TO_WRITE_FILE;
    }

    return P_ERROR_OK;
}


P_Error P_IOStream_Seek(P_IOStream* p_stream, long offset, P_IOStream_SeekMode mode)
{
    P_assert(p_stream != P_NULL, "p_stream is null!");

    int origin;
    switch (mode) {
        case P_IO_STREAM_SEEK_MODE_SET: origin = SEEK_SET; break;
        case P_IO_STREAM_SEEK_MODE_CUR: origin = SEEK_CUR; break;
        case P_IO_STREAM_SEEK_MODE_END: origin = SEEK_END; break;
        default: return P_ERROR_INVALID_PARAMETER;
    }

    if (fseek(p_stream->stream, offset, origin) != 0) {
        return P_ERROR_FAILED_TO_SEEK;
    }

    return P_ERROR_OK;
}

P_Error P_IOStream_Tell(P_IOStream* p_stream, size_t* out_position)
{
    P_assert(p_stream != P_NULL, "p_stream is null!");
    P_assert(out_position != P_NULL, "out_position is null!");

    *out_position = ftell(p_stream->stream);
    
    if (*out_position < 0) {
        return P_ERROR_FAILED_TO_TELL;
    }

    return P_ERROR_OK;
}


#define P_fgetc(io) fgetc(io)
#define P_fputc(io, c) fputc(c, io)

#define P_ftell(io) ftell(io)
#define P_fseek(io, offset, origin) fseek(io, offset, origin)
#define P_frewind(io) frewind(io)

#define P_ferror(io) ferror(io)
#define P_fremove(path) remove(path)
#define P_frename(path_old, path_new) rename(path_old, path_new)

#define P_print_os_error(prefix) perror(prefix)



// NOTE(vanya): File IO - Extras
void P_walk_dir(const char* path, char*** dest_files, int* dest_count)
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
            P_walk_dir(full_path, dest_files, dest_count);
        }
        if (S_ISREG(st.st_mode)) {
	        // Add file
            char** files_tmp = P_realloc(*dest_files, (*dest_count + 1) * sizeof(char*));
            if (!files_tmp) {
                LOG_ERROR("Could not allocate enough space for the files list");
            };
            *dest_files = files_tmp;

            (*dest_files)[*dest_count] = P_malloc(strlen(full_path) + 1);
            strcpy((*dest_files)[*dest_count], full_path);

            (*dest_count)++;

        }
    }
}


// NOTE(vanya): Math helper functions
#define P_abs_f32(f) (P_f32)abs(f)
#define P_abs_i32(i) (P_i32)abs(i)

#define P_round_i32(f) (P_i32)round(f)
#define P_round_f32(f) (P_f32)round(f)

#define P_floor_i32(f) (P_i32)floor(f)
#define P_floor_f32(f) (P_f32)floor(f)

#define P_ceil_i32(f) (P_i32)ceil(f)
#define P_ceil_f32(f) (P_f32)ceil(f)

#define P_TEMPLATE_MIN_MAX_CLAMP(type_name, T) \
    T P_min_##type_name(T a, T b) { return a < b ? a : b; } \
    \
    T P_max_##type_name(T a, T b) { return a > b ? a : b; } \
    \
    T P_clamp_##type_name(T v, T from, T to) { return P_min_##type_name(P_max_##type_name(from, v), to); }

P_TEMPLATE_MIN_MAX_CLAMP(i8, P_i8)
P_TEMPLATE_MIN_MAX_CLAMP(i16, P_i16)
P_TEMPLATE_MIN_MAX_CLAMP(i32, P_i32)
P_TEMPLATE_MIN_MAX_CLAMP(i64, P_i64)

P_TEMPLATE_MIN_MAX_CLAMP(u8, P_u8)
P_TEMPLATE_MIN_MAX_CLAMP(u16, P_u16)
P_TEMPLATE_MIN_MAX_CLAMP(u32, P_u32)
P_TEMPLATE_MIN_MAX_CLAMP(u64, P_u64)

P_TEMPLATE_MIN_MAX_CLAMP(f32, P_f32)
P_TEMPLATE_MIN_MAX_CLAMP(f64, P_f64)
