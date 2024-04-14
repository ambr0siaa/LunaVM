#ifndef ARENA_H_
#define ARENA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_DEFAULT_CAPACITY (8 * 1024)
#define ARENA_CMP(size) ((size) < ARENA_DEFAULT_CAPACITY ? ARENA_DEFAULT_CAPACITY : (size))
#define ARENA_ALIGN(size, alignment) ((((size) - 1) / (alignment) + 1) * (alignment))

typedef struct Region Region;

struct Region {
    struct Region *next;
    size_t alloc_pos;
    size_t capacity;
    char data[];
};

Region *region_create(size_t capacity);

typedef struct {
    Region *head;
    Region *tail;
} Arena;

void arena_dump(Arena *arena);
void arena_free(Arena *arena);
void arena_reset(Arena *arena);

void *arena_alloc(Arena *arena, size_t size);
void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment);
void *arena_realloc(Arena *arena, void *old_ptr, size_t old_size, size_t new_size);

#endif // ARENA_H_