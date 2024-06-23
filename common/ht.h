#ifndef HT_H_
#define HT_H_

#ifndef CPU_H_
# include "../luna/src/luna.h"
#else
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdint.h>
#endif

#define HT_MAGIC    0x428a2f98    // cubic root of 2
#define HT_CAPACITY 69

typedef long long int hshv_t; // hash value
typedef hshv_t (*hash_func_t) (const char *s);

#define MD5_CHUNK_SIZE 64
#define F(x, y, z) ((x & y) | ((~x) & z))
#define G(x, y, z) ((x & z) | ((~z) & y))
#define H(x, y, z) (x ^ y ^ z)
#define I(x, y, z) (y ^ ((~z) | x))

#define rol(x, c) (((x) << (c)) | ((x) >> (32 - (c))))
#define ror(x, c) (((x) >> (c)) | ((x) << (32 - (c))))

struct md5 {
    uint8_t sub_chunk[2][MD5_CHUNK_SIZE];
    uint8_t chunk[MD5_CHUNK_SIZE];
    size_t chunk_count;
    uint32_t h[4];
    int flag;
};

extern const uint32_t md5_shifts[64];
extern const uint32_t md5_keys[64];

// sha256 implementation stolen from https://github.com/leahneukirchen/redo-c/blob/master/redo.c
struct sha256 {
    uint64_t len;
    uint32_t h[8];
    uint8_t buf[64];
};

extern const uint32_t sha256_keys[64];

#define Ch(x,y,z)  (z ^ (x & (y ^ z)))
#define Maj(x,y,z) ((x & y) | (z & (x | y)))
#define S0(x)      (ror(x,2) ^ ror(x,13) ^ ror(x,22))
#define S1(x)      (ror(x,6) ^ ror(x,11) ^ ror(x,25))
#define R0(x)      (ror(x,7) ^ ror(x,18) ^ (x>>3))
#define R1(x)      (ror(x,17) ^ ror(x,19) ^ (x>>10))

void sha256_proc(struct sha256 *s, const uint8_t *buf);

struct ht_item {
    hshv_t hash;
    void *value;
    const char *key;
};

struct bucket {
    struct ht_item *item;
    struct bucket *next;
};

typedef struct Hash_Table Hash_Table;

struct Hash_Table {
    struct ht_item **items;
    size_t item_count;
    size_t capacity;

    struct bucket **buckets;
    size_t bucket_count;

    hash_func_t hfp;    // primary hash function
    hash_func_t hfs;    // secondary hash function

    int overflow_flag;
    int collision_flag;
};

#define ht_overflow(ht)         (ht)->overflow_flag = (ht)->item_count + 1 > (ht)->capacity ? 1 : 0
#define ht_colision(ht, index)  (ht)->collision_flag = (ht)->items[index] != NULL ? 1 : 0
#define ht_index(hash, max)     (hash) % (max)

hshv_t hash_func_primary(const char *str);
hshv_t hash_func_secondary(const char *s);

void ht_summary(Hash_Table *ht);
void ht_init(Arena *a, Hash_Table *ht, size_t capacity);

struct ht_item *ht_item_create(Arena *a, const char *key, void *value, hshv_t hash);

struct bucket **buckets_create(Arena *a, size_t count);
struct bucket *bucket_create(Arena *a, struct ht_item *item);
struct bucket *ht_bucket_search(struct bucket *list, hshv_t hash);
void ht_bucket_push(Arena *a, Hash_Table *ht, struct ht_item *new_item, hshv_t index);

int ht_get(Hash_Table *ht, const char *key, void **dst);
void ht_insert(Arena *a, Hash_Table *ht, const char *key, void *value);

void inst_table_init(Arena *a, Hash_Table *ht, int mode);
int inst_table_get(Hash_Table *ht, const char *key);

#endif // HT_H_
