#include "arena.h"
#include <stdint.h>

Region *region_create(size_t capacity)
{
    Region *r = malloc(sizeof(Region));
    r->data = malloc(capacity);
    r->capacity = capacity;
    r->alloc_pos = 0;
    r->next = NULL;
    return r;
}

void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment)
{
    size_t align_size = (size + (alignment - 1)) & ~(alignment - 1);
    
    if (arena->head == NULL && arena->tail == NULL) {
        Region *r = region_create(ARENA_CMP(align_size));
        arena->head = r;
        arena->tail = r;
    }

    Region *cur = arena->head;
    while (1) {
        if (cur == NULL) {
            Region* r = region_create(ARENA_CMP(align_size));
            arena->tail->next = r;
            arena->tail = r;
            cur = arena->tail;
        }

        if (cur->alloc_pos + align_size <= cur->capacity) {
            char *ptr = (char*)(cur->data + cur->alloc_pos);
            memset(ptr, 0, align_size);
            cur->alloc_pos += align_size;
            return ptr;
        
        } else {
            cur = cur->next;
            continue;
        }
    }
}

void *arena_alloc(Arena *arena, size_t size)
{
    return arena_alloc_aligned(arena, size, sizeof(void*));
}

void *arena_realloc(Arena *arena, void *old_ptr, size_t old_size, size_t new_size)
{
    if (new_size > old_size) {
        void *new_ptr = arena_alloc(arena, new_size);
        memcpy(new_ptr, old_ptr, old_size);
        return new_ptr;
    } else {
        return old_ptr;
    }
}

void arena_dump(Arena *arena)
{
    size_t i = 1;
    Region *cur = arena->head;
    while (cur != NULL) {
        printf("region %zu: %zu/%zu\n", i, cur->alloc_pos, cur->capacity);
        i += 1;
        cur = cur->next;
    }
}

void arena_reset(Arena *arena)
{
    Region *cur = arena->head;
    while (cur != NULL) {
        cur->alloc_pos = 0;
        cur = cur->next;
    }
}

void arena_free(Arena *arena)
{
    Region *cur = arena->head;
    while (cur != NULL) {
        Region *next = cur->next;
        free(cur->data);
        free(cur);
        cur = next;
    }
}