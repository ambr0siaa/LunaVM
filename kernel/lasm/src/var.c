#include "../include/var.h"

// TODO global: #1 rename all var's to `constant`
//              #2 remake all this file
//              #3 parse expresions with constant (reminder: `&some_constant`)

void var_print(Variable_Statement *var)
{
    printf("name: ["SV_Fmt"], type: [%u], value:", SV_Args(var->name), var->type);
    switch (var->type) {
        case VAR_TYPE_INT:
            printf("%li", var->as_i64);
            break;
        case VAR_TYPE_FLOAT:
            printf("%lf", var->as_f64);
            break;
        case VAR_TYPE_UINT:
            printf("%lu", var->as_u64);
            break;
        default:
            fprintf(stderr, "Error: unknown type in `var_print`\n");
            exit(1);
    }
    printf("\n");
}

Vt_Item vt_push(Vt_Item *v, Variable_Statement var, hshv_t hash)
{
    Vt_Item new = {0};
    new.var = var;
    new.hash = hash;
    if (v->hash == 0) {
        new.next = NULL;
    } else {
        new.next = v;
    }
    return new;
}

void vt_insert(Variable_Table *vt, Variable_Statement var)
{
    char *s = sv_to_cstr(var.name);
    hshv_t hash = hash_function(s);
    hshi_t index = make_index(vt->capacity, hash);
    vt->items[index] = vt_push(&vt->items[index], var, hash);
}

void vt_print(Variable_Table *vt)
{
    printf("\n----------------- Variable Table -----------------\n\n");
    for (size_t i = 0; i < vt->capacity; ++i) {
        if (vt->items[i].hash != 0) {
            Vt_Item cur = vt->items[i];
            do {
                printf("hash: [%lli], index: [%zu], ", cur.hash, i);
                var_print(&cur.var);
                if (cur.next == NULL) break;
                cur = *cur.next;
            } while (cur.next != NULL);
        }
    }
    printf("\n-------------------------------------------------\n\n");
}

Variable_Statement vt_get(Variable_Table *vt, String_View name)
{
    const char *s = sv_to_cstr(name);
    hshv_t hash = hash_function(s);
    hshi_t index = make_index(vt->capacity, hash);
    Vt_Item item = vt->items[index];
    do {
        if (item.hash == hash) 
            return item.var;
        if (item.next == NULL) 
            break;
        item = *item.next;
    } while(1);
    return (Variable_Statement) { .type = VAR_TYPE_ERR };
}