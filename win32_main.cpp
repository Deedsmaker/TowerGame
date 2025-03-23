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

// #include "include\raylib.h"
// #include "include\raymath.h"
// #include "include\rlgl.h"

#include "include/Vulkan/vulkan/vulkan.h"

// #include "structs.hpp"

// global_variable Core core = {};

// #include "utils.cpp"
// #include "render.cpp"


int render_width = 1600;
int render_height = 900;
f32 aspect_ratio = 1.0f;

#define SCREEN_WORLD_SIZE 150.0f

// #define UNIT_SIZE (render_width / 150.0f)

//reference 1920x1080 (1500)
#define UI_SCALING ((render_width * 0.5f + render_height * 0.5f) * 0.00066666f) // like / 1500.0f

b32 screen_size_changed = 0;
b32 bordless_fullscreen = false;

b32 window_minimized = false;

HINSTANCE hInstance;
HWND hWnd;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            // Handle painting here if needed
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateWindowInstance(HINSTANCE hInstance) {
    const char* CLASS_NAME = "VulkanWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    hWnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                    // Window class
        "Vulkan Window",               // Window text
        WS_OVERLAPPEDWINDOW,           // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,  // Size and position
        render_width, render_height,                      // Width and height
        NULL,                          // Parent window
        NULL,                          // Menu
        hInstance,                     // Instance handle
        NULL                           // Additional application data
    );

    if (hWnd == NULL) {
        MessageBox(NULL, "Failed to create window!", "Error", MB_OK);
        exit(1);
    }

    ShowWindow(hWnd, SW_SHOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    ::hInstance = hInstance;

    CreateWindowInstance(hInstance);

    // Initialize Vulkan here (not covered in this example)

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}