#pragma once 

#include "string.hpp"
#include "Allocator.cpp"
#include "array.hpp"

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

b32 make_directory_if_not_exists(String name) { 
    if (!directory_exists(name)) {
        return MakeDirectory(c_str(name)) == 0;   
    }
    
    return false;
}

b32 file_exists(String name) {
    return FileExists(c_str(name));
}

b32 delete_file(String name) {
    if (!file_exists(name)) return false;
    
    return FileRemove(c_str(name));
}

// Rename file or directory.
b32 rename_file(String name, String new_name) {
    if (!file_exists(name)) return false;
    
    return FileRename(c_str(name), c_str(new_name));
}
b32 rename_directory(String name, String new_name) {
    if (!directory_exists(name)) return false;   
    
    return rename_file(name, new_name);
}

Array <String> get_files_in_directory(String directory_name, Allocator *allocator = HEAP_ALLOCATOR) {
    if (!directory_exists(directory_name)) return {0};
    
    FilePathList files_paths = LoadDirectoryFiles(c_str(directory_name));
    
    Array <String> result_paths = {.allocator = allocator};
    
    for (u32 i = 0; i < files_paths.count; i++) {
        result_paths.append(make_string(allocator, files_paths.paths[i]));
    }
    
    UnloadDirectoryFiles(files_paths);
    
    return result_paths;
}

#ifdef _WIN32
    #include <direct.h>
#endif
b32 delete_directory(String path) {
    if (!directory_exists(path)) return false;
      
    auto files = get_files_in_directory(path, &temp_allocator);
    
    for_array(i, &files) {
        delete_file(files.get_value(i));
    }
    
    return _rmdir(c_str(path)) == 0;
}
