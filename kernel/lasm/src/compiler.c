#include "../include/compiler.h"

void lasm_cut_comments_from_line(String_View *line) 
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

String_View lasm_cut_comments_from_src(String_View *sv)
{
    String_View new_sv = {0};
    while (sv->count > 0) {
        String_View line = sv_div_by_delim(sv, '\n');
        line.count += 1;
        lasm_cut_comments_from_line(&line);
        sv_append_sv(&new_sv, line);
    }
    return new_sv;
}

String_View lasm_load_file(Arena *arena, const char *file_path)
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

    long file_size = ftell(fp);
    if (file_size < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    if (fseek(fp, 0, SEEK_SET) < 0) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    char *buf = arena_alloc(arena, file_size * sizeof(char) + 2);

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

void lasm_save_program_to_file(CPU *c, const char *file_path)
{
    FILE *fp = fopen(file_path, "wb");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file by `%s` path\n", file_path);
        exit(1);
    }

    size_t count = fwrite(c->program, sizeof(c->program[0]), c->program_size, fp);
    if (count != c->program_size) {
        fprintf(stderr, "Error: cannot write to file `%s`", file_path);
        exit(1);
    }

    if (ferror(fp)) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", file_path);
        exit(1);
    }

    fclose(fp);
}

void lasm_translate_source(Arena *arena,
                           String_View src_sv, 
                           CPU *c, 
                           Program_Jumps *PJ, 
                           Hash_Table *ht,
                           Const_Table *ct,
                           int db_lex, int db_lnz, int db_ht, int db_line, int db_lex_txt, int db_bc)
{
    String_View src = lasm_cut_comments_from_src(&src_sv);
    Lexer lex = lexer(arena, src, db_lex_txt);

    if (db_lex) print_lex(&lex, LEX_PRINT_MODE_TRUE);

    Linizer lnz = linizer(arena, &lex, ht, db_ht, db_lnz);
    Block_Chain block_chain = parse_linizer(arena, &lnz, PJ, ht, ct, db_line, db_bc);
    if (block_chain.items == NULL) {
        fprintf(stderr, "Error: cannot make block chain\n");
        exit(1);
    }
    
    free(src.data);
    block_chain_to_cpu(arena, c, &block_chain);
}