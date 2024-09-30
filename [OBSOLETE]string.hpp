#pragma once

size_t str_len(const char *line){
    size_t result = 0;   
    while (line[result] && result < 100){
        result++;
    }
    
    return result;
}

void mem_copy(void *dest, void *source, size_t bytes){
    char *new_dest   = (char*)dest;
    char *new_source = (char*)source;
    for (int i = bytes - 1; i >= 0; --i){
        new_dest[i] = new_source[i];
        //printf("%s\n", new_dest[i]);
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

struct String{
    String(const char *_line){  
        count = str_len(_line);
        max_count = count;
        
        if (max_count < 16){
            max_count = 16;
        }
        
        line = (char*)malloc(max_count * sizeof(char));
        
        str_copy(line, _line);
    }
    
    String(){  
        count = 0;
        max_count = 16;
        
        line = (char*)malloc(max_count * sizeof(char));
    }
    
    String(String *str_to_copy){
        count = str_to_copy->count;
        max_count = str_to_copy->max_count;
        
        line = (char*)malloc(max_count * sizeof(char));
        str_copy(line, str_to_copy->line);
    }
    
    char *line;
    size_t count;
    size_t max_count;
    
    void operator+=(const char *add_str){
        size_t add_count = str_len(add_str);
        
        //+1 to safely put '\0' at end
        if (count + add_count + 1 > max_count){
            max_count *= 2;
            char old_line[count];
            str_copy(old_line, line);
            free(line);
            
            line = (char*)malloc(max_count * sizeof(char));
            
            str_copy(line, old_line);
        }
        
        str_copy(line + count, add_str);
        count += add_count; 
        line[count] = '\0';
    }
    
    void operator+=(char ch){
        //+1 to safely put '\0' at end
        if (count + 2 > max_count){
            max_count *= 2;
            char old_line[count];
            str_copy(old_line, line);
            free(line);
            
            line = (char*)malloc(max_count * sizeof(char));
            
            str_copy(line, old_line);
        }
        
        line[count] = ch;
        count++;; 
        line[count] = '\0';
    }
    
    void clear(){
        count = 0;   
        line[0] = '\0';
    }
};


