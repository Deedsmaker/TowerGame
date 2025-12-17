#pragma once

#include <string.h>
#include "my_defines.hpp"
#include <stdlib.h> // For calloc.

// Right now it's just arena, but we keep possibility of different allocators. 
// It's just sounds more nice in context of default_allocator, where we could assign global arena arena for time.
// And everyone will be using this default arena when nothing else is specified.
struct Memory_Arena {
    char *start = NULL;
    i32 reserved = 0;
    i32 watermark = 0;
};

#define HEAP_ALLOCATOR NULL

// NULL on default arena means it will be just malloc.
Memory_Arena temp_arena    = {0};
Memory_Arena *temp = &temp_arena;
Memory_Arena state_arena   = {0};

void init_memory_arena(Memory_Arena *arena, size_t size) {
    assert(arena->reserved <= 0 && arena->watermark == 0 && "On initing arena should be free from all chains");

    arena->reserved = size;
    arena->watermark = 0;
    arena->start = (char *)calloc(1, size);
}

char *alloc(Memory_Arena *arena, size_t size) {
    if (!arena) return (char *)calloc(1, size);
    
    assert(arena->watermark + size < arena->reserved && "We don't handle situation where memory arena consumed more than it could handle. Alloc more on the start or think about your behaviour.");
    
    char *result = arena->start + arena->watermark;
    memset(result, 0, size);
    arena->watermark += size;
    
    return result;
}

void clear_memory_arena(Memory_Arena *arena) {
    arena->watermark = 0;
}

void clear_and_push_zeroes_to_memory_arena(Memory_Arena *arena) {
    arena->watermark = 0;
    memset(arena->start, 0, arena->reserved);
}

void free_memory_arena(Memory_Arena *arena) {
    free(arena->start);
}
    
inline void free_data_in_memory_arena(Memory_Arena *arena, void *data) {
    // Currently arena is just Memory Arena, so we can't just free data in it.
    // That's why we call default 'free' when arena is NULL - that's mean it was allocated with just calloc.
    if (!arena) {
        free(data);
    }
}
