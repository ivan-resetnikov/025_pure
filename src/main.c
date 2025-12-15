#include "entrypoint.c"


#define ASSETS_FILE_PATH "assets.bin"


typedef struct {
    char* path;
    u32 offset;
    u32 size;
} FileEntry;

typedef struct {
    i32 start_x;
    i32 start_y;
    i32 end_x;
    i32 end_y;
} AABB;

typedef struct {
    u32* data;
    i32 width;
    i32 height;
} Image;

typedef struct {
    Image* image;
} Tile;

typedef struct {
    Tile* tiles;
    i32 width;
    i32 height;
    i32 tile_width;
    i32 tile_height;
} Tilemap;


P_IOStream asset_io;
FileEntry* asset_io_files;
i32 asset_io_files_count;

Image img_test;
Image img_diorite;
Tilemap tm;


void io_init_file_stream();

FileEntry* io_get_file_entry(const char* path);
char* io_load_txt(const char* path);
void io_load_img(Image* image, const char* path);

void draw_tilemap(Tilemap* tilemap);
void blit_image_to_frame_simple(Image* image, i32 offset_x, i32 offset_y, float scale);


void P_ready()
{
    io_init_file_stream();

    LOG_CRITICAL("%s", io_load_txt("./assets/text.txt"));

    io_load_img(&img_test, "./assets/textures/tiles/test.img");
    io_load_img(&img_diorite, "./assets/textures/tiles/diorite.img");

    // Test tilemap
    tm.width = 16;
    tm.height = 16;
    tm.tile_width = 8;
    tm.tile_height = 8;
    tm.tiles = P_malloc(sizeof(Tile) * tm.width * tm.height);

    // Test -> Randomise times
    for (i32 x = 0; x < tm.width; x++) {
        for (i32 y = 0; y < tm.width; y++) {
            i32 tile_index = y * tm.width + x;
            Tile* tile = &tm.tiles[tile_index];

            switch (rand() % 3) {
            case 0: {
                tile->image = NULL;
            } break;
            case 1: {
                tile->image = &img_diorite;
            } break;
            case 2: {
                tile->image = &img_test;
            } break;
            }
        }
    }
}


void P_iterate()
{
    tick++;

    P_clear_backbuffer(0, 0, 0);

    draw_tilemap(&tm);
}


void io_init_file_stream()
{
    LOG_DEBUG("Opening asset IO stream");

    P_assert(P_IOStream_FromFile(&asset_io, ASSETS_FILE_PATH, "rb") == P_ERROR_OK, "Failed to open asset IO steam!");

    // NOTE(vanya): Get file count
    P_IOStream_Read(&asset_io, &asset_io_files_count, sizeof(i32), P_NULL);
    LOG_DEBUG("Loading %d files", asset_io_files_count);

    // NOTE(vanya): Load file index
    asset_io_files = P_realloc(asset_io_files, sizeof(FileEntry) * asset_io_files_count);

    for (int i = 0; i < asset_io_files_count; i++) {
        FileEntry* f = &asset_io_files[i];

        // NOTE(vanya): Path
        int path_size = 0;
        P_IOStream_Read(&asset_io, &path_size, sizeof(i32), P_NULL);

        f->path = P_malloc(path_size);
        P_IOStream_Read(&asset_io, f->path, path_size, P_NULL);
        f->path[path_size] = '\0';

        // NOTE(vanya): Offset + size
        P_IOStream_Read(&asset_io, &f->offset, sizeof(u32), P_NULL);
        P_IOStream_Read(&asset_io, &f->size, sizeof(u32), P_NULL);
    }
}


FileEntry* io_get_file_entry(const char* path)
{
    FileEntry* file_entry = NULL;
    for (int file_index = 0; file_index < asset_io_files_count; file_index++)
    {
        FileEntry* candidate_file_entry = &asset_io_files[file_index];

        if (P_strcmp((const char*)candidate_file_entry->path, path) == 0) {
            file_entry = candidate_file_entry;
            break;
        }
    }

    if (file_entry == NULL) {
        LOG_ERROR("Could not find asset file entry with path: %s!", path);
    }

    return file_entry;
}


char* io_load_txt(const char* path)
{
    FileEntry* file_entry = io_get_file_entry(path);
    if (file_entry == NULL) {
        LOG_ERROR("Could not find text file!");
        return NULL;
    }

    char* text_buffer = (char*)P_calloc(1, file_entry->size + 1);
    if (!text_buffer) {
        LOG_ERROR("Failed to allocate text file buffer!");
        return NULL;
    }

    size_t a;
    P_IOStream_Seek(&asset_io, file_entry->offset, P_IO_STREAM_SEEK_MODE_SET);
    P_IOStream_Read(&asset_io, text_buffer, file_entry->size, &a);
    
    // NOTE(vanya): Text files come without a null-terminator
    text_buffer[file_entry->size] = '\0';

    return text_buffer;
}


void io_load_img(Image* image, const char* path)
{
    FileEntry* file_entry = io_get_file_entry(path);
    if (file_entry == NULL) {
        LOG_ERROR("Could not find image file!");
        goto fail;
    }

    P_IOStream_Seek(&asset_io, file_entry->offset, P_IO_STREAM_SEEK_MODE_SET);

    if (P_IOStream_Read(&asset_io, &image->width, sizeof(i32), NULL) != P_ERROR_OK) { LOG_ERROR("Failed to read file!"); goto fail; };
    if (P_IOStream_Read(&asset_io, &image->height, sizeof(i32), NULL) != P_ERROR_OK) { LOG_ERROR("Failed to read file!"); goto fail; };

    size_t data_size = sizeof(u32) * image->width * image->height;
    image->data = P_malloc(data_size);

    if (image->data == P_NULL) { LOG_ERROR("Failed to allocate image buffer!"); goto fail; };

    if (P_IOStream_Read(&asset_io, image->data, data_size, NULL) != P_ERROR_OK) { LOG_ERROR("Failed to read the image buffer!"); goto fail; };

    return;

fail:
    image->width = 0;
    image->height = 0;
    image->data = P_NULL;
}


void draw_tilemap(Tilemap* tilemap)
{
    for (i32 x = 0; x < tilemap->width; x++) {
        for (i32 y = 0; y < tilemap->width; y++) {
            i32 tile_index = y * tilemap->width + x;
            Tile* tile = &tilemap->tiles[tile_index];

            if (tile->image != NULL) {
                blit_image_to_frame_simple(
                        tile->image, 
                        x * tilemap->tile_width * PIXEL_SCALE,
                        y * tilemap->tile_height * PIXEL_SCALE,
                        (float)PIXEL_SCALE);
            }
        }
    }
}


void blit_image_to_frame_simple(Image* image, i32 offset_x, i32 offset_y, float scale)
{
    // AABB
    AABB aabb;
    aabb.start_x = P_clamp_i32(offset_x, 0, backbuffer_width);
    aabb.start_y = P_clamp_i32(offset_y, 0, backbuffer_height);
    aabb.end_x = P_clamp_i32(offset_x + P_floor_i32((double)image->width * scale), 0, backbuffer_width);
    aabb.end_y = P_clamp_i32(offset_y + P_floor_i32((double)image->height * scale), 0, backbuffer_height);

    i32 x = 0;
    for(i32 screen_x = aabb.start_x; screen_x < aabb.end_x; screen_x++) {
        i32 y = 0;
        for(i32 screen_y = aabb.start_y; screen_y < aabb.end_y; screen_y++) {
            u32 pixel = image->data[P_floor_i32((float)y / scale) * image->width + P_floor_i32((float)x / scale)];
            
            u8 a = (pixel >> 24) & 0xFF; // NOTE(vanya): We can later use alpha channel to sample the framebuffer and blend the colors
            u8 r = (pixel >> 16) & 0xFF;
            u8 g = (pixel >> 8) & 0xFF;
            u8 b = pixel & 0xFF;

            P_set_pixel(screen_x, screen_y, r, g, b);
            y++;
        }
        x++;
    }
}

