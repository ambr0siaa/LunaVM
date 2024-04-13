#ifndef CONSTS_H_
#define CONSTS_H_

#ifndef PARSER_H_
#   include "../../common/sv.h"
#endif

#include "../../common/ht.h"

typedef enum {
    CONST_TYPE_INT = 0,
    CONST_TYPE_UINT,
    CONST_TYPE_FLOAT,
    CONST_TYPE_ERR
} Const_Type;

typedef struct {
    String_View name;
    Const_Type type;
    union {
        int64_t as_i64;
        uint64_t as_u64;
        double as_f64;
    };
} Const_Statement;

typedef struct ct_item {
    Const_Statement cnst;
    hshv_t hash;
    struct ct_item *next;
} Ct_Item;

#define CT_CAPACITY 60

typedef struct {
    Ct_Item items[CT_CAPACITY];
    size_t capacity;
} Const_Table;

void ct_print(Const_Table *ct);
void cnst_print(Const_Statement *cnst);
void ct_insert(Const_Table *ct, Const_Statement cnst);
Const_Statement ct_get(Const_Table *ct, String_View name);
Ct_Item ct_push(Ct_Item *ci, Const_Statement cnst, hshv_t hash);

#endif // CONSTS_H_