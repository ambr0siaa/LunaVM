#ifndef TABLE_H_
#define TABLE_H_

#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

#include "sv.h"
typedef unsigned long hash_t;

#define TABLE_SCALER 3

typedef struct {
    String_View key;
    size_t value;
    hash_t hash;
} Table_Item;

typedef struct {
    size_t count;
    size_t scaler;
    size_t capacity;
    Table_Item *items;
} Table;

hash_t hash_string(char *s, size_t len);

#define index_step(idx)     ((size_t)(sqrt((double)(idx))*1e6))
#define table_overflow(t)   (((t)->count + 1) >= (3 * (t)->capacity / 4))

#define table_resize(t, n)  \
    do {                    \
        table_clean((n));   \
        (t)->scaler *= 2;   \
        goto overflow;      \
    } while(0)

#define table_init(t, c)                                    \
    do {                                                    \
        size_t size = (c)*sizeof(*(t)->items);              \
        (t)->items = malloc(size);                          \
        memset((t)->items, 0, size);                        \
        if ((t)->items) {                                   \
            (t)->capacity = (c);                            \
            (t)->scaler = 1;                                \
            (t)->count = 0;                                 \
        }                                                   \
    } while(0)

#define table_clean(t)      \
    do {                    \
        free((t)->items);   \
        (t)->items = NULL;  \
        (t)->capacity = 0;  \
        (t)->scaler = 0;    \
        (t)->count = 0;     \
    } while(0)

#define table_insert_item(t, item, index)                                   \
    do {                                                                    \
        inserted = false;                                                   \
        if ((t)->items[(index)].hash == 0) {                                \
            inserted = true;                                                \
            (t)->items[(index)] = (item);                                   \
            (t)->count += 1;                                                \
        } else {                                                            \
            for (size_t j = 0; j < (t)->capacity; ++j) {                    \
                (index) = ((index) + index_step((index))) % (t)->capacity;  \
                if ((t)->items[(index)].hash == 0) {                        \
                    (t)->items[(index)] = (item);                           \
                    (t)->count += 1;                                        \
                    inserted = true;                                        \
                    break;                                                  \
                }                                                           \
            }                                                               \
        }                                                                   \
    } while(0)

#define table_insert(t, item, type)                                             \
    do {                                                                        \
        bool inserted = false;                                                  \
        size_t index = (item).hash % (t)->capacity;                             \
        if (table_overflow((t))) goto overflow;                                 \
        table_insert_item((t), (item), index);                                  \
        if (!inserted) {                                                        \
        overflow:                                                               \
            type new = {0};                                                     \
            (t)->scaler <<= TABLE_SCALER;                                       \
            table_init(&new, (t)->capacity * (t)->scaler);                      \
            new.scaler = (t)->scaler;                                           \
            if (!new.items) {                                                   \
                (t)->capacity = (t)->count * 4;                                 \
                goto overflow;                                                  \
            }                                                                   \
            for (size_t i = 0; i < (t)->capacity; ++i) {                        \
                if ((t)->items[i].hash != 0) {                                  \
                    index = (t)->items[i].hash % new.capacity;                  \
                    table_insert_item(&new, (t)->items[i], index);              \
                    if (!inserted) table_resize((t), &new);                     \
                }                                                               \
            }                                                                   \
            index = (item).hash % new.capacity;                                 \
            table_insert_item(&new, (item), index);                             \
            if (!inserted) table_resize((t), &new);                             \
            table_clean((t));                                                   \
            *(t) = new;                                                         \
        }                                                                       \
    } while (0)

#define table_get(t, dst)                                           \
    do {                                                            \
        size_t index = (dst).hash % (t)->capacity;                  \
        if ((t)->items[index].hash == (dst).hash) {                 \
            (dst) = (t)->items[index];                              \
        } else {                                                    \
            for (size_t i = 0; i < (t)->capacity; ++i) {            \
                index = (index + index_step(index)) % (t)->capacity;\
                if ((t)->items[index].hash == (dst).hash) {         \
                    (dst) = (t)->items[index];                      \
                    break;                                          \
                }                                                   \
            }                                                       \
        }                                                           \
    } while(0)

#endif /* TABLE_H_ */
