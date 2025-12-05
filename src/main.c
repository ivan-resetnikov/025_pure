typedef struct {
    int start_x;
    int start_y;
    int end_x;
    int end_y;
} AABB;

typedef struct {
    u32* data;
    int width;
    int height;
} Image;

typedef struct {
    Image* image;
} Tile;

typedef struct {
    Tile* tiles;
    int width;
    int height;
    int tile_width;
    int tile_height;
} Tilemap;


Image img_diorite;
Tilemap tm;


void load_img(Image* image, const char* path);
void draw_tilemap(Tilemap* tilemap);
void blit_image_to_frame_simple(Image* image, int offset_x, int offset_y, float scale);


void P_ready()
{
    load_img(&img_diorite, "/home/ivan/dev/c++/025_rpg/assets/textures/tiles/diorite.img");

    // Test tilemap
    tm.width = 16;
    tm.height = 16;
    tm.tile_width = 8;
    tm.tile_height = 8;
    tm.tiles = P_malloc(sizeof(Tile) * tm.width * tm.height);

    // Test -> Randomise times
    for (int x = 0; x < tm.width; x++) {
        for (int y = 0; y < tm.width; y++) {
            int tile_index = y * tm.width + x;
            Tile* tile = &tm.tiles[tile_index];

            tile->image = rand() % 2 ? &img_diorite : NULL;
        }
    }
}

void P_iterate()
{
    tick++;

    P_clear_backbuffer(0, 0, 0);

    draw_tilemap(&tm);
}

void load_img(Image* image, const char* path)
{
    // TODO(vanya): This code is terrible
    // and we should read from the file DB
    // NOT directly from the platform!
    //
    // Rework!

    FILE* f = P_fopen(path, "rb");
    P_assert(f != NULL, "null file");
    P_fread(f, &image->width, sizeof(i32));
    LOG_DEBUG("%d", image->width);
    P_fread(f, &image->height, sizeof(i32));
    LOG_DEBUG("%d", image->height);

    size_t data_size = sizeof(u32) * image->width * image->height;
    image->data = malloc(data_size);
    P_fread(f, image->data, data_size);
    P_fclose(f);
}

void draw_tilemap(Tilemap* tilemap)
{
    for (int x = 0; x < tilemap->width; x++) {
        for (int y = 0; y < tilemap->width; y++) {
            int tile_index = y * tilemap->width + x;
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

void blit_image_to_frame_simple(Image* image, int offset_x, int offset_y, float scale)
{
    // AABB
    AABB aabb;
    aabb.start_x = P_clamp_i32(offset_x, 0, backbuffer_width);
    aabb.start_y = P_clamp_i32(offset_y, 0, backbuffer_height);
    aabb.end_x = P_clamp_i32(offset_x + P_floor_i32((float)image->width * scale), 0, backbuffer_width);
    aabb.end_y = P_clamp_i32(offset_y + P_floor_i32((float)image->height * scale), 0, backbuffer_height);

    int x = 0;
    for(int screen_x = aabb.start_x; screen_x < aabb.end_x; screen_x++) {
        int y = 0;
        for(int screen_y = aabb.start_y; screen_y < aabb.end_y; screen_y++) {
            u32 pixel = image->data[P_floor_i32((float)y / scale) * image->width + P_floor_i32((float)x / scale)];
            
            uint8_t a = (pixel >> 24) & 0xFF; // NOTE(vanya): We can later use alpha channel to sample the framebuffer and blend the colors
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            P_set_pixel(screen_x, screen_y, r, g, b);
            y++;
        }
        x++;
    }
}

