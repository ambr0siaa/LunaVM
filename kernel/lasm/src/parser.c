#include "../include/parser.h"

void ll_append(Label_List *ll, Label label)
{
    if (ll->capacity == 0) {
        ll->capacity = INIT_CAPACITY;
        ll->labels = malloc(sizeof(ll->labels[0]) * ll->capacity);
    }

    if (ll->count + 1 > ll->capacity) {
        ll->capacity *= 2;
        ll->labels = realloc(ll->labels, ll->capacity);
    }

    ll->labels[ll->count++] = label;
}

Label ll_search_label(Label_List *ll, String_View name)
{
    for (size_t i = 0; i < ll->count; ++i) {
        Label label = ll->labels[i];
        if (sv_cmp(label.name, name)) {
            return label;
        }
    }
    return (Label) {
        .addr = -1
    };
}

Token parse_val(Lexer *lex)
{
    Eval eval = {0};
    parse_arefmetic_expr(&eval, lex);
    eval_tree(&eval);
    Token tk = eval.root->token; 
    eval_clean(eval.root, &eval.count);
    return tk;
}

void objb_push(Object_Block *objb, Object obj) { da_append(objb, obj); }
void objb_clean(Object_Block *objb) { da_clean(objb); }
void block_chain_push(Block_Chain *block_chain, Object_Block objb) { da_append(block_chain, objb); }

void block_chain_clean(Block_Chain *block_chain)
{
    for (size_t i = 0; i < block_chain->count; ++i) {
        objb_clean(&block_chain->items[i]);
    }
    da_clean(block_chain);
}

void parse_kind_reg_reg(Inst_Addr *inst_pointer, Lexer *lex, Object_Block *objb)
{
    for (Token tk = lex->items[lex->tp + 1]; 
        tk.type != TYPE_NONE;
        tk = token_next(lex)) {

        if (tk.type == TYPE_COMMA) continue;
        else if (tk.type == TYPE_TEXT) {
            Register reg = parse_register(tk.txt);
            objb_push(objb, OBJ_REG(reg));
        } else {
            // TODO: better errors
            fprintf(stderr, "Error: in `parse_kind_reg_reg` cannot parse register\n");
            exit(1);
        }
    }
    *inst_pointer += 3;
}

void parse_kind_reg_val(Inst_Addr *inst_pointer, Lexer *lex, Object_Block *objb)
{
    for (Token tk = lex->items[lex->tp + 1]; 
        tk.type != TYPE_NONE;
        tk = token_next(lex)) {

        if (tk.type == TYPE_COMMA) continue;
        else if (tk.type == TYPE_TEXT) {
            Register reg = parse_register(tk.txt);
            objb_push(objb, OBJ_REG(reg));

        } else if (tk.type == TYPE_VALUE || tk.type == TYPE_OPEN_BRACKET) {
            Object val_obj;
            token_back(lex, 1);
            Token val_tk = parse_val(lex);
            token_next(lex);

            if (val_tk.val.type == VAL_FLOAT) {
                val_obj = OBJ_FLOAT(val_tk.val.f64);
            } else {
                val_obj = OBJ_INT(val_tk.val.i64);
            }

            objb_push(objb, val_obj);

        } else if (tk.type == TYPE_DOLLAR) {
            tk = token_next(lex);
            int64_t frame_shift = tk.val.i64 + STACK_FRAME_SIZE;
            objb_push(objb, OBJ_INT(frame_shift));

        } else {
            // TODO: better erros
            assert(0 && "TODO: in `parse_kind_reg_val` implement condition");
        }
    }
    *inst_pointer += 3;
}

void parse_kind_reg(Inst_Addr *inst_pointer, Lexer *lex, Object_Block *objb)
{
    Token tk = token_next(lex);
    if (tk.type == TYPE_TEXT) {
        Register reg = parse_register(tk.txt);
        objb_push(objb, OBJ_REG(reg));

    } else {
        // TODO: better errors
        fprintf(stderr, "Error: in `parse_kind_reg`\n");
        exit(1);
    }
    *inst_pointer += 2;
}

void ll_print(Label_List *ll)
{
    printf("\n--------- Label List -----------\n\n");
    for (size_t i = 0; i < ll->count; ++i) {
        Label label = ll->labels[i];
        printf("name: {"SV_Fmt"}; addr: {%lu}\n", SV_Args(label.name), label.addr);
    }
    printf("\n--------------------------------\n\n");
}

void parse_kind_val(Inst inst, 
                    Inst_Addr *inst_pointer,
                    Lexer *lex, 
                    Object_Block *objb, 
                    Program_Jumps *PJ,
                    size_t line_num)
{
    switch (inst) {
        case INST_PUSH_V: {
            Object val_obj;
            Token val_tk = parse_val(lex);
            token_next(lex);
            if (val_tk.val.type == VAL_FLOAT) {
                val_obj = OBJ_FLOAT(val_tk.val.f64);
            } else {
                val_obj = OBJ_INT(val_tk.val.i64);
            }
            objb_push(objb, val_obj);
        }
        break;

        case INST_JZ:
        case INST_JNZ:
        case INST_JMP:
        case INST_CALL:
            Object addr_obj;
            Token tk = token_next(lex);
            if (tk.type == TYPE_VALUE) {
                if (tk.val.type == VAL_INT && tk.val.i64 >= 0) {
                    addr_obj = OBJ_UINT(tk.val.i64);
                    objb_push(objb, addr_obj);
                } else {
                    fprintf(stderr, "Error: program jumps have to be integer and > 0\n");
                    exit(1);
                }
            } else if (tk.type == TYPE_TEXT) {
                Label label = ll_search_label(&PJ->current, tk.txt);
                if (label.addr == (uint64_t)(-1)) {
                    Label dlabel = {
                        .name = tk.txt,
                        .addr = line_num
                    };
                    ll_append(&PJ->deferred, dlabel);
                    addr_obj = OBJ_UINT(dlabel.addr);
                    objb_push(objb, addr_obj);

                } else {
                    addr_obj = OBJ_UINT(label.addr);
                    objb_push(objb, addr_obj);
                }

            } else {
                fprintf(stderr, "Error: inst `%s` expects label or addr\n", inst_as_cstr(inst));
                exit(1);
            }
            break;

        default: {
            fprintf(stderr, "Error: inst `%s` unsupported program jumps or stack operations\n", inst_as_cstr(inst));
            exit(1);
        }
    }
    *inst_pointer += 2;
}

Inst parse_inst(Lexer *lex, Hash_Table *ht)
{
    Token tk = token_next(lex);
    int inst = translate_inst(tk.txt, ht);
    if (inst == -1) {
        fprintf(stderr, "Error: unknown inst\n");
        exit(1);
    } else {
        return (Inst)(inst);
    }
}

Inst convert_to_cpu_inst(Inst inst, Inst_Kind *inst_kind, Lexer *lex)
{
    Inst new_inst;
    Inst_Kind kind;

    Token tk = token_get(lex, 0, SKIP_FALSE);
    if (tk.type == TYPE_NONE) {
        kind = KIND_NONE;
    } else {
        Token copy = tk;
        tk = token_get(lex, 1, SKIP_FALSE);
        if (tk.type == TYPE_COMMA) {
            tk = token_get(lex, 2, SKIP_FALSE);
            if (tk.type == TYPE_VALUE || tk.type == TYPE_OPEN_BRACKET || tk.type == TYPE_DOLLAR) {
                kind = KIND_REG_VAL;
            } else {
                kind = KIND_REG_REG;
            }
        } else {
            if ((copy.type == TYPE_OPEN_BRACKET || copy.type == TYPE_VALUE) ||
                inst == INST_JMP ||
                inst == INST_JNZ ||
                inst == INST_JZ ||
                inst == INST_CALL) {
                kind = KIND_VAL;
            } else {
                kind = KIND_REG;
            }
        }
    }

    *inst_kind = kind;

    switch (kind) {
        case KIND_REG_REG: {
            switch (inst) {
                case INST_ADD:
                    new_inst = INST_ADD_RR;
                    break;

                case INST_SUB:
                    new_inst = INST_SUB_RR;
                    break;

                case INST_DIV:
                    new_inst = INST_DIV_RR;
                    break;

                case INST_MUL:
                    new_inst = INST_MUL_RR;
                    break;

                case INST_MOV:
                    new_inst = INST_MOV_RR;
                    break;

                case INST_CMP:
                    new_inst = inst;
                    break;

                case INST_RET:
                    new_inst = INST_RET_RR;
                    break; 

                default:
                    fprintf(stderr, "inst `%s` hasn't got kind `REG_REG`\n", inst_as_cstr(inst));
                    exit(1);
            }
        }
        break;

        case KIND_REG_VAL: {
            switch (inst) {
                case INST_ADD:
                    new_inst = INST_ADD_RV;
                    break;

                case INST_SUB:
                    new_inst = INST_SUB_RV;
                    break;

                case INST_DIV:
                    new_inst = INST_DIV_RV;
                    break;

                case INST_MUL:
                    new_inst = INST_MUL_RV;
                    break;
                    
                case INST_MOV:
                    tk = token_get(lex, 2, SKIP_FALSE);
                    if (tk.type == TYPE_DOLLAR) {
                        new_inst = INST_MOVS;
                    } else {
                        new_inst = INST_MOV_RV;
                    }
                    break;

                case INST_CMP:
                    new_inst = inst;
                    break;

                case INST_RET:
                    new_inst = INST_RET_RV;
                    break;

                default:
                    fprintf(stderr, "inst `%s` hasn't got kind `REG_VAL`\n", inst_as_cstr(inst));
                    exit(1);
            }
        }
        break;

        case KIND_REG: {
            switch (inst) {
                case INST_PUSH:
                    new_inst = INST_PUSH_R;
                    break;

                case INST_POP:
                    new_inst = INST_POP_R;
                    break;

                case INST_DBR:
                    new_inst = inst;
                    break;

                default:
                    fprintf(stderr, "inst `%s` hasn't got kind `REG`\n", inst_as_cstr(inst));
                    exit(1);
            }
        }
        break;

        case KIND_VAL: {
            switch (inst) {
                case INST_PUSH:
                    new_inst = INST_PUSH_V;
                    break;

                case INST_JZ:
                case INST_JMP:
                case INST_JNZ:
                case INST_CALL:
                    new_inst = inst;
                    break;

                default:
                    fprintf(stderr, "inst `%s` hasn't got kind `VAL`\n", inst_as_cstr(inst));
                    exit(1);
            }
        }
        break;

        case KIND_NONE: {
            switch (inst) {
                case INST_POP:
                    new_inst = INST_POP_N;
                    break;

                case INST_RET:
                    new_inst = INST_RET_N;
                    break;

                case INST_HLT:
                case INST_VLAD:
                    new_inst = inst;
                    break;

                default:
                    fprintf(stderr, "inst `%s` hasn't got kind `none`\n", inst_as_cstr(inst));
                    exit(1);
            }
        }
        break;

        default: {
            fprintf(stderr, "Error: unknown kind `%u`\n", kind);
            break;
        }
    }
    return new_inst;
}

Block_Chain parse_linizer(Linizer *lnz, Program_Jumps *PJ, Hash_Table *ht, int line_debug)
{
    Block_Chain block_chain = {0};
    Inst_Addr inst_pointer = 0;
    size_t inst_counter = 0;
    for (size_t i = 0; i < lnz->count; ++i) {
        Object_Block objb = {0};
        Line line = lnz->items[i];
        if (line.type == LINE_INST) {
            Inst_Kind kind;
            Lexer sublex = line.item;
            Inst src_inst = parse_inst(&sublex, ht);
            Inst cpu_inst = convert_to_cpu_inst(src_inst, &kind, &sublex);
            objb_push(&objb, OBJ_INST(cpu_inst));
            if (line_debug) {
                printf("line: %zu; ", i + 1);
                printf("inst: %s; kind: %u\n", inst_as_cstr(cpu_inst), kind);
            }

            switch (kind) {
                case KIND_REG:     parse_kind_reg(&inst_pointer, &sublex, &objb);                               break;
                case KIND_VAL:     parse_kind_val(cpu_inst, &inst_pointer, &sublex, &objb, PJ, inst_counter);   break;
                case KIND_NONE:    inst_pointer += 1;                                                           break;
                case KIND_REG_REG: parse_kind_reg_reg(&inst_pointer, &sublex, &objb);                           break;
                case KIND_REG_VAL: parse_kind_reg_val(&inst_pointer, &sublex, &objb);                           break;
                default: {
                    fprintf(stderr, "Error: unknown kind `%u`\n", kind);
                    exit(1);
                }
            }
            block_chain_push(&block_chain, objb);
            inst_counter += 1;

        } else if (line.type == LINE_LABEL) {
            Token tk = line.item.items[0];
            if (tk.type != TYPE_TEXT) {
                fprintf(stderr, "Error: in line [%zu] expected label\n", i + 1);
                exit(1);
            }
            String_View label_name = tk.txt;
            Label label = { .name = label_name, .addr = inst_pointer};
            ll_append(&PJ->current, label);

        } else {
            fprintf(stderr, "Error: in `parse_linizer` unknown line [%lu] type\n", i + 1);
            exit(1);
        }
    }

    for (size_t i = 0; i < PJ->deferred.count; ++i) {
        Label dlabel = PJ->deferred.labels[i];
        Label label = ll_search_label(&PJ->current, dlabel.name);
        if (label.addr != (uint64_t)(-1)) {
            block_chain.items[dlabel.addr].items[1] = OBJ_UINT(label.addr); 
        }
    }

    return block_chain;
}

void objb_to_cpu(CPU *c, Object_Block *objb)
{
    for (size_t i = 0; i < objb->count; ++i) {
        Object obj = objb->items[i];
        if (c->program_size + 1 >= c->program_capacity) {
            c->program_capacity *= 2;
            c->program = realloc(c->program, c->program_capacity * sizeof(*c->program));
        }
        c->program[c->program_size++] = obj;
    }
}

void block_chain_to_cpu(CPU *c, Block_Chain *block_chain)
{
    if (c->program_capacity == 0) {
        c->program_capacity = PROGRAM_INIT_CAPACITY;
        c->program = malloc(c->program_capacity * sizeof(*c->program));
        c->program_size = 0;
    }

    for (size_t i = 0; i < block_chain->count; ++i) {
        Object_Block objb = block_chain->items[i];
        objb_to_cpu(c, &objb);
    }
}

int translate_inst(String_View inst_sv, Hash_Table *ht)
{
    if (ht->capacity == 0) {
        inst_ht_init(ht, HT_DEBUG_FALSE);
    }
    
    char *inst_cstr = sv_to_cstr(inst_sv);
    int inst = ht_get_inst(ht, inst_cstr);
    free(inst_cstr);

    return inst;
}

Register parse_register(String_View sv)
{   
    char *acc = reg_as_cstr(ACC);
    if (sv_cmp(sv, sv_from_cstr(acc)))
        return ACC;

    char *accf = reg_as_cstr(ACCF);
    if (sv_cmp(sv, sv_from_cstr(accf)))
        return ACCF;

    int n = sv.data[1] - '0';

    if (sv.data[0] == 'r') {
        return n;
    }

    else if (sv.data[0] == 'f') {
        n = n + ACC + 1;
        return n;
    }

    else {
        fprintf(stderr, "Error: cannot find register by `%d`\n", n);
        exit(1);
    }
}