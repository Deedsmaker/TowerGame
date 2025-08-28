#pragma once

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "Allocator.cpp"
#include "my_defines.hpp"

#define for_array(index, array) for (i32 index = 0; index < (array)->count; index++)

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
    // That's for situations where we want to reuse element. (like in redo).
    T *increase_count_and_get_last() {
        grow_if_need((void **)(&data), sizeof(T), &capacity, count, 1);
        
        count += 1;
        
        return last();
    }
    
    void append_another_array(Array <T> *array) {
        for_array(i, array) {
            append(array->get_value(i));
        }
    }
    
    T *insert(T value, i32 index) {
        // We could think about growing array if insert index exeeds current capacity, but curently I think we should keep it 
        // simple and allow insert only on existing places.
        assert(index > 0 && index < count);
        
        data[index] = value;
        
        return &data[index];
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
    
    inline void remove_first_encountered(T *value) {
        i32 index = find(value);
        if (index >= 0) remove(index);
    }
    inline void remove_first_encountered(T value) {
        remove_first_encountered(&value);
    }
    
    inline void remove_all_encountered(T *value) {
        for_array(i, this) {
            if (*get(i) == *value) {
                remove(i);
                i--;
            }
        }
    }
    inline void remove_all_encountered(T value) {
        remove_all_encountered(&value);
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
    
    if (capacity > 0) {
        array->data = (T*) alloc(allocator, capacity * sizeof(T));
    }
}

template <typename T>
Array <T> copy_array(Array<T> *to_copy) {
    Array <T> result = {.allocator = to_copy->allocator};
    init_array(&result, to_copy->capacity, to_copy->allocator);
    
    for_array(i, to_copy) {
        result.append(*to_copy->get(i));
    }
    
    return result;
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
    
    inline void remove_first_encountered(T *value) {
        i32 index = find(value);
        if (index >= 0) remove(index);
    }
    inline void remove_first_encountered(T value) {
        remove_first_encountered(&value);
    }
    
    inline void remove_all_encountered(T *value) {
        for_array(i, this) {
            if (*get(i) == *value) {
                remove(i);
                i--;
            }
        }
    }
    inline void remove_all_encountered(T value) {
        remove_all_encountered(&value);
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

#define for_chunk_array_value(chunk_value, type, arr) type *chunk_value = NULL; for (i32 i = (arr)->next_occupied_value(0, &chunk_value); i < (arr)->chunks_count * (arr)->chunk_size && chunk_value; i = (arr)->next_occupied_value(i + 1, &chunk_value)) 

#define for_chunk_array(index, arr) for (i32 index = (arr)->next_avaliable_index(0, true); index < (arr)->chunks_count * (arr)->chunk_size; index = (arr)->next_avaliable_index(index + 1, true)) 

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
    
    // should_be_occupied here because sometimes we want to find avaliable space and sometime we want to find occupied space
    // so that's just for code reusing and it stands by the name of next_*avaliable*_value just because it was written first
    // before I thought about that and that's not scary because that comment is exist.
    inline i32 next_avaliable_value(i32 start_index, T **element, b32 should_be_occupied = false) {
        i32 index = next_avaliable_index(start_index, should_be_occupied);
        if (index < chunks_count * chunk_size) *element = get(index);
        else *element = NULL; 
        
        return index;
    }
    inline i32 next_occupied_value(i32 start_index, T **element) {
        return next_avaliable_value(start_index, element, true);
    }
    
    inline i32 next_avaliable_index(i32 start_index, b32 should_be_occupied = false) {
        i32 start_chunk_index = start_index / chunk_size;
        
        // That happens where start_index is equal to total possible count of chunk array.
        // In for_chunk_array that will happen if chunk array is heavily populated and we're asking for a next index giving 
        // (count - 1) + 1 trying to get next index. Here we should return default fail value, which is total possible count 
        // of elements.
        if (start_chunk_index == chunks_count) {
            return chunks_count * chunk_size;            
        }
        
        assert(start_chunk_index < chunks_count);
        
        if (!first_chunk) {
            init_chunk(&first_chunk);  
            chunks_count += 1;
            return 0;
        } 
        
        Chunk *chunk = first_chunk;
        for (i32 i = 0; i < start_chunk_index; i++) chunk = chunk->next;
        
        i32 start_index_in_chunk = start_index - start_chunk_index * chunk_size;
        for (i32 i = start_chunk_index; i < chunks_count; i++) {
            for (i32 j = i == start_chunk_index ? start_index_in_chunk : 0; j < chunk_size; j++) {
                Chunk_Element *array_element = &chunk->elements[j];
                b32 should_return = should_be_occupied ? array_element->occupied : !array_element->occupied;
                if (should_return) {
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
        i32 chunk_index = index / chunk_size;
        assert(chunk_index < chunks_count);
        for (i32 i = 0; i < chunk_index; i++) chunk = chunk->next;
        
        assert(index_in_chunk(index, chunk, chunk_index));
        assert(chunk->elements[index - (chunk_index * chunk_size)].occupied);
        return &chunk->elements[index - (chunk_index * chunk_size)].value;
    }
    
    inline T get_value(i32 index) {
        return *get(index);
    }
    
    b32 is_index_avaliable_for_new_value(i32 index) {
        Chunk *chunk = first_chunk;
        i32 chunk_index = index / chunk_size;
        if (chunk_index >= chunks_count) return false;
        
        for (i32 i = 0; i < chunk_index; i++) chunk = chunk->next;
        
        assert(index_in_chunk(index, chunk, chunk_index));
        return !chunk->elements[index - (chunk_index * chunk_size)].occupied;
    }
    
    i32 find_free_space_and_grow_if_need() {
        if (!first_chunk) {
            init_chunk(&first_chunk);
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
        
        // We should not set this chunk variable to null, because here it should be our last chunk so we could create next.
        assert(chunk && "Chunk array growing bug!");
        // If we're here then we did not found any free space in existing chunks, so we creating new chunk.
        
        // Right now "chunk" variable should be last chunk.
        chunk->next = (Chunk *)alloc(allocator, sizeof(Chunk));
        chunk->next->elements = (Chunk_Element *)alloc(allocator, chunk_size * sizeof(Chunk_Element));
        chunks_count += 1;
        
        // So we returning the first index of newly created chunk. If it was second chunk and chunk_size is 32 - we're returning 32.
        // return chunks_count * (chunk_size - 1);
        assert(chunks_count > 1);
        return chunk_size * (chunks_count - 1);
    }
    
    T *append(T value, i32 *added_index = NULL) {
        if (chunk_size == 0) chunk_size = 32;
    
        i32 append_index = find_free_space_and_grow_if_need();
        assert(append_index >= 0);
    
        Chunk *chunk = first_chunk;
        i32 chunk_index = append_index / chunk_size;
        assert(chunk_index < chunks_count);
        for (i32 i = 0; i < chunk_index; i++) chunk = chunk->next;
        
        assert(index_in_chunk(append_index, chunk, chunk_index) && "Chunk array append bug!");
        Chunk_Element *chunk_element = &chunk->elements[append_index - (chunk_index * chunk_size)];
            
        if (added_index) *added_index = append_index;
        
        assert(!chunk_element->occupied && "Chunk array append occupied bug!");
        chunk_element->occupied = true;
        chunk_element->value = value;
        chunk->occupied_count += 1;
        assert(chunk->occupied_count <= chunk_size);
        return &chunk_element->value;
    }
    
    T *insert(T value, i32 index) {
        assert(index >= 0 && index < chunks_count * chunk_size);
        
        Chunk *chunk = first_chunk;
        i32 chunk_index = index / chunk_size;
        assert(chunk_index < chunks_count);
        for (i32 i = 0; i < chunk_index; i++) chunk = chunk->next;
        
        assert(index_in_chunk(index, chunk, chunk_index) && "Chunk array insert bug!");
        Chunk_Element *chunk_element = &chunk->elements[index - (chunk_index * chunk_size)];
            
        // We're not asserting occupied just because I'm assuming that we're know what we're doing. 
        // It's used while copying level context entities array and then re-inserting deep copy of an entity in 
        // the same place. I'll think more about do we really need this assert or we just should change logic in 
        // one place where we're asserting. (started thinking 17.08.2025).
        // assert(!chunk_element->occupied);
        
        if (chunk_element->occupied) {
            
        } else {
            chunk_element->occupied = true;
            chunk->occupied_count += 1;
        }
        
        chunk_element->value = value;
        
        assert(chunk->occupied_count <= chunk_size);
        return &chunk_element->value;
    }
    
    void remove(i32 index) {
        Chunk *chunk = first_chunk;
        i32 chunk_index = index / chunk_size;
        assert(chunk_index < chunks_count);
        for (i32 i = 0; i < chunk_index; i++) chunk = chunk->next;

        assert(index_in_chunk(index, chunk, chunk_index));
        Chunk_Element *chunk_element = &chunk->elements[index - (chunk_index * chunk_size)];
        
        assert(chunk_element->occupied);
        
        chunk_element->occupied = false;
        chunk->occupied_count -= 1;
        assert(chunk->occupied_count >= 0);
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
            free_data_in_allocator(allocator, current);
            current->occupied_count = 0;
            current = next;
        }
        
        // Should we tell that chunks count is zero and actually nullify all of chunks?
    }
    
    inline void init_chunk(Chunk **chunk) {
        *chunk = (Chunk *)alloc(allocator, sizeof(Chunk));
        (*chunk)->elements = (Chunk_Element *)alloc(allocator, chunk_size * sizeof(Chunk_Element));
    }
};

template <typename T>
void init_chunk_array(Chunk_Array <T> *arr, i32 chunk_size, Allocator *allocator) {
    assert(arr->chunks_count == 0);
    arr->allocator = allocator;
    arr->chunk_size = chunk_size;
    arr->init_chunk(&arr->first_chunk);
    arr->chunks_count += 1;    
}

template <typename T>
Chunk_Array <T> copy_chunk_array(Chunk_Array <T> *to_copy) {
    Chunk_Array<T> result = {.allocator = to_copy->allocator};
       
    if (!to_copy->first_chunk) return result;
    
    result.chunk_size = to_copy->chunk_size;
    result.chunks_count = to_copy->chunks_count;
    
    auto *copy_chunk = to_copy->first_chunk;
    auto **my_chunk = &result.first_chunk;
    for (i32 i = 0; i < to_copy->chunks_count; i++) {
        result.init_chunk(my_chunk);
        (*my_chunk)->occupied_count = copy_chunk->occupied_count;
        
        for (i32 j = 0; j < to_copy->chunk_size; j++) {
            (*my_chunk)->elements[j] = copy_chunk->elements[j];
        }
             
        my_chunk = &(*my_chunk)->next;
        copy_chunk = copy_chunk->next;
    }
    
    return result;
}
