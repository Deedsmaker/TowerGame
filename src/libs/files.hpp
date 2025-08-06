#pragma once

#include "array.hpp"
#include "string.hpp"

struct File{
    //When we will need to keep file open this will be changed
    //FILE *fptr;
    b32 loaded = false;
    String name;
    Array<String> lines = {0};
    Allocator *allocator;
};

File load_file(const char *name, const char *mode, Allocator *allocator){
    File loaded_file = {0};
    init_array(&loaded_file.lines, 128, allocator);
    //loaded_file.name = (char*)malloc(str_len(name) * sizeof(char));
    // str_copy(loaded_files.name, name);
    loaded_file.name = make_string(allocator, name);
    
    FILE *fptr = fopen(name, mode);
    
    char buffer[4096];
    
    if (fptr == NULL){
        printf("Could not load file: %s\n", name);
    }

    while (fptr != NULL && fgets(buffer, 4096, fptr)){
        //String str = init_string_from_str(buffer);
        
        // size_t buffer_len = str_len(buffer);
        
        // if (buffer[buffer_len-1] == '\n'){
        //     buffer_len--;
        //     buffer[buffer_len] = '\0';
        // }
        
        loaded_file.lines.append(make_string(allocator, buffer));
        // str_copy(loaded_file.lines.last()->data, buffer);
    }
    
    if (fptr){
        fclose(fptr);
        loaded_file.loaded = true;
    }
    
    return loaded_file;
}

void unload_file(File *file){
    for_array(i, (&file->lines)) {
        file->lines.get(i)->free_data();
    }
    
    file->lines.free_data();
    file->name.free_data();
}



