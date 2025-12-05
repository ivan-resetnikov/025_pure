#include "str_utils.c"

/*
** Structs
*/

typedef struct {
    int path_size;
    char path[P_MAX_PATH_LENGTH];
    size_t file_offset;
    size_t file_size;
} FileEntry;

/*
** Variables
*/

char** in_files_paths = NULL;
int in_files_count = 0;
char** exclusion_patterns = NULL;
int exclusion_patterns_count = 0;

/*
** Declarations
*/

void scan_dir(char* path);
void convert_assets();
size_t get_file_size(char* path);

void convert_bmp_to_image_and_create_neighbour_file(const char* path);

/*
** Implementation
*/

int main(int args_count, char* args[])
{
    // Defaults
    char* in_path = P_strdup("./assets/");
    char* out_path = P_strdup("assets.bin");

    // NOTE(vanya): Inject default exceptions for output files from the app itself
    // The asset convertion pipeline - the one that turns assets out of an art program
    // into the engine-readable format, outputs converted files for each asset.
    // We don't want them back in the pipeline.
    size_t new_size = sizeof(char*) * (exclusion_patterns_count + 1);
    exclusion_patterns = P_realloc(exclusion_patterns, new_size);

    exclusion_patterns[exclusion_patterns_count++] = P_strdup("*.img");

    // Parse args
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
    scan_dir(in_path);

    convert_assets();

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
        P_memset(f->path, 0, P_MAX_PATH_LENGTH);
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
            P_print_os_error("OS error");
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
            LOG_ERROR("Failed to read the input file fully! Written: %zu/%zu bytes.", read_bytes, file_entry->file_size);
            P_free(file_buffer);
            P_fclose(file_io);
            continue;
        }

        size_t written_bytes = P_fwrite(out_file, file_buffer, file_entry->file_size);
        if (written_bytes < file_entry->file_size) {
            LOG_ERROR("Failed to fully write the input file into the output file! Written: %zu/%zu bytes.", written_bytes, file_entry->file_size);
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
    LOG_INFO("Scanning input directory");
    
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


void convert_assets()
{
    LOG_INFO("Converting assets");
    
    for (int i = 0; i < in_files_count; i++) {
        char* in_file_path = in_files_paths[i];
        char* extension = str_sub(in_file_path, str_rfind(in_file_path, '.'), -1);
 
        if (P_strcmp(extension, ".bmp") == 0) {
            LOG_DEBUG("New image file from %s", in_file_path);
            convert_bmp_to_image_and_create_neighbour_file(in_file_path);
        } else {
            LOG_DEBUG("Unhandled extension in: %s", in_file_path);
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


void convert_bmp_to_image_and_create_neighbour_file(const char* path)
{
    LOG_DEBUG("Converting .BMP to .IMG: %s", path);

    // Read BMP
    FILE* file_in = P_fopen(path, "rb");

    u16 magic_number;
    P_fread(file_in, &magic_number, sizeof(magic_number));
    if (magic_number != 0x4D42) { // BM
        LOG_ERROR("Did not find magic number in header!");
        P_fclose(file_in);
        return;
    }

    // Data offset
    P_fseek(file_in, 12, P_SEEK_CUR);
    u32 data_offset;
    P_fread(file_in, &data_offset, sizeof(data_offset));

    // Dimensions
    i32 width, height;
    P_fread(file_in, &width, sizeof(i32));
    P_fread(file_in, &height, sizeof(i32));

    // Planes? and Bits Per Pixel
    u16 planes, bpp;
    P_fread(file_in, &planes, sizeof(planes));
    P_fread(file_in, &bpp, sizeof(bpp));

    // Only support 24-bit or 32-bit BMP
    if (bpp != 24 && bpp != 32) {
        LOG_ERROR("Only support 24-bit or 32-bit BMP");
        P_fclose(file_in);
        return;
    }

    P_fseek(file_in, data_offset, P_SEEK_SET);

    size_t data_size = sizeof(u32) * width * height;
    u32* data = P_malloc(data_size);

    size_t row_padded = ((bpp * width + 31) / 32) * 4;
    u8* row = P_malloc(row_padded);

    for (int y = 0; y < height; ++y) {
        P_fread(file_in, row, row_padded);

        for (int x = 0; x < width; ++x) {
            u8 r, g, b, a = 255;

            if (bpp == 24) {
                r = row[x * 3 + 0];
                g = row[x * 3 + 1];
                b = row[x * 3 + 2];
            } else
            if (bpp == 32) {
                r = row[x * 4 + 0];
                g = row[x * 4 + 1];
                b = row[x * 4 + 2];
                a = row[x * 4 + 3];
            }

            data[y * width + x] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }

    P_fclose(file_in);

    // Write output file
    char* base = str_sub(path, 0, str_rfind(path, '.') - 1);
    char* new_path = str_new_formatted("%s.img", base);

    FILE* file_out = P_fopen(new_path, "wb");
    if (file_out == NULL) {
        LOG_ERROR("Failed to open output file!");
        return;
    }

    P_fwrite(file_out, &width, sizeof(i32));
    P_fwrite(file_out, &height, sizeof(i32));
    P_fwrite(file_out, data, data_size);

    P_fclose(file_out);
}

