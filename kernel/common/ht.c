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

void make_index(Hash_Table *ht, hash_item *hi) 
{
    hshi_t index = ((hi->hash ^ 7) % (IC)) ^ 11;
    index += ((hi->hash << 8) & hi->index) ^ 7;
    while (index >= ht->capacity) { index >>= 2; }    
    hi->index = index;
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
    hshv_t hash = hash_function(s);
    hash_item hi = { .hash = hash };
    make_index(ht, &hi);
    if (hi.index <= HT_CAPACITY) {
        Bucket *cur = ht->bl[hi.index].head;
        while (cur != NULL) {
            if (cur->hi.hash == hi.hash)
                return cur->hi.inst;
            cur = cur->next;
        }
    }
    return -1;
}

void inst_ht_init(Hash_Table *ht, int debug)
{
    ht->capacity = HT_CAPACITY;

    for (Inst i = 0; i < IC; ++i) {
        const char *inst = inst_as_cstr(i);
        hshv_t hash = hash_function(inst);
        hash_item hi = { .key = inst, .inst = i , .hash = hash };
        make_index(ht, &hi);
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