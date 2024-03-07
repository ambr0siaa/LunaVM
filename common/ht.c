#include "ht.h"

int ht_colisions_handle(hash_table *ht, uint64_t index)
{
    if (ht->hs[index].key != NULL) {
        return 1;
    }
    else return 0;
}

uint64_t make_index(hash_table *ht, uint64_t hash, const char *key, int insert)
{
    uint64_t index = hash;
    if (ht_colisions_handle(ht, index)) {
        if (insert) {
            index = index * 2;
        } else {
            if (strcmp(ht->hs[index].key, key)) {
                index *= 2;
            }
        }
    }
    return index;
}

hash_item create_hash_item(const char *key, ui val)
{
    return (hash_item) {
        .key = key,
        .val = val
    };
}

uint64_t hash_function(const char *str)
{
    str_hash sh = {0};
    size_t len = strlen(str);
    sh.c0 = (uint64_t)str[0];
    sh.c1 = (uint64_t)str[1];

    switch (len) {
        case 4:
            sh.c2 = (uint64_t)str[2];
            sh.c3 = (uint64_t)str[3];
            return (sh.c0 + sh.c1 + sh.c2 + sh.c3) / 2;
        case 3:
            sh.c2 = (uint64_t)str[2];
            return (sh.c0 + sh.c1 + sh.c2) / 2;
        case 2:
            return (sh.c0 + sh.c1) / 2;
        default:
            return -1;
    }
}

void ht_insert(hash_table *ht, ui val, const char *key, uint64_t index, uint64_t hash)
{
    hash_item hi = { .key = key, .val = val };

    if (hash > ht->capacity) {
        ht->capacity *= 2;
        ht->hs = realloc(ht->hs, ht->capacity * sizeof(hash_item));
    }

    if (ht_colisions_handle(ht, index)) {
        fprintf(stderr, "Error: cannot push `%u` by index `%lu`; In there [val = %u, key = %s]\n",
                val, index, ht->hs[index].val, ht->hs[index].key);
        exit(1);
    }

    ht->count++;
    ht->hs[index] = hi;
}

void ht_push(hash_table *ht, const char* key, ui val)
{
    uint64_t hash = hash_function(key);
    uint64_t index = make_index(ht, hash, key, 1);
    ht_insert(ht, val, key, index, hash);
}

ui ht_get(hash_table *ht, const char *key)
{
    ui value;
    uint64_t index = make_index(ht, hash_function(key), key, 0);
    if (ht->hs[index].key == NULL) {
        value = 4040;
    } else {
        value = ht->hs[index].val;
    }
    return value;
}

void ht_init(hash_table *ht)
{
    ht->capacity = HT_INIT_CAPACITY;
    ht->hs = malloc(ht->capacity * sizeof(hash_item));
}

void print_ht(hash_table *ht)
{
    printf("\n--------- Hash table ---------\n\n");
    for (size_t i = 0; i < ht->capacity; ++i) {
        if (ht->hs[i].key != NULL) {
            printf("index: %zu, val: %s, key: %s\n", i, inst_as_cstr(ht->hs[i].val), ht->hs[i].key);
        }
    }
    printf("\n------------------------------\n");
}

void ht_clean(hash_table *ht)
{
    ht->capacity = 0;
    ht->count = 0;
    free(ht->hs);
}

void inst_table_init(hash_table *ht, size_t inst_count)
{
    if (ht->count == 0) ht_init(ht);

    for (Inst i = 0; i < inst_count; ++i) {
        ht_push(ht, inst_as_cstr(i), i);
    }
}
