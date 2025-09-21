#pragma once 

#include "string.hpp"
#include "Allocator.cpp"

String read_entire_file(String name, b32 *success, Allocator *allocator = HEAP_ALLOCATOR) {
    
    char *text_data = LoadFileText(c_str(name));
    
    if (!text_data) {
        if (success) *success = false;
        return {0};
    }
    
    String s = make_string(allocator, text_data);;
    
    UnloadFileText(text_data);    
    
    if (success) *success = true;
    
    return s;
}

b32 write_entire_file(String name, String_Builder *builder) {
    return SaveFileText(c_str(name), c_str(builder));
}
    
