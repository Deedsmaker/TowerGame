#pragma once

#define MAX_BUTTONS 128
#define MAX_UI_IMAGES 128
#define MAX_UI_TEXTS 128
#define MAX_UI_ELEMENTS 256

#define UI_FLAGS u32

enum Ui_Flags{
    BUTTON    = 1 << 1,
    UI_TEXT   = 1 << 2,
    UI_IMAGE  = 1 << 3,
    UI_TOGGLE = 1 << 4,
    UI_COLOR_PICKER = 1 << 5
};

struct Button{

};

struct Ui_Image{
};

struct Ui_Text{
    f32 font_size = 22;
    
    Color text_color = BLACK;
    char content[1024];
};

struct Ui_Element{
    Vector2 position;
    Vector2 size;
    Vector2 pivot;

    UI_FLAGS ui_flags;

    b32 has_texture = false;
    Texture texture = {};

    char tag[64];
    Color color = PINK;
    
    Button button;
    Ui_Image ui_image;
    Ui_Text text;
    b32 toggle_value = false;
};

struct Ui_Context{
    // Array<Button, MAX_BUTTONS>     buttons   = Array<Button, MAX_BUTTONS>();
    // Array<Ui_Image, MAX_UI_IMAGES> ui_images = Array<Ui_Image, MAX_UI_IMAGES>();
    // Array<Ui_Text, MAX_UI_TEXTS>   ui_texts  = Array<Ui_Text, MAX_UI_TEXTS>();
    Array<Ui_Element, MAX_UI_ELEMENTS> elements = Array<Ui_Element, MAX_UI_ELEMENTS>();
};

global_variable Ui_Context ui_context = {};

//all in screen space

static Ui_Element *init_ui_element(Vector2 position, Vector2 size, Vector2 pivot, Color color, const char *tag, UI_FLAGS ui_flags){
    // size *= UI_SCALING;
    Ui_Element new_ui_element = {position, size, pivot};
    new_ui_element.ui_flags = ui_flags;
    str_copy(new_ui_element.tag, tag);
    
    new_ui_element.color = color;
    
    ui_context.elements.add(new_ui_element);
    return ui_context.elements.last_ptr();
}

static void init_ui_text(Ui_Text *ui_text, const char *content, f32 font_size, Color text_color){
    str_copy(ui_text->content, content);
    
    font_size *= UI_SCALING;
    
    ui_text->text_color = text_color;
    ui_text->font_size = font_size;
}

b32 make_button(Vector2 position, Vector2 size, Vector2 pivot, const char *text, f32 font_size, const char *tag, Color button_color = BLACK * 0.9f, Color text_color = WHITE * 0.9f, UI_FLAGS additional_flags = 0, b32 toggle_value = false){
    Ui_Element *new_ui_element = init_ui_element(position, size, pivot, button_color, tag, BUTTON | UI_TEXT | additional_flags);
    init_ui_text(&new_ui_element->text, text, font_size, text_color);
    new_ui_element->toggle_value = toggle_value;
    
    Rectangle button_rec = {position.x - size.x * (pivot.x), position.y - size.y * pivot.y, size.x, size.y};
    
    if (CheckCollisionPointRec(input.screen_mouse_position, button_rec)){
        new_ui_element->color = ColorBrightness(new_ui_element->color, 0.5f);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            clicked_ui = true;
            return true;
        }
    }
    
    return false;
}
b32 make_button(Vector2 position, Vector2 size, const char *text, const char *tag, Color button_color = BLACK * 0.9f, Color text_color = WHITE * 0.9f, UI_FLAGS additional_flags = 0){
    return make_button(position, size, {0, 0}, text, 18, tag, button_color, text_color); 
}

void make_ui_image(Vector2 position, Vector2 size, Vector2 pivot, Color color, const char *tag){
    Ui_Element *new_ui_element = init_ui_element(position, size, pivot, color, tag, UI_IMAGE);
    
    Rectangle image_rec = {position.x - size.x * (pivot.x), position.y - size.y * pivot.y, size.x, size.y};
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(input.screen_mouse_position, image_rec)){
        clicked_ui = true;
    }
}

void make_ui_image(Texture texture, Vector2 position, Vector2 size, Vector2 pivot, Color color, const char *tag){
    Ui_Element *new_ui_element = init_ui_element(position, size, pivot, color, tag, UI_IMAGE);
    
    new_ui_element->has_texture = true;
    new_ui_element->texture = texture;
    
    Rectangle image_rec = {position.x - size.x * (pivot.x), position.y - size.y * pivot.y, size.x, size.y};
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(input.screen_mouse_position, image_rec)){
        clicked_ui = true;
    }
}

void make_ui_text(const char *content, Vector2 position, f32 font_size, Color color, const char *tag){
    Ui_Element *new_ui_element = init_ui_element(position, {100, 25}, {0, 0}, color, tag, UI_TEXT);
    init_ui_text(&new_ui_element->text, content, font_size, color);
}

void make_ui_text(const char *content, Vector2 position, const char *tag, f32 font_size = 16, Color color = WHITE){
    Ui_Element *new_ui_element = init_ui_element(position, {100, 25}, {0, 0}, color, tag, UI_TEXT);
    init_ui_text(&new_ui_element->text, content, font_size, color);
}

void make_ui_text(const char *content, Vector2 position, const char *tag, Color color, f32 font_size = 16){
    Ui_Element *new_ui_element = init_ui_element(position, {100, 25}, {0, 0}, color, tag, UI_TEXT);
    init_ui_text(&new_ui_element->text, content, font_size, color);
}

b32 make_ui_toggle(Vector2 position, u64 current_value, const char *tag){
    //Ui_Element *new_ui_element = init_ui_element(position, {32, 32}, {0, 0}, Fade(BLACK, 0.9f), tag, UI_TOGGLE);
    return make_button(position + Vector2_up, {14, 14}, {0, 0}, "", 0, tag, BLACK, VIOLET, UI_TOGGLE, current_value != 0);
}

b32 make_ui_color_picker(Vector2 position, Color color, b32 current_value, const char *tag){
    return make_button(position + Vector2_up, {14, 14}, {0, 0}, "", 0, tag, color, VIOLET, UI_COLOR_PICKER, current_value);
}

b32 make_ui_toggle(Vector2 position, Entity* e, b32 (get_value)(Entity*), const char *tag){
    b32 current_value = get_value(e);
    //Ui_Element *new_ui_element = init_ui_element(position, {32, 32}, {0, 0}, Fade(BLACK, 0.9f), tag, UI_TOGGLE);
    return make_button(position, {14, 14}, {0, 0}, "", 0, tag, BLACK, VIOLET, UI_TOGGLE, current_value);
}

Vector2 panel_last_added_position = {0};
Vector2 panel_last_added_size = {0};
Ui_Element* current_panel = NULL;

void begin_panel(Vector2 position, Vector2 size, Color color, const char* tag){
    Ui_Element* new_panel = init_ui_element(position, size, {0, 0}, color, tag, UI_IMAGE);
    
    Rectangle image_rec = {position.x - size.x * (0), position.y - size.y * 0, size.x, size.y};
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(input.screen_mouse_position, image_rec)){
        clicked_ui = true;
    }
    
    current_panel = new_panel;
    panel_last_added_position = position + Vector2_up * 10 + Vector2_right * 5;
    panel_last_added_size = {0};
}

void end_panel(){
    current_panel = NULL;
}

b32 make_panel_button(const char* text, const char* tag){
    assert(current_panel);   
    
    Vector2 position = panel_last_added_position + Vector2_up * 5 + Vector2_up * panel_last_added_size.y;  
    Vector2 size = {150, 25};
    
    panel_last_added_position = position;
    panel_last_added_size = size;
    
    return make_button(position, size, {0, 0}, text, 20, tag, BLUE, WHITE);
}

void make_panel_text(const char* text, const char* tag){
    assert(current_panel);   
    
    Vector2 position = panel_last_added_position + Vector2_up * 5 + Vector2_up * panel_last_added_size.y;  
    Vector2 size = {100, 20};
    
    panel_last_added_position = position;
    panel_last_added_size = size;
    
    make_ui_text(text, position, tag);
}


