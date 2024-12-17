#define _CRT_SECURE_NO_DEPRECATE

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

#include "include\raylib.h"
#include "include\raymath.h"
#include "include\rlgl.h"

#include "structs.hpp"

global_variable Core core = {};

Vector2 mouse_position = Vector2_zero;

#include "utils.cpp"
#include "render.cpp"

int screen_width = 1600;
int screen_height = 900;

#define UNIT_SIZE (screen_width / 150.0f)

b32 screen_size_changed = 0;
b32 bordless_fullscreen = false;

#include "game.cpp"
int main(){
    InitWindow(screen_width, screen_height, "Pure Action");
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN);
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
