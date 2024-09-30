#pragma once

template<typename T>
struct Array{
    T *data;
    int count = 0;
    int max_count = 0;

    Array(int _max_count){
        max_count = _max_count;
        count = 0;
        data = (T*)malloc(_max_count * sizeof(T));
    }
    
    Array(){
        max_count = 1;
        count = 0;
        data = (T*)malloc(max_count * sizeof(T));
    }
    
    T get(int index){
        return data[index];
    }
    
    T* get_ptr(int index){
        return &data[index];
    }
    
    void add(T value){
        //if (count >= max_count) return;
        
        if (count >= max_count){     
            assert(max_count > 0);
        
            max_count *= 2;
            
            T *old_data = (T*)malloc(count * sizeof(T));
            
            mem_copy(old_data, data, count * sizeof(T));
            free(data);
            
            data = (T*)malloc(max_count * sizeof(T));
            mem_copy(data, old_data, count * sizeof(T));
            
            free(old_data);
        }
        
        // T* element = get(count);
        // memmove(element, value, sizeof(T));
        data[count] = value;
        count++;
    }
    
    void remove(int index){
        for (int i = index; i < count - 1; i++){
            T *current_element = get_ptr(i);
            T *next_element    = get_ptr(i + 1);
            memmove(current_element, next_element, sizeof(T));
        }
        
        count--;
    }
    
    T last(){
        return data[count-1];
    }
    
    T* last_ptr(){
        return &data[count-1];
    }
    
    void clear(){
        // free(data);
        // data = (T*)malloc(max_count * sizeof(T));
        count = 0;
    }
    
    void free_arr(){
        free(data);
    }
    
    // ~Array(){
    //     free(data);   
    // }
};

Array<String> split_str(String str, char *separators, int separators_count){
    Array<String> result = Array<String>(10);
    
    String current_string = String();
    
    for (int i = 0; i < str.count; i++){
        char ch = str.line[i];
        
        int need_separate = 0;
        for (int s = 0; s < separators_count; s++){
            if (ch == separators[s]){
                need_separate = 1;
                break;
            }
        }
        
        if (need_separate){
            if (current_string.count != 0){
                String copied_string = String(&current_string);
                result.add(copied_string);
                
                current_string.clear();
            }
        } else{
            current_string += ch;   
        }
    }
    
    if (current_string.count != 0){
        result.add(current_string);
    }
    
    return result;
}
