#include "../include/asm.h"

String_View asm_load_file(const char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file by `%s` path\n", file_path);
        exit(1);
    }

    if (fseek(fp, 0, SEEK_END) < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    size_t file_size = ftell(fp);
    if (file_size < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    if (fseek(fp, 0, SEEK_SET) < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    char *buf = malloc(file_size);
    if (!buf) {
        fprintf(stderr, "cannot allocate memory for file: %s\n", 
                strerror(errno));
    }

    size_t program_size = fread(buf, 1, file_size, fp);

    if (ferror(fp)) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    fclose(fp);

    return (String_View) {
        .count = program_size,
        .data = buf
    };
}

void ll_append(Lable_List *ll, Lable lable)
{
    if (ll->capacity == 0) {
        ll->capacity = INIT_CAPACITY;
        ll->lables = malloc(sizeof(ll->lables[0]) * ll->capacity);
    }

    if (ll->count + 1 > ll->capacity) {
        ll->capacity *= 2;
        ll->lables = realloc(ll->lables, ll->capacity);
    }

    ll->lables[ll->count++] = lable;
}

Lable ll_search_lable(Lable_List *ll, String_View name)
{
    for (size_t i = 0; i < ll->count; ++i) {
        Lable lable = ll->lables[i];
        if (sv_cmp(lable.name, name)) {
            return lable;
        }
    }
    return (Lable) {
        .addr = -1
    };
}

Register parse_register(String_View sv)
{   
    char *acc = reg_as_cstr(ACC);
    if (sv_cmp(sv, sv_from_cstr(acc)))
        return ACC;

    char *accf = reg_as_cstr(ACCF);
    if (sv_cmp(sv, sv_from_cstr(accf)))
        return ACCF;

    size_t n = sv.data[1] - '0';

    if (sv.data[0] == 'r') {
        return n;
    }

    else if (sv.data[0] == 'f') {
        n = n + ACC + 1;
        return n;
    }

    else {
        fprintf(stderr, "Error: cannot find register\n");
        exit(1);
    }
}

hash_table Inst_Table = {0};

Inst parse_inst(String_View inst_sv)
{
    if (Inst_Table.count == 0) {
        inst_table_init(&Inst_Table, IC);
    }

    sv_append_nul(&inst_sv);
    const char *inst_cstr = inst_sv.data;

    Inst inst = ht_get(&Inst_Table, inst_cstr);
    return inst;
} 

Object parse_value(String_View sv)
{
    int is_float = 0;
    for (size_t i = 0; i < sv.count; ++i) {
        if (sv.data[i] == '.') {
            is_float = 1;
            break;
        }
    }

    if (is_float) {
        sv_append_nul(&sv);
        char *float_cstr = sv.data;
        char *endptr = float_cstr; 

        double d = strtod(float_cstr, &float_cstr);

        if (d == 0 && endptr == float_cstr) {
            fprintf(stderr, "Error: cannot parse `%s` to float64\n",float_cstr);
            exit(1);
        }

        printf("Parse float num: %lf\n", d);
        return OBJ_FLOAT(d);

    } else {
        uint64_t INT = sv_to_int(sv);
        printf("Parse int num: %li\n", INT);
        return OBJ_INT(INT);
    }
}
 
void asm_cut_comments(String_View *line) 
{
    size_t i = 0;
    size_t count = 0;
    while (i < line->count && count != 2) {
        if (line->data[i] == ';') count++;
        i++;
    }

    if (count == 2) {
        line->count = i - 2;
    } 
}

void asm_translate_source(CPU *c, Program_Jumps *PJ, String_View src)
{
    // TODO: - add more info about errors
    //       - add line and symbol numbers where was error  
    
    if (c->program_capacity == 0) {
        c->program_capacity = PROGRAM_INIT_CAPACITY;
        c->program = malloc(c->program_capacity * sizeof(c->program[0]));
    }

    c->program_size = 0;  
    while (src.count > 0) {
        assert(c->program_size <= c->program_capacity);

        if (c->program_size + 1 > c->program_capacity) {
            do { c->program_capacity *= 2; } while (c->program_size + 1 > c->program_capacity);
            c->program = realloc(c->program, c->program_capacity);
        }

        String_View line = sv_trim(sv_div_by_delim(&src, '\n'));
        asm_cut_comments(&line);
        
        if (line.count > 0) {
            String_View inst_name = sv_trim(sv_div_by_delim(&line, ' '));
            
            if (inst_name.count > 0 && inst_name.data[inst_name.count - 1] == ':') {
                String_View lable_name = {
                    .count = inst_name.count - 1,
                    .data = inst_name.data
                };
                
                Lable lable = {
                    .name = lable_name,
                    .addr = c->program_size
                };

                ll_append(&PJ->current, lable);
                inst_name = sv_div_by_delim(&line, ' ');
            }

            if (inst_name.count > 0) {
                // if inst will be INST_HLT function won't do any checks and just push inst into a cpu program
                Inst inst = parse_inst(inst_name);
                c->program[c->program_size++] = OBJ_INST(inst);
            
                if (inst_has_2_regs(inst)) {
                    // TODO: for all arefmetic insts add number support by `$` symbol
                    String_View reg1_sv = sv_trim(sv_div_by_delim(&line, ','));
                    
                    if (line.count == 0)  {
                        fprintf(stderr, "Usage: "SV_Fmt" <register>, <register>\n", SV_Args(inst_name));
                        fprintf(stderr, "Error: inst `"SV_Fmt"` expected 2 args\n", SV_Args(inst_name));
                        exit(1);
                    }

                    Register reg1 = parse_register(reg1_sv);
                    c->program[c->program_size++] = OBJ_REG(reg1);

                    String_View reg2_sv = sv_trim(line);

                    Register reg2 = parse_register(reg2_sv);
                    c->program[c->program_size++] = OBJ_REG(reg2);

                } else if (inst == INST_MOVI || inst == INST_MOVF) {
                    // TODO: add parsing for floating values
                    //       and checks: INT or FLOAT
                    
                    String_View reg1_sv = sv_trim(sv_div_by_delim(&line, ','));

                    if (line.count == 0)  {
                        fprintf(stderr, "Usage: "SV_Fmt" <register>, <val>\n", SV_Args(inst_name));
                        fprintf(stderr, "Error: inst `"SV_Fmt"` expected 2 args\n", SV_Args(inst_name));
                        exit(1);
                    }

                    Register reg1 = parse_register(reg1_sv);
                    c->program[c->program_size++] = OBJ_REG(reg1);

                    String_View val = sv_trim(line);
                    c->program[c->program_size++] = parse_value(val);
                    
                } else if (inst == INST_DBR) {
                    String_View reg_sv = sv_trim(line);
                    Register reg = parse_register(reg_sv);

                    c->program[c->program_size++] = OBJ_REG(reg);

                } else if (inst == INST_JMP || inst == INST_JNZ || inst == INST_JZ) {
                    String_View addr_sv = sv_trim(line);

                    if (isdigit(*addr_sv.data)) {
                        uint64_t addr = sv_to_int(addr_sv);
                        if (addr < 0) {
                            fprintf(stderr, "Error: address must be greater than 0\n");
                            exit(1);
                        }

                        c->program[c->program_size++] = OBJ_UINT(addr);
                    } else {
                        Lable lable = ll_search_lable(&PJ->current, addr_sv);
                    
                        if (lable.addr != -1) {
                            c->program[c->program_size++] = OBJ_UINT(lable.addr);
                        } else {
                            Lable dlable = {
                                .addr = c->program_size,
                                .name = addr_sv
                            };

                            ll_append(&PJ->deferred, dlable);
                            c->program[c->program_size++] = OBJ_INT(-1);                
                        }
                    }

                }
            }  
        }
    }

    for (size_t i = 0; i < PJ->deferred.count; ++i) {
        Lable lable = ll_search_lable(&PJ->current, PJ->deferred.lables[i].name);
        if (lable.addr != -1) {
            c->program[PJ->deferred.lables[i].addr] = OBJ_UINT(lable.addr);
        }
    }
}

char *asm_shift_args(int *argc, char ***argv)
{
    assert(argc >= 0);
    char *result = **argv;

    *argv += 1;
    *argc -= 1;

    return result;
}