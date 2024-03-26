#ifndef HT_H_
#define HT_H_

#ifndef ASM_H_
#   include <stdio.h>
#   include <stdlib.h>
#   include <string.h>
#   include "../cpu/src/cpu.h"
#endif

#include <stdint.h>

typedef long long hshv_t;           // hash table value
typedef unsigned long long hshi_t;  // hash table index

typedef struct {
    hshv_t hash;
    hshi_t index;
    const char *key;
    unsigned int inst;
} hash_item;

typedef struct bucket {
    hash_item hi;
    struct bucket *next;
} Bucket;

typedef struct {
    Bucket *head;
    Bucket *tail;
    size_t count;
} Bucket_List;

#define HT_CAPACITY 55
#define HT_DEBUG_TRUE 1
#define HT_DEBUG_FALSE 0

typedef struct {
    Bucket_List bl[HT_CAPACITY];
    size_t capacity;
} Hash_Table;

Bucket *new_bucket(hash_item hi);
hshv_t hash_function(const char *s);
int ht_get_inst(Hash_Table *ht, const char *s);

void ht_free(Hash_Table *ht);
void ht_push(Hash_Table *ht, hash_item hi);
void inst_ht_init(Hash_Table *ht, int debug);
void make_index(Hash_Table *ht, hash_item *hi);
void buket_push(Bucket_List *bl, Bucket *bucket);

#endif // HT_H_
