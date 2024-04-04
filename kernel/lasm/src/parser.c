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

Object parse_variable(Variable_Table *vt, String_View name)
{
    Variable_Statement var = vt_get(vt, name);
    if (var.type == VAR_TYPE_ERR) {
        fprintf(stderr, "Error: unknown variable `"SV_Fmt"`\n", SV_Args(name));
        exit(1);
    }
    // var_print(&var);
    Object val;
    switch (var.type) {
        case VAR_TYPE_INT:
            val = OBJ_INT(var.as_i64);
            break;   
        case VAR_TYPE_UINT:
            val = OBJ_UINT(var.as_u64);
            break;
        case VAR_TYPE_FLOAT:
            val = OBJ_FLOAT(var.as_f64);
            break;
        default:
            fprintf(stderr, "Error: unknown type `%u` in `parse_kind_reg_val`\n", var.type);
            exit(1);
    }
    return val;
}

void parse_kind_reg_val(Inst_Addr *inst_pointer, Lexer *lex, Object_Block *objb, Variable_Table *vt)
{
    for (Token tk = lex->items[lex->tp + 1]; 
        tk.type != TYPE_NONE;
        tk = token_next(lex)) {

        if (tk.type == TYPE_COMMA) continue;
        else if (tk.type == TYPE_TEXT) {
            int reg = parse_register(tk.txt);
            if (reg != -1) {
                objb_push(objb, OBJ_REG(reg));
            } else {
                Object val = parse_variable(vt, tk.txt);
                objb_push(objb, val);
            }

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
                    Variable_Table *vt,
                    size_t line_num)
{
    switch (inst) {
        case INST_PUSH_V: {
            Object val_obj;
            Token tk = token_get(lex, 0, SKIP_FALSE);
            if (tk.type == TYPE_OPEN_BRACKET || tk.type == TYPE_VALUE) {
                Token val_tk = parse_val(lex);
                token_next(lex);
                if (val_tk.val.type == VAL_FLOAT) {
                    val_obj = OBJ_FLOAT(val_tk.val.f64);
                } else {
                    val_obj = OBJ_INT(val_tk.val.i64);
                }
                objb_push(objb, val_obj);
            } else {
                Object val = parse_variable(vt, tk.txt);
                objb_push(objb, val);
            }
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

int try_register(String_View sv)
{
    if (sv.data[0] == 'r' || sv.data[0] == 'f') {
        if (isdigit(sv.data[1])) {
            return 1;
        } else {
            return 0;
        }
    } else if (sv_cmp(sv, sv_from_cstr("acc"))) {
        return 1;
    } else if (sv_cmp(sv, sv_from_cstr("accf"))) {
        return 1;
    } else {
        return 0;
    }
}

// TODO: when error occur show line with uncorrect syntax
Inst convert_to_cpu_inst(Inst inst, Inst_Kind *inst_kind, Lexer *lex)
{
    Inst new_inst;
    Inst_Kind kind;

    Token tk = token_get(lex, 0, SKIP_FALSE);
    if (tk.type == TYPE_NONE) {
        kind = KIND_NONE;
    } else {
        tk = token_get(lex, 1, SKIP_FALSE);
        if (tk.type == TYPE_COMMA) {
            tk = token_get(lex, 2, SKIP_FALSE);
            if (tk.type == TYPE_VALUE || tk.type == TYPE_OPEN_BRACKET || tk.type == TYPE_DOLLAR) {
                kind = KIND_REG_VAL;
            } else {
                if (try_register(tk.txt))
                    kind = KIND_REG_REG;
                else
                    kind = KIND_REG_VAL;
            }
        } else {
            tk = token_get(lex, 0, SKIP_FALSE);
            if ((tk.type == TYPE_OPEN_BRACKET || tk.type == TYPE_VALUE)) {
                kind = KIND_VAL;
            } else {
                if (try_register(tk.txt)) 
                    kind = KIND_REG;
                else 
                    kind = KIND_VAL;
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

// TODO: remake
void parse_variable_expr(Lexer *lex, Variable_Table *vt)
{   
    size_t start_tp = lex->tp;
    for (Token tk = token_next(lex); tk.type != TYPE_NONE; tk = token_next(lex)) {
        if (tk.type == TYPE_TEXT) {
            Variable_Statement var = vt_get(vt, tk.txt);
            if (var.type == VAR_TYPE_ERR) {
                fprintf(stderr, "Error: cannot find variable by name `"SV_Fmt"`\n", SV_Args(tk.txt));
                exit(1);
            }
            Token new_tk = {0};
            new_tk.type = TYPE_VALUE;
            switch (var.type) {
                case VAR_TYPE_INT:
                    new_tk.val.type = VAL_INT;
                    new_tk.val.i64 = var.as_i64;
                    break;
                case VAR_TYPE_FLOAT:
                    new_tk.val.type = VAL_FLOAT;
                    new_tk.val.f64 = var.as_f64;
                    break;
                default:
                    fprintf(stderr, "Error: unknown type `%u`\n", var.type);
                    exit(1);
            }
            lex->items[lex->tp - 1] = new_tk;
        }
    }
    lex->tp = start_tp; 
}

// TODO: remake this
// TODO: better errors
Variable_Statement parse_line_variable(Lexer *lex)
{
    Variable_Statement var = {0};
    token_next(lex); // skip key word
    
    Token var_name = token_next(lex);
    if (var_name.type != TYPE_TEXT) {
        fprintf(stderr, "Error: uncorrect name for variable\n");
        exit(1);
    }
    var.name = var_name.txt;

    Token open_curly = token_next(lex);
    if (open_curly.type != TYPE_OPEN_CURLY) {
        fprintf(stderr, "Error: after name expectes `{`\n");
        exit(1);
    }

    Token var_type = token_next(lex);
    if (var_type.type != TYPE_TEXT) {
        fprintf(stderr, "Error: expectes text for defining type\n");
        exit(1);
    }

    if (sv_cmp(var_type.txt, sv_from_cstr("i64"))) {
        var.type = VAR_TYPE_INT;

    } else if (sv_cmp(var_type.txt, sv_from_cstr("u64"))) {
        var.type = VAR_TYPE_UINT;

    } else if (sv_cmp(var_type.txt, sv_from_cstr("f64"))) {
        var.type = VAR_TYPE_FLOAT;

    } else {
        fprintf(stderr, "Error: unknown type `"SV_Fmt"` for variable\n", SV_Args(var_type.txt));
        exit(1);
    }

    Token semicolon = token_next(lex);
    if (semicolon.type != TYPE_SEMICOLON) {
        fprintf(stderr, "Error: expected `;` after type\n");
        exit(1);
    }

    Token var_value = token_next(lex);
    switch (var.type) {
        case VAR_TYPE_INT:
            if (var_value.val.type != VAL_INT) {
                fprintf(stderr, "Error: after type `i64` expectes integer\n");
                exit(1);
            }
            var.as_i64 = var_value.val.i64;
            break;

        case VAR_TYPE_UINT:
            if (var_value.val.type != VAL_INT) {
                fprintf(stderr, "Error: after type `u64` expectes unsigned integer\n");
                exit(1);
            }
            if (var_value.val.i64 < 0 ) {
                fprintf(stderr, "Error: unsigned value cannot be less than 0\n");
                exit(1);    
            }
            var.as_u64 = var_value.val.i64;
            break;

        case VAR_TYPE_FLOAT:
            if (var_value.val.type != VAL_FLOAT) {
                fprintf(stderr, "Error: after type `f64` expectes float\n");
                exit(1);
            }
            var.as_f64 = var_value.val.f64;
            break;

        default:
            fprintf(stderr, "Error: unknown type in `parse_variable`\n");
            exit(1);
    }

    return var;
}

Object_Block parse_line_inst(Line line, Hash_Table *ht, Program_Jumps *PJ, Variable_Table *vt,
                             size_t inst_counter, size_t *inst_pointer, 
                             int db_line, size_t line_num)
{
    Inst_Kind kind;
    Object_Block objb = {0};
    Lexer sublex = line.item;
    Inst src_inst = parse_inst(&sublex, ht);
    Inst cpu_inst = convert_to_cpu_inst(src_inst, &kind, &sublex);
    objb_push(&objb, OBJ_INST(cpu_inst));
    
    if (db_line) {
        printf("line: %zu; ", line_num + 1);
        printf("inst: %s; kind: %u\n", inst_as_cstr(cpu_inst), kind);
    }

    switch (kind) {
        case KIND_REG:     parse_kind_reg(inst_pointer, &sublex, &objb);                                 break;
        case KIND_VAL:     parse_kind_val(cpu_inst, inst_pointer, &sublex, &objb, PJ, vt, inst_counter); break;
        case KIND_NONE:    *inst_pointer += 1;                                                           break;
        case KIND_REG_REG: parse_kind_reg_reg(inst_pointer, &sublex, &objb);                             break;
        case KIND_REG_VAL: parse_kind_reg_val(inst_pointer, &sublex, &objb, vt);                         break;
        default: {
            fprintf(stderr, "Error: unknown kind `%u`\n", kind);
            exit(1);
        }
    }

    return objb;
}

void parse_line_label(String_View name, Program_Jumps *PJ, size_t inst_pointer)
{
    Label label = { .name = name, .addr = inst_pointer};
    ll_append(&PJ->current, label);
}

void block_chain_debug(Block_Chain *bc)
{
    printf("\n---------------------------------------------\n\n");
    for (size_t i = 0; i < bc->count; ++i) {
        Object_Block objb = bc->items[i];
        printf("object block: %zu\n", i);
        for (size_t j = 0; j < objb.count; ++j) {
            Object o = objb.items[j];
            printf("inst: %u, reg: %u, i64: %li, u64: %lu, f64: %lf\n",
                    o.inst, o.reg, o.i64, o.u64, o.f64);
        }
        printf("\n");
    }
    printf("\n---------------------------------------------\n\n");
}

Block_Chain parse_linizer(Linizer *lnz, Program_Jumps *PJ, Hash_Table *ht, Variable_Table *vt, int line_debug, int bc_debug)
{
    size_t entry_ip = 0;
    size_t inst_counter = 0;
    Inst_Addr inst_pointer = 0;
    Block_Chain block_chain = {0};
    for (size_t i = 0; i < lnz->count; ++i) {
        Line line = lnz->items[i];
        if (line.type == LINE_INST) {
            Object_Block objb = parse_line_inst(line, ht, PJ, vt, inst_counter, &inst_pointer, line_debug, i);
            block_chain_push(&block_chain, objb);
            inst_counter += 1;

        } else if (line.type == LINE_LABEL) {
            Token tk = line.item.items[0];
            if (tk.type != TYPE_TEXT) {
                fprintf(stderr, "Error: in line [%zu] expected label\n", i + 1);
                exit(1);
            }
            parse_line_label(tk.txt, PJ, inst_pointer);

        } else if (line.type == LINE_ENTRY_LABLE) {
            entry_ip = inst_pointer;
            Token tk = line.item.items[0];
            if (tk.type != TYPE_TEXT) {
                fprintf(stderr, "Error: in line [%zu] expected label\n", i + 1);
                exit(1);
            }

            parse_line_label(tk.txt, PJ, inst_pointer);

        } else if (line.type == LINE_VAR) {
            Variable_Statement var = parse_line_variable(&line.item);
            vt_insert(vt, var);

        } else {
            fprintf(stderr, "Error: in `parse_linizer` unknown line [%lu] type\n", i + 1);
            exit(1);
        }
    }

    // TODO: erros for unused deffred labels
    for (size_t i = 0; i < PJ->deferred.count; ++i) {
        Label dlabel = PJ->deferred.labels[i];
        Label label = ll_search_label(&PJ->current, dlabel.name);
        if (label.addr != (uint64_t)(-1)) {
            block_chain.items[dlabel.addr].items[1] = OBJ_UINT(label.addr); 
        }
    }

    Object_Block entry_obj = {0};
    objb_push(&entry_obj, OBJ_UINT(entry_ip));
    block_chain_push(&block_chain, entry_obj);

    if (bc_debug == BLOCK_CHAIN_DEBUG_TRUE) 
        block_chain_debug(&block_chain);
    
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

int parse_register(String_View sv)
{   
    char *acc = reg_as_cstr(ACC);
    if (sv_cmp(sv, sv_from_cstr(acc)))
        return ACC;

    char *accf = reg_as_cstr(ACCF);
    if (sv_cmp(sv, sv_from_cstr(accf)))
        return ACCF;

    int n = sv.data[1] - '0';

    if (sv.data[0] == 'r' && isdigit(sv.data[1])) {
        return n;

    } else if (sv.data[0] == 'f' && isdigit(sv.data[1])) {
        n = n + ACC + 1;
        return n;

    } else {
        return -1;
    }
}