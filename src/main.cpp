#define _CRT_SECURE_NO_DEPRECATE

#ifndef UNICODE
    #define UNICODE
#endif

#include <stdio.h>
#include <stdlib.h> 
//#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "my_defines.hpp"
#include "my_math.cpp"

#include "allocator.h"

#include "string.cpp"
#include "array.cpp"

#include "allocator.cpp"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "time.cpp"
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

void make_thing() {
    const char *letters = "QWERTYUIOPASDFGHJKLZXCVBNM";

    int size = 0;
    int *codepoints = LoadCodepoints(letters, &size);
    printf("%d\n", codepoints[0]);
    f32 font_size = 224;
    Font font = LoadFontEx("resources/MonospaceBold.ttf", font_size, codepoints, size);
    
    char str[256] = {0};
    
    Array <Image> backgrounds = {0};
    backgrounds.append(LoadImage("temp/blue.png"));
    backgrounds.append(LoadImage("temp/purple.png"));
    backgrounds.append(LoadImage("temp/white.png"));
    backgrounds.append(LoadImage("temp/pink.png"));
    backgrounds.append(LoadImage("temp/violet.png"));
    
    Array <String> icons = get_files_in_directory(tstring("temp/icons"));
    
    for_array (b, &backgrounds) {
        Image background = backgrounds.get_value(b);
        b32 dark_background = b < 2;
        Color color = dark_background ? WHITE : BLACK;
        Color alt_color = dark_background ? BLACK : PURPLE;
    
        for (i32 i = 0; i < size; i++) {
            sprintf(str, "%c", letters[i]);
            printf(str);
            // Image image = GenImageColor(256, 256, RED);
            Image image = ImageCopy(background);
            
            
            Rectangle rec = {256 - font_size, 256 - font_size, font_size - (256-font_size), font_size - (256-font_size)};
            ImageDrawRectangleLines(&image, rec, 5, color);
            
            Rectangle glyphRec = font.recs[i];
            GlyphInfo glyph = font.glyphs[i];
            
            
            Vector2 center = {128, 128};
            
            Vector2 position = {center.x - (glyphRec.width * 0.5f) - glyph.offsetX, 0};
            
            if (str[0] != 'Q') {
                position.y += 10;
            } else {
                position.y -= 5;
            }
            
            if (dark_background) ImageDrawTextEx(&image, font, str, position + Vector2_right * 5 + Vector2_up * 5, font_size, 0, alt_color);
            else                 ImageDrawTextEx(&image, font, str, position + Vector2_right * 4 + Vector2_up * 4, font_size, 0, alt_color);
            
            
            ImageDrawTextEx(&image, font, str, position, font_size, 0, color);
            
            ExportImage(image, tprintf("temp/result/Achievement_%d_%s.jpg", b, str));
            
            ImageColorGrayscale(&image);
            ImageColorBrightness(&image, -70);
            ExportImage(image, tprintf("temp/result/Achievement_%d_%s_gray.jpg", b, str));
        }
        
        for_array (i, &icons) {
            Image icon = LoadImage(c_str(icons.get_value(i)));
            Image image = ImageCopy(background);
            
            Rectangle rec = {256 - font_size, 256 - font_size, font_size - (256-font_size), font_size - (256-font_size)};
            ImageDrawRectangleLines(&image, rec, 5, color);
            
            Rectangle src_rec = {0, 0, (f32)icon.width, (f32)icon.height};
            Rectangle dst_rec = {0, 0, 256.0f, 256.0f};
            
            String image_name = strip_path_to_just_name(icons.get_value(i), temp);
            image_name = remove_extension(image_name, temp);
            if (string_contains(image_name, tstring("infinity"))) {
                dst_rec.width *= 1.2f;
                dst_rec.height *= 0.8f;
                dst_rec.x -= 128 * 0.2f;
                dst_rec.y += 128 * 0.2f;
            }
            
            ImageDraw(&image, icon, src_rec, dst_rec, WHITE);
            
            
            ExportImage(image, tprintf("temp/result/Achievement_icon_%d_%s.jpg", b, c_str(image_name)));
            
            ImageColorGrayscale(&image);
            ImageColorBrightness(&image, -70);
            ExportImage(image, tprintf("temp/result/Achievement_icon_%d_%s_gray.jpg", b, c_str(image_name)));
        }
        
        Image image = ImageCopy(background);
    }
}

int main(){
    *temp = init_allocator(Megabytes(16), ARENA_ALLOCATOR);

    // make_thing();
    
    // return 0;

    // main_font = LoadFont("resources/MonospaceBold.ttf");

    SetTraceLogLevel(LOG_WARNING_RAYLIB);

    InitWindow(screen_width, screen_height, "(Not that) Pure (of a) Action");
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
        
        if (!IsWindowFocused() && editor_state == EDITOR){
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
    
    unload_log_file();
    
    return 0;
}
