#include "ht.h"

hshv_t hash_function(const char *s) 
{
    const int n = strlen(s);
    const int p = 31;
    const int m = 1e9 + 7;
    hshv_t hash = 0;
    hshv_t p_pow = 1;
    for (int i = 0; i < n; ++i) {
        hash = (hash + (s[i] - 'a' + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return hash;
}

hshi_t make_index(size_t capacity, hshv_t hash) 
{
    hshi_t index = ((hash ^ 7) % (34)) ^ 11;
    index += (((hash << 8) & 0) ^ 7);
    while (index > capacity) { index >>= 2; }    
    return index;
}

Bucket *new_bucket(hash_item hi)
{
    Bucket *bucket = malloc(sizeof(Bucket));
    bucket->hi = hi;
    bucket->next = NULL;
    return bucket;
}

void buket_push(Bucket_List *bl, Bucket *bucket)
{
    if (bl->tail != NULL && bl->head != NULL) {
        bl->tail->next = bucket;
        bl->tail = bucket;
        bl->count += 1;
    } else {
        bl->head = bucket;
        bl->tail = bucket;
        bucket->next = NULL;
        bl->count = 1;
    }
}

void ht_push(Hash_Table *ht, hash_item hi)
{
    Bucket *bucket = new_bucket(hi);
    buket_push(&ht->bl[hi.index], bucket);
}

int ht_get_inst(Hash_Table *ht, const char *s)
{
    hash_item hi = {0};
    ht_get(ht, s, &hi);
    if (hi.hash == -1) {
        return hi.hash;
    } else {
        return hi.inst;
    }
}

void ht_get(Hash_Table *ht, const char *key, hash_item *dst)
{
    hshv_t hash = hash_function(key);
    hash_item hi = { .hash = hash };
    hi.index = make_index(ht->capacity, hi.hash);
    if (hi.index <= HT_CAPACITY) {
        Bucket *cur = ht->bl[hi.index].head;
        while (cur != NULL) {
            if (cur->hi.hash == hi.hash) {
                *dst = cur->hi;
                return;
            }
            cur = cur->next;
        }
    }
    dst->hash = -1;
}

void inst_ht_init(Hash_Table *ht, int debug)
{
    ht->capacity = HT_CAPACITY;

    for (Inst i = 0; i < IC; ++i) {
        const char *inst = inst_as_cstr(i);
        hshv_t hash = hash_function(inst);
        hash_item hi = { .key = inst, .inst = i , .hash = hash };
        hi.index = make_index(ht->capacity, hi.hash);
        ht_push(ht, hi);
        if (debug == HT_DEBUG_TRUE) {
            printf("inst: [%s]; hash: [%lli], index: [%llu]\n", inst, hash, hi.index);
        }
    }
}

void ht_free(Hash_Table *ht)
{
    for (size_t i = 0; i < ht->capacity; ++i) {
        if (ht->bl[i].head != NULL) {
            Bucket *cur = ht->bl[i].head;
            while (cur != NULL) {
                Bucket *copy = cur;
                if (cur->next == NULL) {
                    free(copy);
                    break;
                }
                cur = cur->next;
                free(copy);
            } 
        }
    }
}

void bucket_list_print(Bucket_List *bl)
{
    Bucket *cur = bl->head;
    while (cur != NULL) {
        printf("hash: [%lli], index: [%llu], key: [%s], inst: [%u]\n", 
               cur->hi.hash, cur->hi.index, cur->hi.key, cur->hi.inst);
        cur = cur->next;
    }
}

void ht_print(Hash_Table *ht)
{
    for (size_t i = 0; i < ht->capacity; ++i) {
        if (ht->bl[i].count != 0) {
            printf("Item index: %zu\n", i);
            bucket_list_print(&ht->bl[i]);
        }
    }
    printf("\n");
}