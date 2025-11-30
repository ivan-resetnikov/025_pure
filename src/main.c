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


void load_bmp(Image* image);
void draw_tilemap(Tilemap* tilemap);
void blit_image_to_frame_simple(Image* image, int offset_x, int offset_y, float scale);
int min(int a, int b);
int max(int a, int b);
int clampi(int v, int from, int to);


void P_ready()
{
    load_bmp(&img_diorite);

    // Test tilemap
    tm.width = 10;
    tm.height = 10;
    tm.tile_width = 3;
    tm.tile_height = 3;
    tm.tiles = P_malloc(sizeof(Tile) * tm.width * tm.height);
}

void P_iterate()
{
    tick++;

    P_clear_backbuffer(0, 0, 0);

    // Test -> Randomise times
    for (int x = 0; x < tm.width; x++) {
        for (int y = 0; y < tm.width; y++) {
            int tile_index = y * tm.width + x;
            Tile* tile = &tm.tiles[tile_index];

            tile->image = rand() % 2 ? &img_diorite : NULL;
        }
    }

    draw_tilemap(&tm);
}

void load_bmp(Image* image)
{
    // Test image
    static u32 image_data[] = {
        0xFFFF0000, 0xFF0000FF, 0xFF00FF00,
        0xFF00FF00, 0xFFFF0000, 0xFF0000FF,
        0xFF0000FF, 0xFF00FFFF, 0xFFFF0000,
    };

    image->width = 3;
    image->height = 3;
    image->data = image_data;
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
    aabb.start_x = clampi(offset_x, 0, backbuffer_width);
    aabb.start_y = clampi(offset_y, 0, backbuffer_height);
    aabb.end_x = clampi(offset_x + P_floor((float)image->width * scale), 0, backbuffer_width);
    aabb.end_y = clampi(offset_y + P_floor((float)image->height * scale), 0, backbuffer_height);

    int x = 0;
    for(int screen_x = aabb.start_x; screen_x < aabb.end_x; screen_x++) {
        int y = 0;
        for(int screen_y = aabb.start_y; screen_y < aabb.end_y; screen_y++) {
            u32 pixel = image->data[P_floor((float)y / scale) * image->width + P_floor((float)x / scale)];
            
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

inline int min(int a, int b)
{
    return a < b ? a : b;
}

inline int max(int a, int b)
{
    return a > b ? a : b;
}

int clampi(int v, int from, int to)
{
    return min(max(from, v), to);
}

