#include "stl_windows_x86_64.c"
#include <windows.h>


BITMAPINFO bmi;
HWND hwnd;
HDC hdc;
u64 refresh_rate = 60;

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


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY: {
        running = 0;
        PostQuitMessage(0);
        return 0;
    } break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prev, LPSTR cmd, int show)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "025_pure";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0, wc.lpszClassName, "025_pure",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        backbuffer_width, backbuffer_height,
        0, 0, hInst, 0);

    ShowWindow(hwnd, SW_SHOW);

    // Backbuffer
    backbuffer = malloc(backbuffer_width * backbuffer_height * 4);

    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = backbuffer_width;
    bmi.bmiHeader.biHeight = -backbuffer_height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hdc = GetDC(hwnd);

    // Refresh rate
    DEVMODE dm = {0};
    dm.dmSize = sizeof(dm);
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) && dm.dmDisplayFrequency > 10) {
        refresh_rate = dm.dmDisplayFrequency;
    }

    u32 frame_active_us = (u32)(1000000.0 / refresh_rate);
    u32 frame_inactive_us = (u32)(1000000.0 / 10.0);

    P_ready();

    MSG msg;
    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        P_iterate();

        StretchDIBits(
            hdc,
            0, 0, backbuffer_width, backbuffer_height,
            0, 0, backbuffer_width, backbuffer_height,
            backbuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);

        HWND fg = GetForegroundWindow();
        int active = (fg == hwnd);

        Sleep(active ? frame_active_us/1000 : frame_inactive_us/1000);
    }

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

// NOTE(vanya): Payload - the game
#include "main.c"
