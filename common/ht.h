#ifndef HT_H_
#define HT_H_

#define ui unsigned int 

#ifndef ASM_H_
#   include <stdio.h>
#   include <stdlib.h>
#   include <string.h>
#   include "../cpu/src/cpu.h"
#endif

#include <stdint.h>

typedef struct {
    ui val;
    const char *key;
} hash_item;

#define HT_INIT_CAPACITY 512

typedef struct {
    hash_item *hs;
    size_t capacity;
    size_t count;
} hash_table;

typedef struct {
    uint64_t c0 : 7;
    uint64_t c1 : 7;
    uint64_t c2 : 7;
    uint64_t c3 : 7;
} str_hash;

extern void print_ht(hash_table *ht);
extern void ht_init(hash_table *ht);

extern uint64_t hash_function(const char *str);
extern uint64_t make_index(hash_table *ht, uint64_t hash, const char *key, int insert);
extern hash_item create_hash_item(const char *key, ui val);

extern void ht_push(hash_table *ht, const char* key, ui val);
extern void ht_insert(hash_table *ht, ui val, const char *key, uint64_t index, uint64_t hash);
extern void inst_table_init(hash_table *ht, size_t inst_count);

extern ui ht_get(hash_table *ht, const char *key);
extern int ht_colisions_handle(hash_table *ht, uint64_t index);

#endif // HT_H_