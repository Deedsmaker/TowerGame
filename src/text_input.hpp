#pragma once

#define MAX_INPUT_FIELDS 64
#define INPUT_FIELD_MAX_CHARS 1024

struct Input_Field{
    Vector2 position;
    Vector2 size;
    i32 index = -1;
    //Vector2 pivot;
    
    char content[INPUT_FIELD_MAX_CHARS];
    char tag[64];
    int chars_count = 0;
    
    Color color = GRAY;
    
    f32 font_size = 18;
    
    b32 in_focus = false;
    b32 changed = false;
};

global_variable Static_Array<Input_Field, MAX_INPUT_FIELDS> input_fields = Static_Array<Input_Field, MAX_INPUT_FIELDS>();
global_variable Input_Field focus_input_field;
global_variable b32 just_focused = false;

float backspace_press_time = 0;
float last_hold_deleted_time = 0;

void copy_input_field(Input_Field *dest, Input_Field *src);

void set_focus_input_field(const char *data){
    str_copy(focus_input_field.content, data);
    focus_input_field.chars_count = str_len(data);
}

void clear_focus_input_field(){
    focus_input_field.content[0] = '\0';
    focus_input_field.chars_count = 0;
}

b32 clicked_on_input_field = false;

void update_input_field(){
    focus_input_field.changed = false;

    if (focus_input_field.in_focus && IsKeyPressed(KEY_ESCAPE)){
        focus_input_field.in_focus = false;
    }

    if (IsKeyPressed(KEY_TAB) && !console.is_open && input_fields.count > 0){
        focus_input_field.in_focus = false;
        Input_Field *next;
        if (IsKeyDown(KEY_LEFT_SHIFT)){
            next = input_fields.get((focus_input_field.index - 1) < 0 ? input_fields.count - 1 : focus_input_field.index - 1);
        } else{
            next = input_fields.get((focus_input_field.index + 1) % input_fields.count);
        }
        next->in_focus = true;
        copy_input_field(&focus_input_field, next);
        just_focused = true;
    }
    
    if (focus_input_field.in_focus){
        //SetMouseCursor(MOUSE_CURSOR_IBEAM);

        int key = GetCharPressed();

        if (just_focused && IsKeyPressed(KEY_BACKSPACE)){
            just_focused = false;
            focus_input_field.chars_count = 0;
        }

        while (key > 0)
        {
            if ((key >= 32) && (key <= 125) && (focus_input_field.chars_count < INPUT_FIELD_MAX_CHARS))
            {
                just_focused = false;
                int content_len = str_len(focus_input_field.content);
                char char_key = (char)key;
                
                //NO SPACES OR SLASHES FIRST
                if ((char_key != ' ' && char_key != '/') || content_len > 0){
                    focus_input_field.content[focus_input_field.chars_count] = char_key;
                    focus_input_field.content[focus_input_field.chars_count+1] = '\0';
                    focus_input_field.chars_count++;
                    
                    focus_input_field.changed = true;
                }
            }

            key = GetCharPressed();
        }
        
        local_persist f32 hold_time = 0;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && clicked_on_input_field){
            hold_time += core.time.real_dt;
            b32 is_content_number = focus_input_field.chars_count >= 1 && is_digit_or_minus(focus_input_field.content[0]);
            if (focus_input_field.chars_count >= 2) is_content_number = is_content_number && (is_digit_or_minus(focus_input_field.content[1]) || focus_input_field.content[1] == '.');
            
            if (is_content_number && hold_time > 0.2f){
                f32 num = to_f32(focus_input_field.content);
                                
                num += (GetMouseDelta().x) * 0.01f;
                str_copy(focus_input_field.content, tprintf("%f", num));
                focus_input_field.changed = true;
            }
        } else{
            hold_time = 0;            
            clicked_on_input_field = false;
        }
        
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            if (IsKeyDown(KEY_LEFT_CONTROL)){
                focus_input_field.chars_count = 1;
            }
            focus_input_field.chars_count--;
            if (focus_input_field.chars_count < 0) focus_input_field.chars_count = 0;
            focus_input_field.content[focus_input_field.chars_count] = '\0';
            focus_input_field.changed = true;
            
            backspace_press_time = core.time.app_time;
        }
        
        if (IsKeyDown(KEY_BACKSPACE))
        {
            f32 time_since_backspace_pressed = core.time.app_time - backspace_press_time;
            f32 time_since_hold_deleted = core.time.app_time - last_hold_deleted_time;
        
            if (time_since_backspace_pressed >= 0.2f && time_since_hold_deleted >= 0.05f){
                focus_input_field.chars_count--;
                if (focus_input_field.chars_count < 0) focus_input_field.chars_count = 0;
                focus_input_field.content[focus_input_field.chars_count] = '\0';
                focus_input_field.changed = true;
                
                last_hold_deleted_time = core.time.app_time;
            }   
        }
    } else{
         //SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

// b32 is_input_field_exists(Input_Field input_field){
//     for (int i = 0; i < input_fields.count; i++){
//         if (input_field.id == input_fields.get(i).id){
//             return true;
//         }
//     }
    
//     return false;
// }

// void make_last_in_focus(){
//     input_fields.last_ptr()->in_focus = true;
//     focus_input_field = input_fields.last();
// }

global_variable char next_to_make_focused_tag[512] = "\0";
void make_next_input_field_in_focus(const char *tag){
    str_copy(next_to_make_focused_tag, tag);
}

void copy_input_field(Input_Field *dest, Input_Field *src){
    str_copy(dest->content, src->content);
    str_copy(dest->tag,     src->tag);
    dest->chars_count = src->chars_count;
    dest->in_focus = src->in_focus;
    dest->index = src->index;
}

b32 make_input_field(const char *content, Vector2 position, Vector2 size, const char *tag, Color color = GRAY, bool remove_focus_after_enter = true){
    //means it's the same and we want to keep all contents
    Input_Field input_field = {position, size};
    if (size.y < 50){
        input_field.size.y = input_field.font_size;
    }
    input_field.color = color;
    input_field.index = input_fields.count;
    
    Rectangle field_rect = {input_field.position.x, input_field.position.y, input_field.size.x, input_field.size.y};
    b32 hovered_over = CheckCollisionPointRec(input.screen_mouse_position, field_rect);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hovered_over){
        clicked_on_input_field = true;
    }
    
    if (focus_input_field.in_focus && str_equal(focus_input_field.tag, tag)){
        input_field.in_focus = true;
        input_field.index = input_fields.count;
        copy_input_field(&input_field, &focus_input_field);
        
        input_fields.append(input_field);
        
        if (IsKeyPressed(KEY_ENTER)){
            if (remove_focus_after_enter){
                focus_input_field.in_focus = false;
            }
            
            return true;
        }
    
        return focus_input_field.changed && remove_focus_after_enter;
    } else{
        str_copy(input_field.content, content);
        input_field.chars_count = str_len(content);
        str_copy(input_field.tag, tag);
    }
    
    
    if (hovered_over){
        input_field.color = ColorBrightness(input_field.color, 0.2f);
    }
    
    
    if (next_to_make_focused_tag[0] && str_contains(tag, next_to_make_focused_tag) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hovered_over){
        input_field.in_focus = true;
        //str_copy(input_field.content, "");
        next_to_make_focused_tag[0] = '\0';
        copy_input_field(&focus_input_field, &input_field);
        just_focused = true;
        clicked_ui = true;
    }
    
    input_fields.append(input_field);
    
    return false;
}

b32 make_input_field(const char *content, Vector2 position, f32 width, const char *tag, Color color = GRAY, bool remove_focus_after_enter = true){
    return make_input_field(content, position, {width, 18}, tag, color, remove_focus_after_enter);
}
