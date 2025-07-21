#pragma once

// Right now it's just arena, but we keep possibility of different allocators. 
// It's just sounds more nice in context of default_allocator, where we could assign global arena allocator for time.
// And everyone will be using this default allocator when nothing else is specified.
struct Allocator {
    i32 reserved = 0;
    i32 watermark = 0;
    u8 *start = NULL;
};

// NULL on default allocator means it will be just malloc.
Allocator *default_allocator = NULL;

Allocator temp_allocator    = {};
Allocator level_allocator   = {};
Allocator state_allocator   = {};

void init_allocator(Allocator *allocator, size_t size) {
    assert(allocator->reserved <= 0 && allocator->watermark == 0 && "On initing allocator should be free from all chains");

    allocator->reserved = size;
    allocator->watermark = 0;
    allocator->start = (u8 *)malloc(size);
}

u8 *alloc(size_t size, Allocator *allocator) {
    if (!allocator) allocator = default_allocator;
    if (!allocator) return (u8 *)malloc(size);
    
    assert(allocator->watermark + size < allocator->reserved && "We don't handle situation where memory arena consumed more than it could handle. Alloc more on the start or think about your behaviour.");
    
    u8 *result = allocator->start + allocator->watermark;
    allocator->watermark += size;
    
    return result;
}

void clear_allocator(Allocator *allocator) {
    allocator->watermark = 0;
}

void free_allocator(Allocator *allocator) {
    free(allocator->start);
}

