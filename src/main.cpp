#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h> 
//#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "my_defines.hpp"
#include "Allocator.cpp"

#include "my_math.cpp"
#include "string.cpp"
#include "array.cpp"
#include "old_files.hpp"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "files.cpp"
// #include <windows.h>

// #include "include/Vulkan/vulkan/vulkan.h"

#include "structs.hpp"

global_variable Core core = {};

#include "utils.cpp"
#include "render.cpp"

int screen_width = 1600;
int screen_height = 900;
f32 aspect_ratio = 1.0f;

#define SCREEN_WORLD_SIZE 150.0f

// #define UNIT_SIZE (screen_width / 150.0f)

//reference 1920x1080 (1500)
#define UI_SCALING ((screen_width * 0.5f + screen_height * 0.5f) * 0.00066666f) // like / 1500.0f

b32 screen_size_changed = 0;
b32 bordless_fullscreen = false;

b32 window_minimized = false;

#include "game.cpp"

// void make_thing() {
//     Image image = GenImageColor(256, 256, RED);
    
//     ImageDrawText(&image, "Y", 64, 32, 224, WHITE);

//     // RLAPI void ImageDrawText(Image *dst, const char *text, int posX, int posY, int fontSize, Color color);   // Draw text (using default font) within an image (destination)
//     ExportImage(image, "test.png");
    
// }

int main(){
    // make_thing();

    SetTraceLogLevel(LOG_WARNING);

    InitWindow(screen_width, screen_height, "Pure Action");
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN);
    InitAudioDevice();
    SetMasterVolume(0.3f);
    //EnableEventWaiting();

    SetExitKey(-1);
    
    screen_size_changed = true;
    
    init_game();
    while(!WindowShouldClose()){
        if (IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT)){
            ToggleBorderlessWindowed();
            bordless_fullscreen = !bordless_fullscreen;
            // SetWindowPosition(0,0);
            // SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
        }
        
        if (!IsWindowFocused() && !IsWindowMinimized() && bordless_fullscreen){
            MinimizeWindow();
            window_minimized = true;
        }
        
        if (!IsWindowMinimized() && IsWindowFocused()){
            window_minimized = false;
        }
        
        if (!IsWindowFocused() && game_state == EDITOR){
            WaitTime(1);
        }
        
        if (IsWindowResized()){
            screen_size_changed = true;
        }
        
        if (screen_size_changed){
            screen_width = GetScreenWidth();
            screen_height = GetScreenHeight();
        }
        
        update_game();
        
        screen_size_changed = 0;
    }
    
    CloseWindow();
    return 0;
}
