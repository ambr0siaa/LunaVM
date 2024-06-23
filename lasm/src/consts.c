#include "consts.h"

Const_Statement *cnst_state_create(Arena *a, String_View name, Const_Type type, Const_Value val)
{
    Const_Statement *cnst = arena_alloc(a, sizeof(Const_Statement));
    cnst->name = name;
    cnst->type = type;
    cnst->value = val;
    return cnst;
}

void cnst_print(Const_Statement *cnst)
{
    printf("name: ["SV_Fmt"], type: [%u], value:", SV_Args(cnst->name), cnst->type);
    switch (cnst->type) {
        case CONST_TYPE_INT:
            printf("%"PRIi64"", cnst->value.as_i64);
            break;
        case CONST_TYPE_FLOAT:
            printf("%lf", cnst->value.as_f64);
            break;
        case CONST_TYPE_UINT:
            printf("%"PRIu64"", cnst->value.as_u64);
            break;
        default:
            fprintf(stderr, "Error: unknown type in `ct_print`\n");
            exit(1);
    }
    printf("\n");
}

void ct_init(Arena *a, Const_Table *ct, size_t capacity)
{
    ht_init(a, ct, capacity);
}

void ct_insert(Arena *a, Const_Table *ct, Const_Statement *cnst)
{
    char *s = sv_to_cstr(cnst->name);
    ht_insert(a, ct, s, (void*)cnst);
    free(s);
}

void ct_print(Const_Table *ct)
{
    for (size_t i = 0; i < ct->capacity; ++i) {
        Const_Statement *cnst = (Const_Statement*)ct->items[i]->value;
        if (cnst != NULL)
            cnst_print(cnst);
        cnst = (Const_Statement*)ct->buckets[i]->item->value;
        if (cnst != NULL)
            cnst_print(cnst);
    }
}

Const_Statement *ct_get(Const_Table *ct, String_View name)
{
    Const_Statement *dst;
    char *key = sv_to_cstr(name);
    if (!ht_get(ct, key, (void**)&dst)) {
        dst = NULL;
        fprintf(stderr, "error: cannot get constant by name `"SV_Fmt"`\n", SV_Args(name));
    }
    free(key);
    return dst;
}