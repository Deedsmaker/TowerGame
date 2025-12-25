#pragma once

#include <string.h>
#include "my_defines.hpp"
#include <stdlib.h> // For calloc.

struct Allocator;

enum Allocator_Type {
    DEFAULT_ALLOCATOR = 0,
    ARENA_ALLOCATOR = 1,
    CHUNK_ARENA_ALLOCATOR = 2,
};

struct Arena_Data {
    char *start = NULL;
    char *current = NULL;
    i32 reserved = 0;
    i32 watermark = 0;
};

struct Default_Allocator_Data {
            
};



struct Chunk_Arena_Data {
    struct Chunk : Arena_Data {
        Chunk *next = NULL;
    };
    
    Allocator *allocator = NULL; // That's for allocating chunks itself.
    Chunk first = {0};
    
    i32 chunks_size = 0;
}

struct Allocator {
    Allocator_Type type = DEFAULT_ALLOCATOR;
    
    union {
        Arena_Data arena;
        Default_Allocator_Data default_data;
        Chunk_Arena_Data chunk_data;
    };
};

Allocatror default_allocator = {.type = DEFAULT_ALLOCATOR}; // It would be just malloc.
Allocator temp_allocator    = {0};
Allocator *temp = &temp_allocator;
Allocator state_allocator   = {0};

char *alloc(Allocator *allocator, size_t size) {
    if (!allocator) allocator = default_allocator;

    if (!allocator) return (char *)calloc(1, size);
    
    if (allocator->arena.watermark + size >= allocator->arena.reserved) {
        return NULL;
    }
    
    char *result = allocator->arena.start + allocator->arena.watermark;
    memset(result, 0, size);
    allocator->arena.watermark += size;
    
    return result;
}

bool init_arena_data(Arena_Data *data, size_t size, Allocator *allocator = NULL) {
    if (!allocator) allocator = &default_allocator;

    data->reserved = size;
    data->watermark = 0;
    data->start = alloc(allocator, size), 
    data->current = data->start;
}

Allocator init_allocator(size_t size, Allocator_Type type, Allocator *optional_allocator = NULL) {
    Allocator result = {0};
    result.type = type;

    switch (type) {
        case DEFAULT_ALLOCATOR: {
        } break;
        case ARENA_ALLOCATOR: {
            assert(allocator->arena.reserved <= 0 && allocator->arena.watermark == 0 && "On initing arena - it should be free from all chains");
            
            bool result = init_arena_data(&allocator->arena, size, optional_allocator);
            if (!result) {
                // @TODO: Log error.
            }
        } break;
        case CHUNK_ARENA_ALLOCATOR: {
            Chunk_Arena_Data *chunk_data = &allocator->chunk_data;
            chunk_data->allocator = optional_allocator;
            chunk_data->chunk_size = size;
            
            bool result = init_arena_data(&chunk_data->first, size, optional_allocator);
            if (!result) {
                // @TODO: Log error.
            }
            
                       
        };
    }
}

void clear_allocator(Allocator *allocator) {
    allocator->arena.watermark = 0;
}

void clear_and_push_zeroes_to_allocator(Allocator *allocator) {
    allocator->arena.watermark = 0;
    memset(allocator->arena.start, 0, allocator->arena.reserved);
}

void free_allocator(Allocator *allocator) {
    free(allocator->arena.start);
}
    
inline void free_data_in_allocator(Allocator *allocator, void *data) {
    // Currently allocator is just Memory Arena, so we can't just free data in it.
    // That's why we call default 'free' when allocator is NULL - that's mean it was allocated with just calloc.
    if (!allocator) {
        free(data);
    }
}
