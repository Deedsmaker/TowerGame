#pragma once

#include <assert.h>

#include <string.h>

#define for_array(index, array) for (i32 index = 0; index < array->count; index++)

void grow_if_need(void **data, size_t element_size, i32 *capacity, i32 current_count, i32 appended_count) {
    i32 new_count = current_count + appended_count;
    if (new_count > *capacity) {
        void *old_data = *data;
        
        while (new_count > *capacity) {
            if (*capacity == 0) {
                *capacity = 8;
            } else {
                *capacity *= 2;
            }
        }
        
        *data = malloc(*capacity * element_size);
        // old_data could be not present if we growing for the first time (so data was null).
        if (old_data) {
            memcpy(*data, old_data, current_count * element_size);
        }
        
        free(old_data);
    }
}

template<typename T>
struct Array {
    T *data;  
    i32 count;
    i32 capacity;
    
    inline T *get(i32 index) {
        assert((index >= 0 && index < count) && "Index out of bounds!");
        
        return &data[index];
    }
    
    inline T get_value(i32 index) {
        assert((index >= 0 && index < count) && "Index out of bounds!");
        
        return data[index];
    }
    
    T *append(T value) {
        grow_if_need((void **)(&data), sizeof(T), &capacity, count, 1);
        
        data[count] = value;
        count += 1;
        
        return last();
    }
    
    void remove(i32 index) {
        assert((index >= 0 && index < count) && "Index out of bounds!");
        
        if (index == count - 1) {
            count -= 1;            
            return;
        }
        
        memmove(get(index), get(index+1), sizeof(T) * (count - index - 1));
        count--;
    }
    
    void remove_first_half(){
        int half_count = (int)((float)count * 0.5f);
        int even_correction = count % 2 == 0 ? -1 : 0;
        mem_copy(get(0), get(half_count + even_correction), half_count * sizeof(T));
        
        count = half_count;
    }
    
    b32 contains(T *to_found) {
        for_array(i, this) {
            if (*get(i) == *to_found) {
                return true;
            }
        }
        
        return false;
    }
    
    inline b32 contains(T to_found) {
        return contains(&to_found);
    }
    
    i32 find(T *to_find) {    
        for_array(i, this) {
            if (*get(i) == *to_find) {
                return i;
            }
        }
        
        return -1;
    }
    inline i32 find(T to_find) {
        return find(&to_find);
    }
    
    void free() {
        assert(data);
        free(data);
        data = NULL;
        capacity = 0;
        count = 0;
    }
    
    void clear() {
        count = 0;
    }
    
    inline T *last() {
        return &data[count - 1];
    }
    inline T last_value() {
        return data[count - 1];
    }
};

template<typename T>
void init_array(Array<T> *array, i32 capacity) {
    assert(array->data == NULL && "We probably should init array only when it is not initialized");
    array->capacity = capacity;
    
    array->data = (T*) malloc(capacity * sizeof(T));
}

template<typename T, i32 C>
struct Static_Array {
    T data[C];  
    i32 count;
    i32 capacity = C;
    
    inline T *get(i32 index) {
        assert((index >= 0 && index < count) && "Index out of bounds!");
        
        return &data[index];
    }
    
    inline T get_value(i32 index) {
        assert((index >= 0 && index < count) && "Index out of bounds!");
        
        return data[index];
    }
    
    T *append(T value) {
        assert(count < capacity && "Appending in static array more than we can.");
    
        data[count] = value;
        count += 1;
        
        return last();
    }
    
    void remove(i32 index) {
        assert((index >= 0 && index < count) && "Index out of bounds!");
        
        if (index == count - 1) {
            count -= 1;            
            return;
        }
        
        memmove(get(index), get(index+1), sizeof(T) * (count - index - 1));
        count--;
    }
    
    //nocheckin change names to pop and pop_value.
    T pop(){
        assert(count > 0);
    
        return data[--count];
    }
    
    T* pop_ptr(){
        assert(count > 0);
    
        return &data[--count];
    }
    
    void remove_first_half(){
        int half_count = (int)((float)count * 0.5f);
        int even_correction = count % 2 == 0 ? -1 : 0;
        mem_copy(get(0), get(half_count + even_correction), half_count * sizeof(T));
        
        count = half_count;
    }
    
    b32 contains(T *to_found) {
        for_array(i, this) {
            if (*get(i) == *to_found) {
                return true;
            }
        }
        
        return false;
    }
    
    inline b32 contains(T to_found) {
        return contains(&to_found);
    }
    
    i32 find(T *to_find) {    
        for_array(i, this) {
            if (*get(i) == *to_find) {
                return i;
            }
        }
        
        return -1;
    }
    inline i32 find(T to_find) {
        return find(&to_find);
    }
    
    void clear() {
        count = 0;
    }
    
    inline T *last() {
        return &data[count - 1];
    }
    inline T last_value() {
        return data[count - 1];
    }
};

#define for_chunk_array(element, type, arr) type *element = NULL; for (i32 i = arr->next_avaliable(arr, 0, &element); i < arr->chunks_count * arr->chunk_size && element; i = arr->next_avaliable(arr, i + 1, &element)) 

template<typename T>
struct Chunk_Array {
    struct Array_Chunk_Element {
        T value;  
        b32 occupied;
    };
    struct Array_Chunk {
        Array_Chunk_Element *chunk_data;  
        i32 occupied_count;
        
        Array_Chunk *next;
    };
    Allocator *allocator;
    Array_Chunk *first_chunk;
    i32 chunk_size = 32;
    i32 chunks_count;

    inline b32 index_in_chunk(i32 index, Array_Chunk *chunk, i32 chunk_index) {
        return index >= chunk_index * chunk_size && index < (chunk_index + 1) * chunk_size;
    }
    
    i32 next_avaliable(i32 start_index, T **element) {
        i32 start_chunk_index = start_index / chunk_size;
        Array_Chunk *chunk = first_chunk;
        for (i32 i = 1; i < start_chunk_index; i++) chunk = chunk->next;
        
        i32 start_index_in_chunk = start_index - start_chunk_index * chunk_size;
        for (i32 i = start_chunk_index; i < chunks_count; i++) {
            for (i32 j = i == start_chunk_index ? start_index_in_chunk : 0; j < chunk_size; j++) {
                Array_Chunk_Element *array_element = &chunk->chunk_data[j];
                if (array_element->occupied) {
                    *element = &array_element->value;
                    return j + i * chunk_size;
                }
            }
            
            chunk = chunk->next;
        }
    
        *element = NULL;
        return chunks_count * chunk_size;
    }

    
    inline T *get(i32 index) {
        assert((index >= 0 && index < chunk_size * chunks_count) && "Index out of bounds!");
        
        Array_Chunk *chunk = first_chunk;
        for (i32 i = 0; i < chunks_count; i++) {
            if (i > 0) chunk = chunk->next;
            if (index_in_chunk(index, chunk, i)) {
                // That chunk could be not currently occupied. Not sure what we should do about that.
                return &chunk->chunk_data[index - (i * chunk_size)].value;
            }
        }
        
        assert(false && "That should not happen.");
        return NULL;
    }
    
    inline T get_value(i32 index) {
        return *get(index);
    }
    
    //nocheckin implement.
    i32 find_free_space_and_grow_if_need(i32 appended_count) {
        for_chunk_array(element, T, this) {   
                       
        }
        
        return -1;
    }
    
    T *append(T value) {
        i32 add_index = find_free_space_and_grow_if_need(1);
        assert(add_index >= 0);
    
        Array_Chunk *chunk = first_chunk;
        for (i32 i = 0; i < chunks_count; i++) {
            if (i > 0) chunk = chunk->next;
            if (index_in_chunk(add_index, chunk, i)) {
                Array_Chunk_Element *chunk_element = &chunk->chunk_data[add_index - (i * chunk_size)];
                chunk_element->occupied = true;
                chunk_element->value = value;
                chunk->occupied_count += 1;
                assert(chunk->occupied_count <= chunk_size);
                return &chunk_element->value;
            }
        }
        
        assert(false && "Failed to add elemnt to chunk array");        
        return NULL;
    }
    
    void remove(i32 index) {
        Array_Chunk *chunk = first_chunk;
        for (i32 i = 0; i < chunks_count; i++) {
            if (i > 0) chunk = chunk->next;
            if (index_in_chunk(index, chunk, i)) {
                Array_Chunk_Element *chunk_element = &chunk->chunk_data[index - (i * chunk_size)];
                
                chunk_element->occupied = false;
                chunk->occupied_count -= 1;
                assert(chunk->occupied_count > 0);
                
                return;
            }
        }
        
        assert(false && "Tried to remove index that is not present in chunk array");
    }
    
    // b32 contains(T *to_found) {
    //     for (i32 i = 0; i < count; i++) {
            
    //     }
    //     for_array(i, this) {
    //         if (*get(i) == *to_found) {
    //             return true;
    //         }
    //     }
        
    //     return false;
    // }
    
    // inline b32 contains(T to_found) {
    //     return contains(&to_found);
    // }
    
    // i32 find(T *to_find) {    
    //     for_array(i, this) {
    //         if (*get(i) == *to_find) {
    //             return i;
    //         }
    //     }
        
    //     return -1;
    // }
    // inline i32 find(T to_find) {
    //     return find(&to_find);
    // }
    
    // void clear() {
    //     count = 0;
    // }
};

