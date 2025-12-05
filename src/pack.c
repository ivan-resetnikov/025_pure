#include "str_utils.c"

#define MAX_PATH_LENGTH P_MAX_PATH_LENGTH

typedef struct {
    int path_size;
    char path[MAX_PATH_LENGTH];
    size_t file_offset;
    size_t file_size;
} FileEntry;

/*
** Declarations
*/

void scan_dir(char* path);
size_t get_file_size(char* path);

/*
** Implementation
*/

char** in_files_paths = NULL;
int in_files_count = 0;
char** exclusion_patterns = NULL;
int exclusion_patterns_count = 0;

int main(int args_count, char* args[])
{
    // Parse args
    char* in_path = P_strdup("assets");
    char* out_path = P_strdup("assets.bin");

    for (int arg_index = 1; arg_index < args_count; arg_index++) {
        char* arg = args[arg_index];

        if (str_starts_with(arg, "-i:")) {
            in_path = str_override(in_path, arg + P_strlen("-i:"));
        }

        if (str_starts_with(arg, "-o:")) {
            out_path = str_override(out_path, arg + P_strlen("-o:"));
        }

        if (str_starts_with(arg, "-x:")) {
            size_t new_size = sizeof(char*) * (exclusion_patterns_count + 1);
            exclusion_patterns = P_realloc(exclusion_patterns, new_size);

            exclusion_patterns[exclusion_patterns_count++] = P_strdup(arg + 3);
        }
    }

    LOG_INFO("Input dir: %s", in_path);
    LOG_INFO("Ouput file: %s", out_path);
    LOG_INFO("Excluded files: %d", exclusion_patterns_count);

    // Scan
    LOG_INFO("Scanning input directory");
    scan_dir(in_path);

    // Create output file file
    LOG_INFO("Creating output file");
    FILE* out_file = P_fopen(out_path, "w");
    if (!out_file) {
        LOG_ERROR("Failed to open output file! SDL error:\n%s", "TODO_ERROR");
        return 1;
    }
    
    // Header
    LOG_DEBUG("Writing header");
    P_fwrite(out_file, &in_files_count, sizeof(int));
    
    size_t header_size = P_ftell(out_file);
    
    // File index
    LOG_DEBUG("Creating file index");
    FileEntry* file_entires = NULL;
    int file_entires_count = 0;
    
    // We will first write the header & the file entires index then pack
    // then raw data of each file, one after another.
    // We use increment this variable evry time a file's data is written to track when the next one begins.
    size_t file_entires_size = sizeof(FileEntry) * in_files_count;
    file_entires = P_malloc(file_entires_size);

    size_t index_section_size = 0;
    size_t file_offset = 0;
    for (int i = 0; i < in_files_count; i++) {
        char* file_path = in_files_paths[i];
        LOG_DEBUG("Creating entry #%d: %s", i, file_path);

        FileEntry* f = &file_entires[file_entires_count++];

        // Path
        f->path_size = P_strlen(file_path) + 1;
        P_memset(f->path, 0, MAX_PATH_LENGTH);
        P_memcpy(f->path, file_path, f->path_size + 1);

        // File size
        size_t file_size = get_file_size(f->path);
        if (file_size == 0) {
            LOG_ERROR("Failed to measure file size: %s, skipping!", f->path);
            continue;
        }

        f->file_size = file_size;

        // Offset
        // IMPORTANT: This is the offset that is the sum of the size of every file from earlier.
        // This it not a global file offset YET! We need to ADD to this variable:
        // - the size of the header
        // - and the size of the index section (WHICH IS DYNAMIC!)
        f->file_offset = file_offset;
        file_offset += f->file_size;

        // Index section
        index_section_size += (
            sizeof(int)
            + f->path_size
            + sizeof(size_t)
            + sizeof(size_t)
        );
    }
    
    // Converting dump (sum of sizes of files from earlier) offset to global (output file) offset
    for (int i = 0; i < file_entires_count; i++) {
        FileEntry* f = &file_entires[i];

        f->file_offset += header_size + index_section_size;
    }

    // Write file index
    LOG_DEBUG("Writing file index");
    for (int i = 0; i < file_entires_count; i++) {
        FileEntry* f = &file_entires[i];
        
        // Path
        P_fwrite(out_file, &f->path_size, sizeof(int));
        P_fwrite(out_file, f->path, f->path_size);

        // Offset + size
        P_fwrite(out_file, &f->file_offset, sizeof(size_t));
        P_fwrite(out_file, &f->file_size, sizeof(size_t));
    }

    // Write file contents
    LOG_DEBUG("Writing file contents");
    for (int i = 0; i < file_entires_count; i++) {
        FileEntry* file_entry = &file_entires[i];
        
        LOG_INFO("Dumping %s: %s", bytes_to_human_readable(file_entry->file_size), file_entry->path);

        FILE* file_io = P_fopen(file_entry->path, "r");
        if (!file_io) {
            LOG_ERROR("Failed to open file: %s, skipping!", file_entry->path);
            continue;
        }

        void* file_buffer = P_malloc(file_entry->file_size);
        if (!file_buffer) {
            LOG_ERROR("Failed to allocate file buffer: %s, skipping!", file_entry->path);
            P_fclose(file_io);
            continue;
        }

        size_t read_bytes = P_fread(file_io, file_buffer, file_entry->file_size);
        if (read_bytes < file_entry->file_size) {
            LOG_ERROR("Failed to read the input file fully! Written: %zu/%zu bytes.", read_bytes);
            P_free(file_buffer);
            P_fclose(file_io);
            continue;
        }

        size_t written_bytes = P_fwrite(out_file, file_buffer, file_entry->file_size);
        if (written_bytes < file_entry->file_size) {
            LOG_ERROR("Failed to fully write the input file into the output file! Written: %zu/%zu bytes.", written_bytes);
            P_free(file_buffer);
            P_fclose(file_io);
            continue;
        }

        P_free(file_buffer);
        P_fclose(file_io);
    }

    size_t out_file_size = P_ftell(out_file);

    P_fclose(out_file);

    LOG_INFO("%s created successfully! Final size: %s", out_path, bytes_to_human_readable(out_file_size));

    return 0;
}


void scan_dir(char* path)
{
    char** all_files = NULL;
    int all_files_count = 0;
    P_walk_dir(path, &all_files, &all_files_count);

    for (int i = 0; i < all_files_count; i++) {
        char* candidate_files_path = all_files[i];

        // NOTE(vanya): Compare against exclusion patterns
        bool was_excluded = false;

        for (int i = 0; i < exclusion_patterns_count; i++) {
            char* pattern = exclusion_patterns[i];

            if (str_wildcard_match(candidate_files_path, pattern)) {
                was_excluded = true;
                LOG_DEBUG("Excluding %s, matched exclusion pattern: %s", candidate_files_path, pattern);
                break;
            }
        }

        if (!was_excluded) {
            LOG_DEBUG("Packing %s", candidate_files_path);
            
            size_t new_size = sizeof(char*) * (in_files_count + 1);
            in_files_paths = P_realloc(in_files_paths, new_size);

            in_files_paths[in_files_count] = P_strdup(candidate_files_path);

            in_files_count++;
        }
    }
}


size_t get_file_size(char* path)
{
    FILE* f = P_fopen(path, "r");
    if (!f) {
        LOG_ERROR("Failed to open file: %s!", path);
        P_print_os_error("OS error");
        return 0;
    }

    P_fseek(f, 0, P_SEEK_END);

    size_t file_size = P_ftell(f);

    P_fclose(f);

    return file_size;
}

