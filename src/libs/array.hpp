#pragma once

#include <string.h>

// template<typename T>
// struct Dynamic_Array_old {
//     T *data = NULL;
//     int count = 0;
//     int capacity = 0;

//     Dynamic_Array_old(int _capacity){
//         capacity = _capacity;
//         count = 0;
//         data = (T*)malloc(_capacity * sizeof(T));
//     }
    
//     Dynamic_Array_old(){
//         capacity = 0;
//         count = 0;
//         //data = (T*)malloc(capacity * sizeof(T));
//     }
    
//     T get(int index){
//         return data[index];
//     }
    
//     T* get(int index){
//         return &data[index];
//     }
    
//     T* append(T value){
//         //if (count >= capacity) return;
        
//         if (capacity == 0){
//             capacity = 2;
//             data = (T*)malloc(capacity * sizeof(T));
//         }
        
//         if (count >= capacity){     
//             assert(capacity > 0);
        
//             capacity *= 2;
            
//             T *old_data = data;//(T*)malloc(count * sizeof(T));
//             // mem_copy(old_data, data, count * sizeof(T));
            
//             data = (T*)malloc(capacity * sizeof(T));
//             mem_copy(data, old_data, count * sizeof(T));
            
//             free(old_data);
//         }
        
//         // T* element = get(count);
//         // mem_copy(element, value, sizeof(T));
//         data[count] = value;
//         count++;
        
//         return last_ptr();
//     }
    
//     void remove(int index){
//         if (index == count - 1){
//             count--;
//             return;
//         }
    
//         // for (int i = index; i < count - 1; i++){
//         //     // T *current_element = get(i);
//         //     // T *next_element    = get(i + 1);
//         //     //mem_copy(get(i), get(i+1), sizeof(T));
            
            
//         // }
        
//         memmove(get(index), get(index+1), sizeof(T) * (count - index - 1));
        
//         //mem_copy(get(index), last_ptr(), sizeof(T));
        
//         count--;
//     }
    
//     void remove_first_half(){
//         i32 half_count = (i32)((f32)count * 0.5f);
//         i32 even_correction = count % 2 == 0 ? -1 : 0;
//         mem_copy(get(0), get(half_count + even_correction), half_count * sizeof(T));
        
//         count = half_count;
//     }
    
//     int find(T to_find){
//         for (int i = 0; i < count; i++){ 
//             if (data[i] == to_find){
//                 return i;
//             }
//         }
        
//         return -1;
//     }
    
//     T last(){
//         return data[count-1];
//     }
    
//     T* last_ptr(){
//         return &data[count-1];
//     }
    
//     void clear(){
//         // free(data);
//         // data = (T*)malloc(capacity * sizeof(T));
//         count = 0;
//     }
    
//     void free(){
//         if (data){
//             free(data);
//         }
//         capacity = 0;
//         count = 0;
//     }
    
//     b32 contains(T to_found){
//         for (int i = 0; i < count; i++){
//             if (get(i) == to_found){
//                 return true;
//             }
//         }
        
//         return false;
//     }

    
//     // ~Array_old(){
//     //     free(data);   
//     // }
// };

// template<typename T, int C>
// struct Array_old{
//     T data[C];
//     int count = 0;
//     int capacity = 0;

//     Array_old(){
//         capacity = C;
//         count = 0;
//     }
    
//     //nocheckin names
//     T get_value(int index){
//         return data[index];
//     }
    
//     T* get(int index){
//         return &data[index];
//     }
    
//     T* append(T value){
//         // assert(count < capacity);     
        
//         if (count >= capacity){
//             printf("WARNING: in static array was added more than YOU SHOULD DOG\n");
//             return NULL;
//         } else{
//             data[count++] = value;
//         }
        
//         return last();
//     }
    
//     void remove(int index){
//         if (index == count - 1){
//             count--;
//             return;
//         }
        
//         memmove(get(index), get(index+1), sizeof(T) * (count - index - 1));
//         count--;
//     }
    
//     void remove_first_half(){
//         int half_count = (int)((float)count * 0.5f);
//         int even_correction = count % 2 == 0 ? -1 : 0;
//         mem_copy(get(0), get(half_count + even_correction), half_count * sizeof(T));
        
//         count = half_count;
//     }
    
//     T pop(){
//         assert(count > 0);
    
//         return data[--count];
//     }
    
//     T* pop_ptr(){
//         assert(count > 0);
    
//         return &data[--count];
//     }
    
//     T last_value(){
//         return data[count-1];
//     }
    
//     T* last(){
//         return &data[count-1];
//     }
    
//     void clear(){
//         count = 0;
//     }
    
//     b32 contains(T to_found){
//         for (int i = 0; i < count; i++){
//             if (get_value(i) == to_found){
//                 return true;
//             }
//         }
        
//         return false;
//     }
// };

template<typename T>
struct Table_Data{
    i64 key = -1;
    T value;
};

template<typename T>
struct Hash_Table_Int{
    Table_Data<T> *data;
    //int count = 0;
    int capacity = 1000;
    int total_added_count = 0;
    i64 last_added_key = -1;

    Hash_Table_Int(int count = 10000){
        capacity = count;
        
        data = (Table_Data<T>*)malloc(capacity * sizeof(Table_Data<T>));

        for (int i = 0; i < capacity; i++) {
            data[i].key = -1;
        }
        //count = 0;
    }
    
    b32 has_key(int key){
        if (key == -1){
            return false;
        }
    
        return data[key%capacity].key != -1;
    }
    
    b32 has_key(i64 key){
        if (key == -1){
            return false;
        }
    
        return data[key%capacity].key != -1;
    }
    
    b32 has_index(int index){
        if (index == -1){
            return false;
        }
    
        return data[index].key != -1;
    }
    
    void remove_key(int key){
        data[key%capacity].key = -1;
    }
    
    void remove_key(i64 key){
        data[key%capacity].key = -1;
    }
    
    void remove_index(int index){
        data[index].key = -1;
    }
    
    T get_value(int index){
        return data[index].value;
    }
    
    T* get(int index){
        return &data[index].value;
    }
    
    T get_by_key(i64 key){
        return data[key%capacity].value;
    }
    
    T* get_by_key_ptr(i64 key){
        if (!has_key(key)){
            return NULL;
        }

        return &data[key%capacity].value;
    }
    
    b32 append(int key, T value){
        //assert(count < capacity);     
        //assert(!has_key(key));        
        if (has_key(key)){
            return false;
        }
        
        data[key%capacity].value = value;
        data[key%capacity].key = key;
        
        last_added_key = key;
        total_added_count++;
        return true;
    }
    
    b32 append(i64 key, T value){
        //assert(count < capacity);     
        //assert(!has_key(key));        
        if (has_key(key)){
            return false;
        }
        
        data[key%capacity].value = value;
        data[key%capacity].key = key;
        
        last_added_key = key;
        total_added_count++;
        return true;
    }
    
    // T pop(){
    //     assert(count > 0);
    
    //     return data[--count];
    // }
    
    // T* pop_ptr(){
    //     assert(count > 0);
    
    //     return &data[--count];
    // }
    
    T last_value(){
        return data[last_added_key%capacity].value;
    }
    
    T *last(){
        return &data[last_added_key%capacity].value;
    }
    
    void clear(){
        for (int i = 0; i < capacity; i++){
            // if (data[i].key != -1){
                data[i].key = -1;
            // }
        }
        
        total_added_count = 0;
        last_added_key = -1;
    }
};


// template<typename T>
// void copy_array(Dynamic_Array_old<T> *dest, Dynamic_Array_old<T> *src){
//     //*dest = *src;
//     dest->clear();
    
//     for (int i = 0; i < src->count; i++){
//         dest->append(src->get(i));
//     }
    
//     //dest->data = (T*)malloc(dest->count * sizeof(T));
//     //mem_copy(dest->data, src->data, dest->count * sizeof(T));
// }

// void split_str(char *str, const char *separators, Dynamic_Array_old<Medium_Str> *result){
//     result->clear();

//     size_t len = str_len(str);
//     size_t separators_len = str_len(separators);
    
//     char temp_str[MEDIUM_STR_LEN];
    
//     int start_index = 0;
//     int current_index = 0;
    
//     for (int i = 0; i < len; i++){
//         char ch = str[i];
        
//         int need_separate = 0;
//         for (int s = 0; s < separators_len; s++){
//             if (ch == separators[s]){
//                 need_separate = 1;
//                 break;
//             }
//         }
        
//         if (need_separate){
//             if (current_index - start_index != 0){
//                 size_t added_len = (current_index - start_index);
//                 assert(added_len < MEDIUM_STR_LEN - 1);
            
//                 mem_copy(temp_str, str + start_index, added_len * sizeof(char));
//                 temp_str[added_len] = '\0';
                
//                 result->append({});
//                 str_copy(result->last_ptr()->data, temp_str);
//             }
            
//             start_index = current_index + 1;
//         }
            
//         current_index++;
//     }
    
//     if (current_index - start_index != 0){
//         size_t added_len = (current_index - start_index);
//         assert(added_len < MEDIUM_STR_LEN - 1);
    
//         mem_copy(temp_str, str + start_index, added_len * sizeof(char));
//         temp_str[added_len] = '\0';
        
//         result->append({});
//         str_copy(result->last_ptr()->data, temp_str);

//         // result->append({});
//         // mem_copy(result->last_ptr()->data, str + start_index, (current_index - start_index) * sizeof(char));
//     }
// }

// void split_str(char *str, const char *separators, Dynamic_Array_old<Long_Str> *result){
//     result->clear();

//     size_t len = str_len(str);
//     size_t separators_len = str_len(separators);
    
//     int start_index = 0;
//     int current_index = 0;
    
//     for (int i = 0; i < len; i++){
//         char ch = str[i];
        
//         int need_separate = 0;
//         for (int s = 0; s < separators_len; s++){
//             if (ch == separators[s]){
//                 need_separate = 1;
//                 break;
//             }
//         }
        
//         if (need_separate){
//             if (current_index - start_index != 0){
//                 result->append({});
                
//                 assert((current_index - start_index
                
//                 char temp_str[LONG_STR_LEN];
//                 mem_copy(temp_str, str + start_index, (current_index - start_index) * sizeof(char));
//                 size_t temp_str_len = str_len(temp_str);
//                 temp_str[temp_str_len] = '\0';
//                 temp_str_len++;
                
//                 mem_copy(result->last_ptr()->data, temp_str, temp_str_len * sizeof(char));
                
//                 start_index = current_index + 1;
//             }
            
//         current_index++;
//     }
    
//     if (current_index - start_index != 0){
//         result->append({});
//         mem_copy(result->last_ptr()->data, str + start_index, (current_index - start_index) * sizeof(char));
//     }
// }

// void split_str(String str, const char *separators, int separators_count, Dynamic_Array_old<String> *result){
//     for (int i = 0; i < result->count; i++){
//         result->get(i)->free_str();
//     }

//     result->clear();

//     String current_string = init_string();
    
//     for (int i = 0; i < str.count; i++){
//         char ch = str.data[i];
        
//         int need_separate = 0;
//         for (int s = 0; s < separators_count; s++){
//             if (ch == separators[s]){
//                 need_separate = 1;
//                 break;
//             }
//         }
        
//         if (need_separate){
//             if (current_string.count != 0){
//                 String copied_string = copy_string(&current_string);
//                 result->append(copied_string);
                
//                 current_string.clear();
//             }
//         } else{
//             current_string += ch;   
//         }
//     }
    
//     if (current_string.count != 0){
//         result->append(current_string);
//     }
// }

// Dynamic_Array_old<String> split_str(String str, const char *separators, int separators_count){
//     Dynamic_Array_old<String> result = Dynamic_Array_old<String>(10);
    
//     split_str(str, separators, separators_count, &result);
    
//     return result;
// }

// void free_string_array(Dynamic_Array_old<String> *arr){
//     for (int i = 0; i < arr->count; i++){
//         arr->get(i)->free_str();
//     }
    
//     arr->free();
// }

