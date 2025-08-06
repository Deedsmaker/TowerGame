#pragma once

typedef enum {
    KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    // Alphanumeric keys
    KEY_APOSTROPHE      = 39,       // Key: '
    KEY_COMMA           = 44,       // Key: ,
    KEY_MINUS           = 45,       // Key: -
    KEY_PERIOD          = 46,       // Key: .
    KEY_SLASH           = 47,       // Key: /
    KEY_ZERO            = 48,       // Key: 0
    KEY_ONE             = 49,       // Key: 1
    KEY_TWO             = 50,       // Key: 2
    KEY_THREE           = 51,       // Key: 3
    KEY_FOUR            = 52,       // Key: 4
    KEY_FIVE            = 53,       // Key: 5
    KEY_SIX             = 54,       // Key: 6
    KEY_SEVEN           = 55,       // Key: 7
    KEY_EIGHT           = 56,       // Key: 8
    KEY_NINE            = 57,       // Key: 9
    KEY_SEMICOLON       = 59,       // Key: ;
    KEY_EQUAL           = 61,       // Key: =
    KEY_A               = 65,       // Key: A | a
    KEY_B               = 66,       // Key: B | b
    KEY_C               = 67,       // Key: C | c
    KEY_D               = 68,       // Key: D | d
    KEY_E               = 69,       // Key: E | e
    KEY_F               = 70,       // Key: F | f
    KEY_G               = 71,       // Key: G | g
    KEY_H               = 72,       // Key: H | h
    KEY_I               = 73,       // Key: I | i
    KEY_J               = 74,       // Key: J | j
    KEY_K               = 75,       // Key: K | k
    KEY_L               = 76,       // Key: L | l
    KEY_M               = 77,       // Key: M | m
    KEY_N               = 78,       // Key: N | n
    KEY_O               = 79,       // Key: O | o
    KEY_P               = 80,       // Key: P | p
    KEY_Q               = 81,       // Key: Q | q
    KEY_R               = 82,       // Key: R | r
    KEY_S               = 83,       // Key: S | s
    KEY_T               = 84,       // Key: T | t
    KEY_U               = 85,       // Key: U | u
    KEY_V               = 86,       // Key: V | v
    KEY_W               = 87,       // Key: W | w
    KEY_X               = 88,       // Key: X | x
    KEY_Y               = 89,       // Key: Y | y
    KEY_Z               = 90,       // Key: Z | z
    KEY_LEFT_BRACKET    = 91,       // Key: [
    KEY_BACKSLASH       = 92,       // Key: '\'
    KEY_RIGHT_BRACKET   = 93,       // Key: ]
    KEY_GRAVE           = 96,       // Key: `
    // Function keys
    KEY_SPACE           = 32,       // Key: Space
    KEY_ESCAPE          = 256,      // Key: Esc
    KEY_ENTER           = 257,      // Key: Enter
    KEY_TAB             = 258,      // Key: Tab
    KEY_BACKSPACE       = 259,      // Key: Backspace
    KEY_INSERT          = 260,      // Key: Ins
    KEY_DELETE          = 261,      // Key: Del
    KEY_RIGHT           = 262,      // Key: Cursor right
    KEY_LEFT            = 263,      // Key: Cursor left
    KEY_DOWN            = 264,      // Key: Cursor down
    KEY_UP              = 265,      // Key: Cursor up
    KEY_PAGE_UP         = 266,      // Key: Page up
    KEY_PAGE_DOWN       = 267,      // Key: Page down
    KEY_HOME            = 268,      // Key: Home
    KEY_END             = 269,      // Key: End
    KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    KEY_NUM_LOCK        = 282,      // Key: Num lock
    KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    KEY_PAUSE           = 284,      // Key: Pause
    KEY_F1              = 290,      // Key: F1
    KEY_F2              = 291,      // Key: F2
    KEY_F3              = 292,      // Key: F3
    KEY_F4              = 293,      // Key: F4
    KEY_F5              = 294,      // Key: F5
    KEY_F6              = 295,      // Key: F6
    KEY_F7              = 296,      // Key: F7
    KEY_F8              = 297,      // Key: F8
    KEY_F9              = 298,      // Key: F9
    KEY_F10             = 299,      // Key: F10
    KEY_F11             = 300,      // Key: F11
    KEY_F12             = 301,      // Key: F12
    KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    KEY_LEFT_CONTROL    = 341,      // Key: Control left
    KEY_LEFT_ALT        = 342,      // Key: Alt left
    KEY_LEFT_SUPER      = 343,      // Key: Super left
    KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    KEY_RIGHT_ALT       = 346,      // Key: Alt right
    KEY_RIGHT_SUPER     = 347,      // Key: Super right
    KEY_KB_MENU         = 348,      // Key: KB menu
    // Keypad keys
    KEY_KP_0            = 320,      // Key: Keypad 0
    KEY_KP_1            = 321,      // Key: Keypad 1
    KEY_KP_2            = 322,      // Key: Keypad 2
    KEY_KP_3            = 323,      // Key: Keypad 3
    KEY_KP_4            = 324,      // Key: Keypad 4
    KEY_KP_5            = 325,      // Key: Keypad 5
    KEY_KP_6            = 326,      // Key: Keypad 6
    KEY_KP_7            = 327,      // Key: Keypad 7
    KEY_KP_8            = 328,      // Key: Keypad 8
    KEY_KP_9            = 329,      // Key: Keypad 9
    KEY_KP_DECIMAL      = 330,      // Key: Keypad .
    KEY_KP_DIVIDE       = 331,      // Key: Keypad /
    KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
    KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
    KEY_KP_ADD          = 334,      // Key: Keypad +
    KEY_KP_ENTER        = 335,      // Key: Keypad Enter
    KEY_KP_EQUAL        = 336,      // Key: Keypad =
    // Android key buttons
    KEY_BACK            = 4,        // Key: Android back button
    KEY_MENU            = 82,       // Key: Android menu button
    KEY_VOLUME_UP       = 24,       // Key: Android volume up button
    KEY_VOLUME_DOWN     = 25        // Key: Android volume down button
} KeyboardKey;

// Add backwards compatibility support for deprecated names
#define MOUSE_LEFT_BUTTON   MOUSE_BUTTON_LEFT
#define MOUSE_RIGHT_BUTTON  MOUSE_BUTTON_RIGHT
#define MOUSE_MIDDLE_BUTTON MOUSE_BUTTON_MIDDLE

// Mouse buttons
typedef enum {
    MOUSE_BUTTON_LEFT    = 0,       // Mouse button left
    MOUSE_BUTTON_RIGHT   = 1,       // Mouse button right
    MOUSE_BUTTON_MIDDLE  = 2,       // Mouse button middle (pressed wheel)
    MOUSE_BUTTON_SIDE    = 3,       // Mouse button side (advanced mouse device)
    MOUSE_BUTTON_EXTRA   = 4,       // Mouse button extra (advanced mouse device)
    MOUSE_BUTTON_FORWARD = 5,       // Mouse button forward (advanced mouse device)
    MOUSE_BUTTON_BACK    = 6,       // Mouse button back (advanced mouse device)
} MouseButton;

void print(Vector2 vec){
    printf("{%f, %f}; %.4f\n", vec.x, vec.y, core.time.app_time);
}

void print(f32 num){
    printf("%f; %.4f\n", num, core.time.app_time);
}

void print(i32 num){
    printf("%d; %.4f\n", num, core.time.app_time);
}

void print(const char *str){
    printf("%s; %.4f\n", str, core.time.app_time);
}

// void print(b32 num){
//     if (num){
//         printf("true; %.4f\n", core.time.app_time);
//     } else{
//         printf("false; %.4f\n", core.time.app_time);
//     }
// }

void print(Array<Vector2> *arr){
    printf("[");
    for (int i = 0; i < arr->count; i++){
        printf("{%.4f, %.4f}", arr->get_value(i).x, arr->get_value(i).y);
        
        if (i < arr->count - 1){
            printf(",");
        }
    }
    
    printf("]; %.4f\n", core.time.app_time);
}

char* to_string(int num){
    char* text = (char*)malloc(10 * sizeof(char));
    sprintf(text, "%d", num);
    return text;
}

char* to_string(f32 num){
    char* text = (char*)malloc(10 * sizeof(char));
    sprintf(text, "%f", num);
    return text;
}
char* to_string(f64 num){
    char* text = (char*)malloc(30 * sizeof(char));
    sprintf(text, "%f", num);
    return text;
}

const char* to_string(Color color){
    return TextFormat("{%d, %d, %d, %d}", color.r, color.g, color.b, color.a);
}

// struct Old_Arr{  
//     u8 *data;
//     size_t size;
//     int count;
//     int capacity;
// };

// Old_Arr array_init(size_t size, int count = 10){
//     Old_Arr array = {};
//     array.size = size;
//     array.data = (u8 *)malloc(count * size);
//     array.capacity = count;
    
//     return array;
// }

// void array_free(Old_Arr *array){
//     free(array->data);
// }

// u8 *array_get_pointer(Old_Arr *array, int index){
//     return (array->data) + index * array->size;
// }

// void array_append(Old_Arr *array, void *value){
//     if (array->count >= array->max_count) return;
//     u8* element = array_get_pointer(array, array->count);
//     memmove(element, value, array->size);
//     array->count++;
// }

// void array_remove(Old_Arr *array, int index){
//     for (int i = index; i < array->count - 1; i++){
//         u8 *current_element = array_get_pointer(array, i);
//         u8 *next_element = array_get_pointer(array, i + 1);
//         memmove(current_element, next_element, array->size);
//     }
    
//     array->count--;
// }

void zero_array(char *arr, int count){
    for (int i = 0; i < count; i++){
        arr[i] = NULL;
    }
}
    
b32 is_digit_or_minus(char ch){
    return ch == '-' || is_digit(ch);
}

void fill_i32_from_string(i32 *int_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *int_ptr = to_i32(str_data);
}    

void fill_i32_from_string(u64 *int_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *int_ptr = to_i32(str_data);
}    

void fill_u64_from_string(u64 *int_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *int_ptr = to_u64(str_data);
}

void fill_b32_from_string(b32 *b32_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *b32_ptr = to_i32(str_data);
}    

void fill_f32_from_string(f32 *f32_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *f32_ptr = to_f32(str_data);
}    

void fill_vector2_from_string(Vector2 *vec_ptr, char *x_str, char *y_str){
    assert(is_digit_or_minus(*x_str));
    assert(is_digit_or_minus(*y_str));
    
    vec_ptr->x = to_f32(x_str);
    vec_ptr->y = to_f32(y_str);
}

void fill_vector4_from_string(Color *vec_ptr, char *x_str, char *y_str, char *z_str, char *w_str){
    assert(is_digit_or_minus(*x_str));
    assert(is_digit_or_minus(*y_str));
    assert(is_digit_or_minus(*z_str));
    assert(is_digit_or_minus(*w_str));
    
    vec_ptr->r = to_f32(x_str);
    vec_ptr->g = to_f32(y_str);
    vec_ptr->b = to_f32(z_str);
    vec_ptr->a = to_f32(w_str);
}

void fill_vertices_array_from_string(Static_Array<Vector2, MAX_VERTICES> *vertices, Array<String> line_arr, i32 *index_ptr){
    assert(line_arr.get_value(*index_ptr + 1).data[0] == '[');
    assert(is_digit_or_minus(line_arr.get_value(*index_ptr + 2).data[0]));
    
    *index_ptr += 2;
    
    for (; *index_ptr < line_arr.count - 1 && line_arr.get_value(*index_ptr).data[0] != ']'; *index_ptr += 2){
        String current = line_arr.get_value((*index_ptr));
        String next    = line_arr.get_value((*index_ptr) + 1);
        
        fill_vector2_from_string(vertices->get(vertices->count), current.data, next.data);
        vertices->count++;
    }
}

void fill_vector2_array_from_string(Array<Vector2> *points, Array<String> line_arr, i32 *index_ptr){
    assert(line_arr.get_value(*index_ptr + 1).data[0] == '[');
    assert(is_digit_or_minus(line_arr.get_value(*index_ptr + 2).data[0]));
    
    *index_ptr += 2;
    
    for (; *index_ptr < line_arr.count - 1 && line_arr.get_value(*index_ptr).data[0] != ']'; *index_ptr += 2){
        String current = line_arr.get_value((*index_ptr));
        String next    = line_arr.get_value((*index_ptr) + 1);
        
        points->append({});
        fill_vector2_from_string(points->last(), current.data, next.data);
    }
}

void fill_int_array_from_string(Array<int> *arr, Array<String> line_arr, i32 *index_ptr){
    assert(line_arr.get_value(*index_ptr + 1).data[0] == '[');
    //assert(is_digit_or_minus(line_arr.get_pointer(*index_ptr + 2).data[0]));
    
    *index_ptr += 2;
    
    for (; *index_ptr < line_arr.count - 1 && line_arr.get_value(*index_ptr).data[0] != ']'; *index_ptr += 1){
        String current = line_arr.get_value((*index_ptr));
        //String next    = line_arr.get_pointer((*index_ptr) + 1);
        i32 value = -1;
        fill_i32_from_string(&value, current.data);  
        arr->append(value);
        //fill_vector2_from_string(arr->get_pointer(arr->count), current.data, next.data);
        //arr->count++;
    }
}


