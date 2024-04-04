#ifndef VAR_H_
#define VAR_H_

#ifndef PARSER_H_
#   include "../../common/sv.h"
#endif

#include "../../common/ht.h"

typedef enum {
    VAR_TYPE_INT = 0,
    VAR_TYPE_UINT,
    VAR_TYPE_FLOAT,
    VAR_TYPE_ERR
} Varibale_Type;

typedef struct {
    String_View name;
    Varibale_Type type;
    union {
        int64_t as_i64;
        uint64_t as_u64;
        double as_f64;
    };
} Variable_Statement;

typedef struct vt_item {
    Variable_Statement var;
    hshv_t hash;
    struct vt_item *next;
} Vt_Item;

#define VT_CAPACITY 60

typedef struct {
    Vt_Item items[VT_CAPACITY];
    size_t capacity;
} Variable_Table;

void vt_print(Variable_Table *vt);
void var_print(Variable_Statement *var);
void vt_insert(Variable_Table *vt, Variable_Statement var);
Variable_Statement vt_get(Variable_Table *vt, String_View name);
Vt_Item vt_push(Vt_Item *v, Variable_Statement var, hshv_t hash);

#endif // VAR_H_