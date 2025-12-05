#ifdef P_LINUX_X86_64
    #include "stl_linux_x86_64.c"
    #include <fnmatch.h>
#endif
#ifdef P_WINDOWS_X86_64
    #include "stl_windows_x86_64.c"
    #include <Shlwapi.h>
#endif


char* bytes_to_human_readable(size_t bytes);
void str_path_ensure_forward_slash(char* path);
char* str_new_formatted(const char* fmt, ...);
bool str_starts_with(char* str, char* prefix);
char* str_override(char* dest, char* str);
bool str_wildcard_match(char* str, char* pattern);


char* bytes_to_human_readable(size_t bytes)
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    i32 units_count = sizeof(units) / sizeof(char*);

    size_t count = (size_t)bytes;
    i32 i = 0;
    while (count >= 1024.0 && i < units_count - 1) {
        count /= 1024.0;
        i++;
    }
    
    return str_new_formatted("%.2f %s", (float)bytes, units[i]);
}


void str_path_ensure_forward_slash(char* path)
{
    P_assert(path != NULL, "No path provided!");

    char* ptr_cpy = path;
    while (*ptr_cpy != '\0') {
        if (*ptr_cpy == '\\') *ptr_cpy = '/';

        ptr_cpy++;
    }
}


bool str_starts_with(char* str, char* prefix)
{
    size_t len_prefix = P_strlen(prefix);

    P_assert(P_strlen(str) > len_prefix, "Prefix longer than provided string!");

    return P_strncmp(str, prefix, len_prefix) == 0;
}


char* str_override(char* dest, char* str)
{
    P_assert(dest != NULL, "No destination string provided!");
    P_assert(str != NULL, "No override string provided!");

    P_free(dest);
    char* new_dest = P_strdup(str);
    return new_dest;
}


char *str_new_formatted(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int str_len = P_vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (str_len < 0) {
        va_end(args);
        return NULL;
    }

    char *buffer = P_malloc((str_len) * sizeof(char));
    if (!buffer) {
        va_end(args);
        return NULL;
    }

    P_vsnprintf(buffer, str_len + 1, fmt, args);
    va_end(args);

    return buffer;
}


char* str_sub(const char* str, int start, int end) {
    int str_len = P_strlen(str);

    // NOTE(vanya): Convert negative indices to real indices
    if (end < 0) {
        P_assert(P_abs_i32(end) < str_len, "Negative end index escaped the string length!");
        end = str_len + end;
    }
    if (start < 0) {
        P_assert(P_abs_i32(start) < str_len, "Negative end index escaped the string length!");
        start = str_len + start;
    }

    P_assert(start <= str_len, "Start beyond string length!");
    P_assert(end <= str_len, "End beyond string length!");
    P_assert(start < end, "Start beyond end!");

    int len = end - start + 1;

    char* result = malloc(len + 1); // NOTE(vanya): +1 for null terminator
    if (!result) {
        LOG_ERROR("Failed to allocate substring!");
        return NULL;
    }

    strncpy(result, str + start, len);
    result[len] = '\0'; // NOTE(vanya): Null-terminate
    return result;
}


int str_lfind(const char *str, char ch)
{
    P_assert(str != NULL, "String is NULL!");

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ch) {
            return i;
        }
    }

    return -1;
}


int str_rfind(const char *str, char ch)
{
    P_assert(str != NULL, "String is NULL!");

    for (int i = P_strlen(str) - 1; i > 0; i--) {
        if (str[i] == ch) {
            return i;
        }
    }

    return -1;
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
