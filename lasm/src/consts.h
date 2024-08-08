#ifndef CONSTS_H_
#define CONSTS_H_

#include <inttypes.h>

#include "../../common/sv.h"
#include "../../common/ht.h"

typedef Hash_Table Const_Table;

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

LUNA_API void ct_init(Arena *a, Const_Table *ct, size_t capacity);

LUNA_API Const_Statement *cnst_state_create(Arena *a, String_View name, Const_Type type, Const_Value val);

LUNA_API void ct_print(Const_Table *ct);
LUNA_API void cnst_print(Const_Statement *cnst);

LUNA_API void ct_insert(Arena *a, Const_Table *ct, Const_Statement *cnst);
LUNA_API Const_Statement *ct_get(Const_Table *ct, String_View name);

#endif // CONSTS_H_
