#pragma once

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
};

struct Allocator {
    Allocator_Type type = DEFAULT_ALLOCATOR;
    
    union {
        Arena_Data arena_data;
        Default_Allocator_Data default_data;
        Chunk_Arena_Data chunk_data;
    };
};

Allocator default_allocator = {.type = DEFAULT_ALLOCATOR}; // It would be just malloc.
Allocator temp_allocator    = {.type = ARENA_ALLOCATOR};
Allocator *temp = &temp_allocator;

char *alloc(Allocator *allocator, size_t size);
void set_next_chunks_size(Allocator *allocator, size_t size);
Allocator init_allocator(size_t size, Allocator_Type type, Allocator *optional_allocator = NULL);
void free_allocator(Allocator *allocator);
inline void free_data_in_allocator(Allocator *allocator, void *data);
void clear_allocator(Allocator *allocator, bool zero_data = false);


