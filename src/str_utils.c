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


bool str_wildcard_match(char* str, char* pattern)
{
#ifdef __linux__
    return (fnmatch(pattern, str, 0) == 0);
#endif
#ifdef WIN32
    return (bool)PathMatchSpecA(str, pattern);
#endif
}
