#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

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

// NOTE(vanya): STL abstraction
#define P_memset(dest, fill_byte, size) memset(dest, fill_byte, size)
#define P_malloc(size) malloc(size)
#define P_calloc(num, size) calloc(num, size)
#define P_realloc(old_ptr, size) realloc(old_ptr, size)
#define P_free(ptr) free(ptr)

#define P_strlen(str) strlen(str)
#define P_strncmp(str_a, str_b, size) strncmp(str_a, str_b, size)
#define P_strdup(str) strdup(str)
#define P_vsnprintf(buffer, size, format, vargs) vsnprintf(buffer, size, format, vargs)

#define P_roundi(f) (i32)round(f)
#define P_floori(f) (i32)floor(f)
#define P_roundf(f) (f32)round(f)
#define P_floorf(f) (f32)floor(f)

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

