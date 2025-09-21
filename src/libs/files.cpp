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
    
b32 directory_exists(String name) {
    return DirectoryExists(c_str(name));
}

b32 create_directory_if_not_exists(String name) { 
    if (!directory_exists(name)) {
        return MakeDirectory(c_str(name)) == 0;   
    }
    
    return false;
}
