#pragma once

#define MAX_BUTTONS 32

struct Button{
    Vector2 position;
    Vector2 size;
    Vector2 pivot;
    char text[30];
    f32 font_size = 20;
    
    Color color = WHITE;    
};

struct Ui_Context{
    Array<Button, MAX_BUTTONS> buttons = Array<Button, MAX_BUTTONS>();
};

global_variable Ui_Context ui_context = {};

//all in screen space

b32 make_button(Vector2 position, Vector2 size, Vector2 pivot, const char *text, f32 font_size){
    Button new_button = {position, size, pivot};
    str_copy(new_button.text, text);
    new_button.font_size = font_size;
    ui_context.buttons.add(new_button);
    
    Rectangle button_rec = {position.x - size.x * (pivot.x), position.y - size.y * pivot.y, size.x, size.y};
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), button_rec)){
        return true;
    }
    
    return false;
}
