#pragma once

#include <stdarg.h>

#define MEDIUM_STR_LEN 1024
#define MAX_TEMP_LINES 16

global_variable char temp_lines[MAX_TEMP_LINES][MEDIUM_STR_LEN];
global_variable i32 temp_line_index = 0;

void increment_temp_line_index(){
    temp_line_index = (temp_line_index + 1) % MAX_TEMP_LINES;   
}

size_t str_len(const char *line){
    size_t result = 0;   
    while (line && line[result]){
        result++;
    }
    
    return result;
}

void mem_copy_back(void *dest, void *source, size_t bytes){
    char *new_dest   = (char*)dest;
    char *new_source = (char*)source;
    for (int i = bytes - 1; i >= 0; --i){
        new_dest[i] = new_source[i];
        //printf("%s\n", new_dest[i]);
    }
}

void mem_copy(void *dest, void *source, size_t bytes){
    char *new_dest   = (char*)dest;
    char *new_source = (char*)source;
    for (int i = 0; i < bytes; i++){
        new_dest[i] = new_source[i];
        //printf("%s\n", new_dest[i]);
    }
}

void mem_set(void *dest, u8 value, size_t bytes){
    char *new_dest   = (char*)dest;
    for (int i = 0; i < bytes; i++){
        new_dest[i] = value;
    }
}

void str_copy(char *dest, const char *source){
    int len = str_len(source);
    mem_copy((void*)dest, (void*)source, len * sizeof(char));
    dest[len] = '\0';

    // size_t bytes = str_len(source);
    // for (size_t i = bytes; i > 0; --i){
    //     dest[i - 1] = source[i - 1];
    //     //printf("%s\n", new_dest[i]);
    // }
}

inline char to_lower(char ch){
    return ch | 0x20;
}

inline b32 is_digit(char ch){
    return ch >= 48 && ch <= 57;
}

b32 str_start_with_const(const char *str, const char *start_with){
    int len = str_len(str);
    int start_with_len = str_len(start_with);
    
    if (start_with_len > len){
        return false;
    }
    
    for (int i = 0; i < start_with_len; i++){
        if (to_lower(str[i]) != to_lower(start_with[i])){
            return false;
        }
    }
    
    return true;
}

b32 str_start_with(char *str, const char *start_with){
    return str_start_with_const(str, start_with);
}

b32 str_start_with(const char *str, char *start_with){
    return str_start_with_const(str, start_with);
}

b32 str_start_with(char *str, char *start_with){
    return str_start_with_const(str, start_with);
}

int str_find(const char *line, char symbol){
    size_t line_len = str_len(line);
    
    for (int i = 0; i < line_len; i++){
        if (line[i] == symbol){
            return i;
        }
    }
    
    return line_len;
}

i32 get_index_of_substring_start(const char *line, const char *substring){
    size_t line_length = str_len(line);
    size_t substring_length = str_len(substring);
    
    if (substring_length > line_length) return -1;
    
    i32 substring_start_index = -1;
    i32 found_substring_in_line_count = 0;
    
    for (i32 i = 0; i < line_length; i++) {
        if (substring_start_index == -1 && line[i] == substring[found_substring_in_line_count]) {
            substring_start_index = i;
            found_substring_in_line_count = 1;
            continue;
        }
        
        if (substring_start_index >= 0) {
            if (substring[found_substring_in_line_count] == line[i]) {
                found_substring_in_line_count += 1;
                
                if (found_substring_in_line_count >= substring_length) {
                    assert(found_substring_in_line_count == substring_length);
                    break;
                }
            } else {
                substring_start_index = -1;
                found_substring_in_line_count = 0;
            }
        }
    }
    
    if (found_substring_in_line_count >= substring_length) {
        assert(found_substring_in_line_count == substring_length);
        assert(substring_start_index > -1);        
        return substring_start_index;
    } else {
        return -1;
    }
}

void substring_after_line(char *line, const char *excluded){
    int start_index = 0;
    
    int line_len = str_len(line);
    int excluded_len = str_len(excluded);
    
    if (excluded_len > line_len){
        return;
    }
    
    mem_copy(line, line + excluded_len, (line_len - excluded_len) * sizeof(char));
    line[line_len - excluded_len] = '\0';
}
void substring_before_line(char *line, const char *excluded){
    i32 i = get_index_of_substring_start(line, excluded);
    
    if (i > -1) {
        line[i] = '\0';
    }
}

void substring_before_symbol(char *line, char symbol){
    int start_index = 0;
    
    int line_len = str_len(line);
    
    int copy_len = str_find(line, symbol);
    if (copy_len == line_len){
        printf("EWATAHTELLL\n");
    } else{
        mem_copy(line, line, str_find(line, symbol) * sizeof(char));
        line[copy_len] = '\0';
    }
}
    

char* get_substring_before_symbol(const char *line, char symbol){
    int start_index = 0;
    
    char *buffer = temp_lines[temp_line_index];
    buffer[0] = 0;
    
    i32 symbol_index = str_find(line, symbol);
    mem_copy(buffer, (void*)line, symbol_index * sizeof(char));
    
    // int line_len = str_len(line);
    buffer[symbol_index] = '\0';
    
    increment_temp_line_index();
    return buffer;
}

b32 str_end_with(char *str, const char *end_with){
    int len = str_len(str);
    int end_with_len = str_len(end_with);
    
    if (end_with_len > len){
        return false;
    }
    
    for (int i = 0; i < end_with_len; i++){
        if (str[len - end_with_len + i] != end_with[i]){
            return false;
        }
    }
    
    return true;
}

b32 str_contains(const char *str, const char *contains){
    size_t len = str_len(str);
    size_t contains_len = str_len(contains);
    
    size_t match_count = 0;
    //b32 found_first_char = false;
    
    if (contains_len > len){
        return false;
    }
    
    for (int i = 0; i < len && match_count < contains_len; i++){
        if (match_count == 0 && len - i < contains_len){
            return false;
        }
    
        if (to_lower(str[i]) == to_lower(contains[match_count])){
            match_count++;
        } else{
            //return false;
            match_count = 0;
        }
    }    
    
    return match_count == contains_len;
}

// b32 str_contains(const char *str, const char *contains){
//     return str_contains_const(str, contains);
// }

// b32 str_contains(char *str, char *contains){
//     return str_contains_const(str, contains);
// }

b32 str_equal(char *first, const char *second){
    int len1 = str_len(first);
    int len2 = str_len(second);
    
    if (len1 != len2 || len1 == 0) return false;
    
    for (int i = 0; i < len1; i++){
        if (first[i] != second[i]) return false;
    }
    
    return true;
}

b32 str_equal(char *first, char *second){
    int len1 = str_len(first);
    int len2 = str_len(second);
    
    if (len1 != len2 || len1 == 0) return false;
    
    for (int i = 0; i < len1; i++){
        if (first[i] != second[i]) return false;
    }
    
    return true;
}

b32 str_equal(const char *first, char *second){
    int len1 = str_len(first);
    int len2 = str_len(second);
    
    if (len1 != len2 || len1 == 0) return false;
    
    for (int i = 0; i < len1; i++){
        if (first[i] != second[i]) return false;
    }
    
    return true;
}

const char *tprintf(const char *text, ...){
    char *current_buffer = temp_lines[temp_line_index];
    // mem_set(current_buffer, 0, MEDIUM_STR_LEN);
    current_buffer[0] = 0;
    
    va_list args;
    va_start(args, text);
    i32 byte_count = vsnprintf(current_buffer, MEDIUM_STR_LEN, text, args);
    va_end(args);
    
    if (byte_count >= MEDIUM_STR_LEN){
        char *trunc_buffer = current_buffer + MEDIUM_STR_LEN - 15;
        sprintf(trunc_buffer, "OVERFLOWWstati");
    }
    
    increment_temp_line_index();
    
    return current_buffer;
}

i32 to_i32(const char *text){
    i32 value = 0;
    i32 sign = 1;
    
    if (text[0] == '-') sign = -1;
    while ((text[0] == '+') || (text[0] == '-')){
        text++;
    }
    
    for (i32 i = 0; ((text[i] >= '0') && (text[i] <= '9')); i++){
        value = value * 10 + (i32)(text[i] - '0');
    }
    
    return value * sign;
}

u64 to_u64(const char *text){
    u64 value = 0;
    u64 sign = 1;
    
    if (text[0] == '-') sign = -1;
    while ((text[0] == '+') || (text[0] == '-')){
        text++;
    }
    
    for (u64 i = 0; ((text[i] >= '0') && (text[i] <= '9')); i++){
        value = value * 10 + (u64)(text[i] - '0');
    }
    
    return value * sign;
}

f32 to_f32(const char *text){
    f32 value = 0.0f;
    f32 sign = 1.0f;
    
    if (text[0] == '-') sign = -1.0f;
    while (text[0] == '+' || text[0] == '-'){
        text++;
    }
    
    i32 i = 0;
    for (; (text[i] >= '0' && text[i] <= '9'); i++){
        value = value * 10.0f + (f32)(text[i] - '0');
    }
    
    if (text[i++] == '.'){
        f32 divisor = 10.0f;
        for (; text[i] >= '0' && text[i] <= '9'; i++){
            value += ((f32)(text[i] - '0')) / divisor;
            divisor *= 10;
        }
    }
    
    return value * sign;
}

struct String {
    Allocator *allocator = NULL;
    char *data;
    i32 count = 0;
    
    b32 operator==(String str1){
        if (count != str1.count) return false;
        
        for (int i = 0; i < count; i++){
            if (data[i] != str1.data[i]) return false;
        }
        
        return true;
    }
    
    b32 operator==(const char *line){
        int len = str_len(line);
    
        if (count != len) return false;
        
        for (int i = 0; i < count; i++){
            if (data[i] != line[i]) return false;
        }
        
        return true;
    }
    
    void free_data(){
        // If allocator is persist we don't free that one because Allocator is just arena for the time being.
        if (!allocator){
            free(data);
            count = 0;
        }
    }
};

String make_string(Allocator *allocator, const char *text, ...){
    String result_string = {.allocator = allocator};
    
    va_list args;
    va_start(args, text);
    i32 byte_count = vsnprintf(result_string.data, 0, text, args);
    result_string.data = alloc(result_string.allocator, byte_count + 1);
    vsnprintf(result_string.data, byte_count + 1, text, args);
    va_end(args);
    
    result_string.count = byte_count;
    
    return result_string;
}

String make_substring(String original_string, int start_index, int end_index, Allocator *allocator) {
    String new_string = {.allocator = allocator};
    
    if (end_index < start_index) {
        return new_string;
    }
    
    new_string.count = end_index - start_index + 1; // +1 because if end_index - start_index == 0 we need to add that exact index to substring etc..
    
    new_string.data = alloc(new_string.allocator,  new_string.count + 1); // +1 for null termination.
    // @TODO: think about null termination.
    mem_copy(new_string.data, original_string.data + start_index, new_string.count * sizeof(char));
    new_string.data[new_string.count] = '\0';
    return new_string;
}

String tstring(const char *text, ...) {
    String result_string = {.allocator = &temp_allocator};
    
    va_list args;
    va_start(args, text);
    i32 byte_count = vsnprintf(result_string.data, 0, text, args);
    result_string.data = alloc(result_string.allocator, byte_count + 1);
    vsnprintf(result_string.data, byte_count + 1, text, args);
    va_end(args);
    
    result_string.count = byte_count;
    
    return result_string;
}

// String copy_string(String to_copy){
//     String new_string;

//     new_string.count = to_copy->count;
    
//     new_string.data = (char*)calloc(1, new_string.capacity * sizeof(char));
//     str_copy(new_string.data, to_copy->data);
    
//     return new_string;
// }

struct Medium_Str{
    char data[MEDIUM_STR_LEN];  
};

#define LONG_STR_LEN 2048

struct Long_Str {
    char data[LONG_STR_LEN];  
};


struct String_Builder {
    Allocator *allocator = NULL;  
    
    char *data = NULL;
    i32 count = 0;
    i32 capacity = 0;
};

void init_string_builder(String_Builder *builder, i32 capacity) {
    assert(!builder->data);
    assert(builder->capacity == 0);
    builder->data = alloc(builder->allocator, capacity);
    builder->capacity = capacity;
    builder->count = 0;
}

String_Builder make_string_builder(i32 capacity, Allocator *allocator) {
    String_Builder builder = {.allocator = allocator};
    init_string_builder(&builder, capacity);
    return builder;
}

// @TODO: I would like to get rid of null ternimation on strings, but while we working with Raylib that could be hard.
static inline void builder_grow_if_need(String_Builder *builder, String appended_string) {
    assert(builder->data && builder->capacity > 0);
    
    if (builder->count + appended_string.count + 1 > builder->capacity) {
        char *old_data = builder->data;
        
        while(builder->count + appended_string.count + 1 > builder->capacity) {
            builder->capacity *= 2;
        }
        
        builder->data = alloc(builder->allocator, builder->capacity);
        mem_copy(builder->data, old_data, builder->count * sizeof(char));
        
        free_data_in_allocator(builder->allocator, old_data);
    }
}

void builder_append(String_Builder *builder, String appended_string) {
    if (!builder->data) init_string_builder(builder, 2048);
    
    builder_grow_if_need(builder, appended_string);    
    
    mem_copy(builder->data + builder->count, appended_string.data, appended_string.count * sizeof(char));
    builder->count += appended_string.count;
    builder->data[builder->count] = '\0';
}

void builder_free(String_Builder *builder) {
    assert(builder->data);
    free_data_in_allocator(builder->allocator, builder->data);
}

inline String make_string_from_builder(String_Builder *builder, Allocator *allocator) {
    // @TODO: This works while we're using null termination.
    return make_string(allocator, builder->data);
}

#include "array_new.hpp"

void split_string(Array<String> *result_array, String to_split, String separators) {
    if (separators.count <= 0) {    
        return;
    }
    
    int ground_index = 0;
    
    for (int i = 0; i < to_split.count; i++) {
        for (int s = 0; s < separators.count; s++) {
            if (to_split.data[i] == separators.data[s]) {
                // That check exists for continuous separators.
                if (ground_index < i) {                
                    result_array->append(make_substring(to_split, ground_index, i - 1, result_array->allocator)); // i - 1 because make_substring include end index and we don't want to add separator to string.
                }
                    
                ground_index = i + 1;
            }
        }
    }
    
    // At that point ground index should be equal to (last separator + 1) and we need to add last substring to array.
    // So if last separator was last symbol - ground_index would be equal to to_split.count.
    if (ground_index <= to_split.count - 1) {
        result_array->append(make_substring(to_split, ground_index, to_split.count - 1, result_array->allocator));
    }
}

Array<String> split_string(String to_split, String separators, Allocator *allocator) {
    Array<String> result = {.allocator = allocator};     
    split_string(&result, to_split, separators);    
    return result;
}
