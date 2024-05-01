#include "../include/consts.h"

// TODO global: #1 rename all var's to `constant`
//              #2 remake all this file
//              #3 parse expresions with constant (reminder: `&some_constant`)

void cnst_print(Const_Statement *cnst)
{
    printf("name: ["SV_Fmt"], type: [%u], value:", SV_Args(cnst->name), cnst->type);
    switch (cnst->type) {
        case CONST_TYPE_INT:
            printf("%li", cnst->as_i64);
            break;
        case CONST_TYPE_FLOAT:
            printf("%lf", cnst->as_f64);
            break;
        case CONST_TYPE_UINT:
            printf("%lu", cnst->as_u64);
            break;
        default:
            fprintf(stderr, "Error: unknown type in `ct_print`\n");
            exit(1);
    }
    printf("\n");
}

Ct_Item ct_push(Ct_Item *ci, Const_Statement cnst, hshv_t hash)
{
    Ct_Item new = {0};
    new.cnst = cnst;
    new.hash = hash;
    if (ci->hash == 0) {
        new.next = NULL;
    } else {
        new.next = ci;
    }
    return new;
}

void ct_insert(Const_Table *ct, Const_Statement cnst)
{
    char *s = sv_to_cstr(cnst.name);
    hshv_t hash = hash_function(s);
    hshi_t index = make_index(ct->capacity, hash);
    ct->items[index] = ct_push(&ct->items[index], cnst, hash);
    free(s);
}

void ct_print(Const_Table *ct)
{
    printf("\n----------------- Const Table -----------------\n\n");
    for (size_t i = 0; i < ct->capacity; ++i) {
        if (ct->items[i].hash != 0) {
            Ct_Item cur = ct->items[i];
            do {
                printf("hash: [%lli], index: [%zu], ", cur.hash, i);
                cnst_print(&cur.cnst);
                if (cur.next == NULL) break;
                cur = *cur.next;
            } while (cur.next != NULL);
        }
    }
    printf("\n-------------------------------------------------\n\n");
}

Const_Statement ct_get(Const_Table *ct, String_View name)
{
    char *s = sv_to_cstr(name);
    hshv_t hash = hash_function(s);
    hshi_t index = make_index(ct->capacity, hash);
    free(s);
    Ct_Item item = ct->items[index];
    do {
        if (item.hash == hash) 
            return item.cnst;
        if (item.next == NULL) 
            break;
        item = *item.next;
    } while(1);
    return (Const_Statement) { .type = CONST_TYPE_ERR };
}