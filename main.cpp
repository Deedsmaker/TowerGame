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

#include "include\raylib.h"
#include "include\raymath.h"
#include "include\rlgl.h"

global_variable f32 game_time = 0;

#include "utils.cpp"
#include "render.cpp"

int screen_width = 1600;
int screen_height = 900;

#define UNIT_SIZE (screen_width / 150.0f)

b32 screen_size_changed = 0;

#include "game.cpp"
int main(){
    InitWindow(screen_width, screen_height, "aboba");

    SetExitKey(-1);
    
    screen_size_changed = true;
    
    init_game();
    while(!WindowShouldClose()){
        dt = GetFrameTime();
        
        if (IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT)){
            ToggleBorderlessWindowed();
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
