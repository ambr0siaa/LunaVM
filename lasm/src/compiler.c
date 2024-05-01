#include "../include/compiler.h"

Lasm *lasm_init()
{
    Lasm *L = malloc(sizeof(Lasm));
    
    L->arena = (Arena) {0};
    L->inst_table = (Hash_Table) {0};
    inst_ht_init(&L->arena, &L->inst_table, 0);
    
    L->curjmps = (Label_List)  {0};
    L->defjmps = (Label_List)  {0};
    L->src =     (String_View) {0};
    L->lnz =     (Linizer)     {0};
    L->lex =     (Lexer)       {0};
    L->ct =      (Const_Table) { .capacity = CT_CAPACITY };
    
    L->debug.ht = 0;
    L->debug.lex = 0;
    L->debug.lex_txts = 0;
    L->debug.line = 0;
    L->debug.lnz = 0;

    L->input_file = NULL;
    L->output_file = NULL;

    L->program = NULL;
    L->program_capacity = 0;
    L->program_size = 0;  
    
    return L;
}

void lasm_cleanup(Lasm *L)
{
    L->program_capacity = 0;
    L->program_size = 0;
    L->input_file = NULL;
    L->output_file = NULL;
    
    arena_free(&L->arena);
    free(L);
    L = NULL;
}

void lasm_cmd_args(Lasm *L, int *argc, char ***argv)
{
    const char *program = luna_shift_args(argc, argv);

    if (*argc < 1) {
        USAGE(program);
        fprintf(stderr, "Error: %s expected input and output\n", program);
        exit(1);
    }

    while (*argc > 0) {
        const char *flag = luna_shift_args(argc, argv);
        if (!strcmp("-o", flag)) {
            if (*argc < 1) {
                fprintf(stderr, "Error: %s expected output file\n", program);
                exit(1);
            }
            
            L->output_file = luna_shift_args(argc, argv);

        } else if (!strcmp("-i", flag)) {
            if (*argc < 1) {
                fprintf(stderr, "Error: %s expected input file\n", program);
                exit(1);
            }

            L->input_file = luna_shift_args(argc, argv);

        } else if (!strcmp("-dbFull", flag)) {
            L->debug.ht = 1;
            L->debug.lex = 1;
            L->debug.lnz = 1;
            L->debug.line = 1;
            L->debug.lex_txts = 1;

        } else if (!strcmp("-dbHt", flag)) {
            L->debug.ht = 1;

        } else if (!strcmp("-dbLex", flag)) {
            L->debug.lex = 1;

        } else if (!strcmp("-dbLnz", flag)) {
            L->debug.lnz = 1;

        } else if (!strcmp("-dbLexTxts", flag)) {
            L->debug.lex_txts = 1;

        } else if (!strcmp("-dbLine", flag)) {
            L->debug.line = 1;

        } else if (!strcmp("-dbPrg", flag)) {
            L->debug.output_program = 1;

        } else if (!strcmp("-h", flag)) {
            USAGE(program);
            lasm_help();
            exit(0);

        } else {
            fprintf(stderr, "Error: unknown flag `%s`\n", flag);
            exit(1);
        }
    }
}

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

void lasm_load_file(Lasm *L)
{
    L->src = sv_read_file(L->input_file);
}

void lasm_save_program_to_file(Lasm *L)
{
    FILE *fp = fopen(L->output_file, "wb");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file by `%s` path\n", L->output_file);
        exit(1);
    }

    size_t count = fwrite(L->program, sizeof(L->program[0]), L->program_size, fp);
    if (count != L->program_size) {
        fprintf(stderr, "Error: cannot write to file `%s`", L->output_file);
        exit(1);
    }

    if (ferror(fp)) {
        fprintf(stderr, "Error: cannot read from `%s` file\n", L->output_file);
        exit(1);
    }

    fclose(fp);
}

void lasm_translate_source(Lasm *L)
{
    if (L->src.count == 0) {
        fprintf(stderr, "Error: cannot translate file `%s`\n", L->input_file);
        exit(1);
    }

    String_View src = lasm_cut_comments_from_src(&L->src);
    L->lex = lexer(&L->arena, src, L->debug.lex_txts);

    if (L->debug.lex) 
        print_lex(&L->lex, LEX_PRINT_MODE_TRUE);

    L->lnz = linizer(&L->arena, &L->lex, &L->inst_table, L->debug.ht, L->debug.lnz);
    Block_Chain block_chain = parse_linizer(L);
    if (block_chain.items == NULL) {
        fprintf(stderr, "Error: cannot make block chain\n");
        exit(1);
    }
    
    free(src.data);
    block_chain_to_lasm(L, &block_chain);
}

void lasm_help()
{
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "\t-i          mandatory flag. Input source code with extention `.asm`\n");
    fprintf(stdout, "\t-o          mandatory flag. Input file for output (output is Luna's byte code) with extention `.ln`\n");
    fprintf(stdout, "\t-dbHt       print a hash table state\n");
    fprintf(stdout, "\t-dbLnz      print a linizer that was formed from lexer\n");
    fprintf(stdout, "\t-dbLex      print a lexer that was formed from source code\n");
    fprintf(stdout, "\t-dbLine     print lines with instruction and them kind\n");
    fprintf(stdout, "\t-dbLexTxts  print lexer's tokens with type `TYPE_TXT` while working function `lexer`\n");
    fprintf(stdout, "\t-dbFull     print all debug info into console\n");
}