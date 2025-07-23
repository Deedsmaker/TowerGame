#pragma once

#include "array.hpp"
#include "string.hpp"

struct File{
    //When we will need to keep file open this will be changed
    //FILE *fptr;
    b32 loaded = false;
    char name[MEDIUM_STR_LEN];
    Dynamic_Array<Long_Str> lines;
};

File load_file(const char *name, const char *mode){
    File file;
    //file.name = (char*)malloc(str_len(name) * sizeof(char));
    str_copy(file.name, name);
    file.lines = Dynamic_Array<Long_Str>(128);
    
    FILE *fptr = fopen(name, mode);
    
    //const unsigned MAX_LENGTH = 5000;
    char buffer[LONG_STR_LEN];
    
    if (fptr == NULL){
        printf("NO FILE LOADING TODAY: %s\n", name);
    }

    while (fptr != NULL && fgets(buffer, LONG_STR_LEN, fptr)){
        //String str = init_string_from_str(buffer);
        
        size_t buffer_len = str_len(buffer);
        
        if (buffer[buffer_len-1] == '\n'){
            buffer_len--;
            buffer[buffer_len] = '\0';
        }
        
        file.lines.add({});
        str_copy(file.lines.last_ptr()->data, buffer);
    }
    
    if (fptr){
        fclose(fptr);
        file.loaded = true;
    }
    
    return file;
}

void unload_file(File *file){
    //free_string_array(&file->lines);
    file->lines.free_arr();
    //free(file->name);
}



