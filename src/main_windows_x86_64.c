#include "main.c"

#include <windows.h>
#include <stdint.h>

static BITMAPINFO bmi;
static HWND hwnd;
static HDC hdc;
static LARGE_INTEGER perf_freq;
static uint64_t refresh_rate = 60;

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
    wc.lpszClassName = "025_rpg";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0, wc.lpszClassName, "025_rpg",
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

    uint32_t frame_active_us = (uint32_t)(1000000.0 / refresh_rate);
    uint32_t frame_inactive_us = (uint32_t)(1000000.0 / 30.0);

    ready();

    MSG msg;
    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        iterate();

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

void set_pixel(int x, int y, u8 r, u8 g, u8 b)
{
    backbuffer[y * backbuffer_width + x] = (u32)(b | (g << 8) | (r << 16) | 0xFF000000);
}

