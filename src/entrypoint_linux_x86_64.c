#define P_DEFINE_TYPE_SHORTHANDS
#include "stl.c"

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>


// NOTE(vanya): Frame backbuffer
#define PIXEL_SCALE 10

int backbuffer_width = 640;
int backbuffer_height = 480;
u32* backbuffer = NULL;

// NOTE(vanya): Window 
bool running = true;
u64 tick = 0;

// NOTE(vanya): Game functions
void P_ready();
void P_iterate();
void P_clear_backbuffer(u8 r, u8 g, u8 b);
void P_set_pixel(int x, int y, u8 r, u8 g, u8 b);
u32 P_pack_rgba(u8 r, u8 g, u8 b, u8 a);


// NOTE(vanya): Entrypoint
int main(int arg_count, char* args[])
{
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        LOG_CRITICAL("Failed to open display");
        return 1;
    }

    int default_screen_index = DefaultScreen(display);
    Window root = DefaultRootWindow(display);
    
    Window win = XCreateSimpleWindow(
        display,
        root,
        0, 0, // Position
        backbuffer_width, backbuffer_height, // Size
        1, // Border width
        BlackPixel(display, default_screen_index), // Border color
        WhitePixel(display, default_screen_index) // Fill color
    );
    
    XSelectInput(display, win, ExposureMask | KeyPressMask);
    XMapWindow(display, win);

    // Window title
    XStoreName(display, win, "025_pure");

    // Backbuffer image
    GC gc = XCreateGC(display, win, 0, NULL);

    XImage* img = XCreateImage(display, DefaultVisual(display, default_screen_index),
                               DefaultDepth(display, default_screen_index),
                               ZPixmap, 0, P_malloc(backbuffer_width * backbuffer_height * 4),
                               backbuffer_width, backbuffer_height, 32, 0);

    backbuffer = (u32*)img->data;

    // Get preffered refresh rate
    f64 refresh_rate = 60;

    XRRScreenResources* res = XRRGetScreenResources(display, root);
    if (res) {
        RROutput primary_output = XRRGetOutputPrimary(display, root); // Primary monitor
        XRROutputInfo* output = XRRGetOutputInfo(display, res, primary_output);
        
        if (output->connection == RR_Connected) {
            XRRCrtcInfo* crtc = XRRGetCrtcInfo(display, res, output->crtc);
            if (crtc && crtc->mode) {
                for (int i = 0; i < res->nmode; i++) {
                    XRRModeInfo mode = res->modes[i];
                    if (mode.id == crtc->mode) {
                        refresh_rate = (double)mode.dotClock / (mode.hTotal * mode.vTotal);
                    }
                }
            }
            if (crtc) XRRFreeCrtcInfo(crtc);
        }
    }

    useconds_t frame_delay_us_active = (useconds_t)(1e6 / refresh_rate);
    useconds_t frame_delay_us_inactive = (useconds_t)(1e6 / 10);

    P_ready();

    // Main loop
    XEvent ev;
    while (running) {
        // Process events
        XEvent ev;
        while (XPending(display)) {
            XNextEvent(display, &ev);

            switch (ev.type) {
            }
        }

        // Tick
        P_iterate();

        // Flush
        XPutImage(display, win, gc, img, 0, 0, 0, 0, backbuffer_width, backbuffer_height);
        XFlush(display);

        // Sleep
        Window focused_win;
        int revert;
        XGetInputFocus(display, &focused_win, &revert);

        usleep((focused_win == win) ? frame_delay_us_active : frame_delay_us_inactive);
    }

    // Let the OS cleanup
    return 0;
}


void P_clear_backbuffer(u8 r, u8 g, u8 b)
{
    // TODO(vanya): Make use of SIMD since it is 64 bit architecture
    
    u32 color = P_pack_rgba(r, g, b, 255);
    size_t count = backbuffer_width * backbuffer_height;

    for (size_t i = 0; i < count; i++) {
        backbuffer[i] = color;
    }
}

void P_set_pixel(int x, int y, u8 r, u8 g, u8 b)
{
    backbuffer[y * backbuffer_width + x] = P_pack_rgba(r, g, b, 255);
}

inline u32 P_pack_rgba(u8 r, u8 g, u8 b, u8 a)
{
    return (u32)((b) | (g << 8) | (r << 16) | (a << 24) | 0xFF000000);
}
