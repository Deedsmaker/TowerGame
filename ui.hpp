#pragma once

#define MAX_BUTTONS 32
#define MAX_UI_IMAGES 32
#define MAX_UI_TEXTS 64

struct Button{
    Vector2 position;
    Vector2 size;
    Vector2 pivot;
    char text[30];
    char tag[32];
    f32 font_size = 22;
    
    Color color = BLACK * 0.8f;    
    Color text_color = WHITE * 0.9f;
};

struct Ui_Image{
    Vector2 position;
    Vector2 size;
    Vector2 pivot;
    char tag[32];

    Color color;
};

struct Ui_Text{
    Vector2 position;
    f32 font_size = 22;
    
    char content[128];
    char tag[32];

    Color color;
};

struct Ui_Context{
    Array<Button, MAX_BUTTONS>     buttons   = Array<Button, MAX_BUTTONS>();
    Array<Ui_Image, MAX_UI_IMAGES> ui_images = Array<Ui_Image, MAX_UI_IMAGES>();
    Array<Ui_Text, MAX_UI_TEXTS>   ui_texts  = Array<Ui_Text, MAX_UI_TEXTS>();
};

global_variable Ui_Context ui_context = {};

//all in screen space

b32 make_button(Vector2 position, Vector2 size, Vector2 pivot, const char *text, f32 font_size, const char *tag){
    Button new_button = {position, size, pivot};
    str_copy(new_button.text, text);
    str_copy(new_button.tag, tag);
    
    new_button.font_size = font_size;
    ui_context.buttons.add(new_button);
    
    Rectangle button_rec = {position.x - size.x * (pivot.x), position.y - size.y * pivot.y, size.x, size.y};
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), button_rec)){
        return true;
    }
    
    return false;
}

void make_ui_image(Vector2 position, Vector2 size, Vector2 pivot, Color color, const char *tag){
    Ui_Image new_ui_image = {position, size, pivot};
    new_ui_image.color = color;
    str_copy(new_ui_image.tag, tag);
    
    ui_context.ui_images.add(new_ui_image);
}

void make_ui_text(const char *content, Vector2 position, f32 font_size, Color color, const char *tag){
    Ui_Text new_ui_text = {position, font_size};
    new_ui_text.color = color;
    str_copy(new_ui_text.tag, tag);
    str_copy(new_ui_text.content, content);
    
    ui_context.ui_texts.add(new_ui_text);
}
