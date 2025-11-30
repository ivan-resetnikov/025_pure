#define WIN32

#ifdef __linux__
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <assert.h>

    #include <fnmatch.h>
    
    #define P_assert assert
#endif
#ifdef WIN32
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <assert.h>

    #include <Shlwapi.h>
    
    #define P_assert assert
    #define P_free free
    #define P_strdup strdup
    #define P_malloc malloc
    #define P_calloc calloc
    #define P_vsnprintf vsnprintf
#endif

typedef uint8_t bool;
#define true 1
#define false 0


char* bytes_to_human_readable(size_t bytes);
void str_path_ensure_forward_slash(char* path);
char* str_new_formatted(const char* fmt, ...);
bool str_starts_with(char* str, char* prefix);
char* str_override(char* dest, char* str);
bool str_wildcard_match(char* str, char* pattern);


char* bytes_to_human_readable(size_t bytes)
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};

    const int units_count = sizeof(units) / sizeof(char*);
    double count = (double)bytes;
    int i = 0;

    while (count >= 1024.0 && i < units_count) {
        count /= 1024.0;
        i++;
    }

    return str_new_formatted("%.2f %s", count, units[i]);
}


void str_path_ensure_forward_slash(char* path)
{
    P_assert(path != NULL);

    char* ptr_cpy = path;
    while (*ptr_cpy != '\0') {
        if (*ptr_cpy == '\\') *ptr_cpy = '/';

        ptr_cpy++;
    }
}


bool str_starts_with(char* str, char* prefix)
{
    size_t len_prefix = strlen(prefix);

    P_assert(strlen(str) > len_prefix);

    return strncmp(str, prefix, len_prefix) == 0;
}


char* str_override(char* dest, char* str)
{
    P_free(dest);
    char* new_dest = P_strdup(str);
    return new_dest;
}


char* str_new_formatted(const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int str_len = P_vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    char* str = (char*)P_calloc(1, str_len);
    if (!str) {
        SDL_Log("Failed to allocate enough memory for the new string.");
        return NULL;
    }

    va_start(args, fmt);
    P_vsnprintf(str, str_len, fmt, args);
    va_end(args);

    return str;
}


bool str_wildcard_match(char* str, char* pattern)
{
#ifdef __linux__
    return (fnmatch(pattern, str, 0) == 0);
#endif
#ifdef WIN32
    return (bool)PathMatchSpecA(str, pattern);
#endif
}
