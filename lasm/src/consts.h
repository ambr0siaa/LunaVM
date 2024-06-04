#ifndef CONSTS_H_
#define CONSTS_H_

#include <inttypes.h>

#ifndef SV_H_
#include "../common/sv.h"
#endif

#ifndef HT_H_
#include "../common/ht.h"
#endif

typedef enum {
    CONST_TYPE_INT = 0,
    CONST_TYPE_FLOAT,
    CONST_TYPE_UINT,
    CONST_TYPE_ERR
} Const_Type;

// TODO: replace by Expr
typedef union {
    int64_t as_i64;
    uint64_t as_u64;
    double as_f64;
} Const_Value;

typedef struct {
    Const_Type type;
    String_View name;
    Const_Value value;
} Const_Statement;

#define CT_CAPACITY 69

typedef Hash_Table Const_Table;
void ct_init(Arena *a, Const_Table *ct, size_t capacity);

void ct_print(Const_Table *ct);
Const_Statement *cnst_state_create(Arena *a, String_View name, Const_Type type, Const_Value val);
void cnst_print(Const_Statement *cnst);
void ct_insert(Arena *a, Const_Table *ct, Const_Statement *cnst);
Const_Statement *ct_get(Const_Table *ct, String_View name);

#endif // CONSTS_H_