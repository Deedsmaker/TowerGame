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
    i64 reserved = 0;
    i64 watermark = 0;
};

struct Default_Allocator_Data {
            
};

struct Chunk_Arena_Data {
    struct Chunk : Arena_Data {
        Chunk *next = NULL;
    };
    
    Allocator *allocator = NULL; // That's for allocating chunks itself.
    Chunk first = {0};
    
    i64 default_chunks_size = 0; // Not necessary all chunks size would be that, because we could be asked to allocate more and in that case full chunk would be required size (for example that could happen on dynamic array growth).
}

struct Allocator {
    Allocator_Type type = DEFAULT_ALLOCATOR;
    
    union {
        Arena_Data arena_data;
        Default_Allocator_Data default_data;
        Chunk_Arena_Data chunk_data;
    };
};

Allocatror default_allocator = {.type = DEFAULT_ALLOCATOR}; // It would be just malloc.
Allocator temp_allocator    = {0};
Allocator *temp = &temp_allocator;
Allocator state_allocator   = {0};

void clear_allocator(Allocator *allocator) {
    allocator->arena_data.watermark = 0;
    allocator->arena_data.current = allocator->arena_data.current;
}

void clear_and_push_zeroes_to_allocator(Allocator *allocator) {
    allocator->arena_data.watermark = 0;
    memset(allocator->arena_data.start, 0, allocator->arena_data.reserved);
}

inline void free_data_in_allocator(Allocator *allocator, void *data) {
    if (!allocator) allocator = &default_allocator;

    switch (allocator->type) {
        case DEFAULT_ALLOCATOR: {
            free(data);
        } break;
        default: {
            // Do not free data inside arena allocators.
        } break;
    }
}

void free_allocator(Allocator *allocator) {
    if (!allocator) return;
    
    switch (allocator->type) {
        case DEFAULT_ALLOCATOR: {
            // Do not freeing it. It's just malloc and we can't free malloc even if we really wanted to.
        } break;
        case ARENA_ALLOCATOR: {
            free(allocator->arena_data.start);
        } break;
        case CHUNK_ARENA_ALLOCATOR: {
            auto chunk_data = allocator->chunk_data;
            auto current = &chunk_data.first_chunk;
            free_data_in_allocator(chunk_data->allocator, current->start);
            
            Chunk_Arena_Data::Chunk *next = current->next;
            while (next) {
                current = next;
                next = next->next;
                
                free_data_in_allocator(chunk_data->allocator, current->start);
                free_data_in_allocator(chunk_data->allocator, current);
            }
        } break;
        default: {
            // @TODO: Log unhandled allocator.
        }
    }
}
    
bool init_arena_data(Arena_Data *data, size_t size, Allocator *allocator = NULL) {
    if (!allocator) allocator = &default_allocator;

    data->reserved = size;
    data->watermark = 0;
    data->start = alloc(allocator, size), 
    data->current = data->start;
}

inline char *alloc_arena_data(Arena_Data *data, size_t size) { 
    if (data->watermark + size >= data->reserved) {
        return NULL;
    }
    
    char *result = allocator->arena_data.current;
    memset(result, 0, size);
    data->watermark += size;
    allocator->arena_data.current += size;
    
    return result;
}

char *alloc(Allocator *allocator, size_t size) {
    if (!allocator) allocator = default_allocator;

    switch (allocator->type) {
        case DEFAULT_ALLOCATOR: {
            return (char *)calloc(1, size);            
        } break;
        case ARENA_ALLOCATOR: {
            char *result = alloc_arena_data(&allocator->arena_data, size);
            if (!result) {            
                // @TODO: Log error.
            }
            
            return result;
        } break;
        case CHUNK_ARENA_ALLOCATOR: {
            auto chunk_data = &allocator->chunk_data;
            auto last_chunk = &chunk_data->first;
            while (last_chunk->next) last_chunk = last_chunk->next;
            
            i64 avaliable = last_chunk->reserved - last_chunk->watermark;
            if (avaliable >= size) {
                auto result = alloc_arena_data(last_chunk, size);
                assert(result && "We have checked for avaliable space, but alloc returned NULL"); 
                return result;
            } else {
                // Here we will allocate new chunk and new chunk size would be max(size, chunk_data->default_chunks_size), 
                // because we could ask for far more than chunk allocator itself was set to store (for example on array growth).
                
                Chunk_Arena_Data::Chunk *new_chunk = alloc(chunk_data->allocator, sizeof(Chunk_Arena_Data::Chunk));
                last_chunk->next = new_chunk;
                
                bool success = init_arena_data(new_chunk, max(chunk_data->default_chunks_size, size), chunk_data->allocator);
                if (!success) {
                    // @TODO: Log error.
                }
                
                auto result = alloc_arena_data(new_chunk, size);
                return result;
            }
        } break;
        default: {
            // @TODO: Log unhandled allocator.
        }
    }

    
    return result;
}

void set_next_chunks_size(Allocator *allocator, size_t size) {
    if (allocator->type != CHUNK_ARENA_ALLOCATOR) {    
        log("WARNING: Tried to set next cnunks size on non CHUNK_ARENA_ALLOCATOR.", LOG_WARNING);
        return;
    }
    
    allocator->chunk_data.default_chunks_size = size;
}

Allocator init_allocator(size_t size, Allocator_Type type, Allocator *optional_allocator = NULL) {
    Allocator result = {0};
    result.type = type;

    switch (type) {
        case DEFAULT_ALLOCATOR: {
        } break;
        case ARENA_ALLOCATOR: {
            assert(allocator->arena_data.reserved <= 0 && allocator->arena_data.watermark == 0 && "On initing arena - it should be free from all chains");
            
            bool success = init_arena_data(&allocator->arena, size, optional_allocator);
            if (!success) {
                // @TODO: Log error.
            }
        } break;
        case CHUNK_ARENA_ALLOCATOR: {
            Chunk_Arena_Data *chunk_data = &allocator->chunk_data;
            chunk_data->allocator = optional_allocator;
            chunk_data->default_chunks_size = size;
            
            bool success = init_arena_data(&chunk_data->first, size, optional_allocator);
            if (!success) {
                // @TODO: Log error.
            }
        };
        default: {
            // @TODO: Log unhandled allocator.
        }
    }
}

