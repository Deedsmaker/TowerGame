#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdint.h>
#include <assert.h>

typedef float  f32;
typedef double f64;


//#define _CRT_SECURE_NO_WARNINGS

#define global_variable static
#define local_persist   static
#define internal        static

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u32 b32;

#include "my_math.cpp"

#include "include\raylib.h"
#include "include\raymath.h"
#include "include\rlgl.h"

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
    
    init_game();
    while(!WindowShouldClose()){
        dt = GetFrameTime();
        
        update_game();
        screen_size_changed = 0;
    }
    
    CloseWindow();
    return 0;
}
