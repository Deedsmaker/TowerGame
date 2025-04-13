#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// #define UNICODE

#include <stdio.h>
#include <stdlib.h> 
//#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "../my_libs/my_defines.hpp"
#include "../my_libs/my_math.cpp"
#include "../my_libs/string.hpp"
#include "../my_libs/array.hpp"
#include "../my_libs/files.hpp"

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

// #include "include\raylib.h"
// #include "include\raymath.h"
// #include "include\rlgl.h"


// #include "structs.hpp"

// global_variable Core core = {};

// #include "utils.cpp"
// #include "render.cpp"


u32 render_width = 1600;
u32 render_height = 900;
f32 aspect_ratio = 1.0f;

#define SCREEN_WORLD_SIZE 150.0f

// #define UNIT_SIZE (render_width / 150.0f)

//reference 1920x1080 (1500)
#define UI_SCALING ((render_width * 0.5f + render_height * 0.5f) * 0.00066666f) // like / 1500.0f

b32 screen_size_changed = 0;
b32 bordless_fullscreen = false;

b32 window_minimized = false;
b32 should_close = false;

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            should_close = true;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND create_window(HINSTANCE hInstance, int width, int height) {
    const char *class_name = "Directx11WindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = window_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;
    RegisterClass(&wc);

    return CreateWindowEx(0, class_name, "Directx11 Window",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        width, height, NULL, NULL, hInstance, NULL);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    HWND hwnd = create_window(hInstance, render_width, render_height);
    ShowWindow(hwnd, nShowCmd);
    
    while (1){
        if (should_close){
            break;
        }
    
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
    }

    return 0;
}