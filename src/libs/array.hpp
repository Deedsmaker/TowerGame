#pragma once

#include <assert.h>

#define for_array(index, array) for (i32 index = 0; index < array->count; index++)

inline void grow_if_need(void **data, size_t element_size, i32 *capacity, i32 current_count, i32 appended_count) {
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
        
        *data = calloc(1, *capacity * element_size);
        // old_data could be not present if we growing for the first time (so data was null).
        if (old_data) {
            memcpy(*data, old_data, current_count * element_size);
            free(old_data);
        }
        
    }
}

template<typename T>
struct Array {
    Allocator *allocator;
    
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
        i32 half_count = (i32)((f32)count * 0.5f);
        i32 even_correction = count % 2 == 0 ? -1 : 0;
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
    
    T pop_value(){
        assert(count > 0);
        return data[--count];
    }
    T* pop(){
        assert(count > 0);
        return &data[--count];
    }
    
    void free_data() {
        if (data) {
            free(data);
            data = NULL;
        } else {
            assert(capacity == 0 && count == 0);
        }
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
void init_array(Array<T> *array, i32 capacity, Allocator *allocator) {
    assert(array->data == NULL && "We probably should init array only when it is not initialized");
    array->capacity = capacity;
    
    array->data = (T*) calloc(1, capacity * sizeof(T));
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
    
    T pop_value(){
        assert(count > 0);
    
        return data[--count];
    }
    T* pop(){
        assert(count > 0);
    
        return &data[--count];
    }
    
    void remove_first_half(){
        i32 half_count = (i32)((f32)count * 0.5f);
        i32 even_correction = count % 2 == 0 ? -1 : 0;
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

#define for_chunk_array_value(chunk_value, type, arr) type *chunk_value = NULL; for (i32 i = arr->next_avaliable_value(0, &chunk_value); i < arr->chunks_count * arr->chunk_size && chunk_value; i = arr->next_avaliable_value(i + 1, &chunk_value)) 

#define for_chunk_array(index, arr) for (i32 index = arr->next_avaliable_index(0); index < arr->chunks_count * arr->chunk_size; index = arr->next_avaliable_index(index + 1)) 

template<typename T>
struct Chunk_Array {
    struct Chunk_Element {
        T value;  
        b32 occupied;
    };
    struct Chunk {
        Chunk_Element *elements;  
        i32 occupied_count;
        
        Chunk *next;
    };
    Allocator *allocator;
    Chunk *first_chunk;
    i32 chunk_size = 32;
    i32 chunks_count;

    inline b32 index_in_chunk(i32 index, Chunk *chunk, i32 chunk_index) {
        return index >= chunk_index * chunk_size && index < (chunk_index + 1) * chunk_size;
    }
    
    inline i32 next_avaliable_value(i32 start_index, T **element) {
        i32 index = next_avaliable_index(start_index);
        if (index < chunks_count * chunk_size) *element = get(index);
        else element = NULL; 
        
        return index;
    }
    
    inline i32 next_avaliable_index(i32 start_index) {
        i32 start_chunk_index = start_index / chunk_size;
        Chunk *chunk = first_chunk;
        for (i32 i = 1; i < start_chunk_index; i++) chunk = chunk->next;
        
        i32 start_index_in_chunk = start_index - start_chunk_index * chunk_size;
        for (i32 i = start_chunk_index; i < chunks_count; i++) {
            for (i32 j = i == start_chunk_index ? start_index_in_chunk : 0; j < chunk_size; j++) {
                Chunk_Element *array_element = &chunk->elements[j];
                if (array_element->occupied) {
                    return j + i * chunk_size;
                }
            }
            
            chunk = chunk->next;
        }
    
        return chunks_count * chunk_size;
    }
    
    T *get(i32 index) {
        assert((index >= 0 && index < chunk_size * chunks_count) && "Index out of bounds!");
        
        Chunk *chunk = first_chunk;
        for (i32 i = 0; i < chunks_count; i++) {
            if (i > 0) chunk = chunk->next;
            if (index_in_chunk(index, chunk, i)) {
                // That chunk could be not currently occupied. Not sure what we should do about that.
                return &chunk->elements[index - (i * chunk_size)].value;
            }
        }
        
        assert(false && "That should not happen.");
        return NULL;
    }
    
    inline T get_value(i32 index) {
        return *get(index);
    }
    
    i32 find_free_space_and_grow_if_need() {
        if (!first_chunk) {
            first_chunk = (Chunk *)alloc(allocator, sizeof(Chunk));
            first_chunk->elements = (Chunk_Element *)alloc(allocator, chunk_size * sizeof(Chunk_Element));
            chunks_count += 1;
            return 0;
        }
    
        Chunk *chunk = first_chunk;
        for (i32 i = 0; i < chunks_count; i++) {
            if (i > 0) chunk = chunk->next;
        
            if (chunk->occupied_count >= chunk_size) continue;
        
            for (i32 j = 0; j < chunk_size; j++) {
                Chunk_Element *element = &chunk->elements[j];
                if (!element->occupied) {
                    return j + i * chunk_size;
                }
            }
        }
        
        assert(chunk && "We should not set this chunk variable to null, because here it should be our last chunk so we could create next.");
        // If we're here then we did not found any free space in existing chunks, so we creating new chunk.
        
        // Right now "chunk" variable should be last chunk.
        chunk->next = (Chunk *)alloc(allocator, sizeof(Chunk));
        chunk->next->elements = (Chunk_Element *)alloc(allocator, chunk_size * sizeof(Chunk_Element));
        chunks_count += 1;
        
        // So we returning the first index of newly created chunk. If it was second chunk and chunk_size is 32 - we're returning 32.
        return chunks_count * (chunk_size - 1);
    }
    
    T *append(T value, i32 *added_index = NULL) {
        if (chunk_size == 0) chunk_size = 32;
    
        i32 add_index = find_free_space_and_grow_if_need();
        assert(add_index >= 0);
    
        Chunk *chunk = first_chunk;
        for (i32 i = 0; i < chunks_count; i++) {
            if (i > 0) chunk = chunk->next;
            if (index_in_chunk(add_index, chunk, i)) {
                Chunk_Element *chunk_element = &chunk->elements[add_index - (i * chunk_size)];
                
                if (added_index) *added_index = add_index;
                
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
        Chunk *chunk = first_chunk;
        for (i32 i = 0; i < chunks_count; i++) {
            if (i > 0) chunk = chunk->next;
            if (index_in_chunk(index, chunk, i)) {
                Chunk_Element *chunk_element = &chunk->elements[index - (i * chunk_size)];
                
                chunk_element->occupied = false;
                chunk->occupied_count -= 1;
                assert(chunk->occupied_count > 0);
                
                return;
            }
        }
        
        assert(false && "Tried to remove index that is not present in chunk array");
    }
    
    inline b32 contains(T *to_find) {
        return find(to_find) >= 0;
    }
    
    inline b32 contains(T to_find) {
        return contains(&to_find);
    }
    
    i32 find(T *to_find) {    
        for_chunk_array_value(value, T, this) {
            if (*value == *to_find) {
                return i;
            }
        }
        
        return -1;
    }
    inline i32 find(T to_find) {
        return find(&to_find);
    }
    
    void clear() {
        if (!first_chunk) return;
        
        Chunk *chunk = first_chunk;
        for (i32 i = 0; i < chunks_count; i++) {
            if (i > 0) chunk = chunk->next;
            
            for (i32 j = 0; j < chunk_size; j++) {
                Chunk_Element *element = &chunk->elements[j];
                element->occupied = false;
            }
            
            chunk->occupied_count = 0;
        }
    }
    
    void free_data() {
        // Our allocator currently is just arena and we don't free individual elemnts in arena.
        if (allocator || !first_chunk) return;
        
        Chunk *current = first_chunk;
        Chunk *next;
        while (current) {
            next = current->next;
            free(current);
            current = next;
        }
    }
    
    void init_chunk(Chunk *chunk) {
        chunk = (Chunk *)alloc(allocator, sizeof(Chunk));
        chunk->elements = (Chunk_Element *)alloc(allocator, chunk_size * sizeof(Chunk_Element));
    }
};

template <typename T>
void init_chunk_array(Chunk_Array <T> *arr, i32 chunk_size, Allocator *allocator) {
    assert(arr->chunks_count == 0);
    arr->allocator = allocator;
    arr->chunk_size = chunk_size;
    arr->init_chunk(arr->first_chunk);
}

template <typename T>
Chunk_Array <T> copy_chunk_array(Chunk_Array <T> *to_copy) {
    Chunk_Array<T> result = {.allocator = to_copy->allocator};
       
    if (!to_copy->first_chunk) return result;
    
    result.chunk_size = to_copy->chunk_size;
    result.chunks_count = to_copy->chunks_count;
    
    auto *copy_chunk = to_copy->first_chunk;
    auto *my_chunk = result.first_chunk;
    for (i32 i = 0; i < to_copy->chunks_count; i++) {
        result.init_chunk(my_chunk);
        my_chunk->occupied_count = copy_chunk->occupied_count;
        
        for (i32 j = 0; j < to_copy->chunk_size; j++) {
            my_chunk->elements[j] = copy_chunk->elements[j];
        }
             
        my_chunk = my_chunk->next;
        copy_chunk = copy_chunk->next;
    }
}
