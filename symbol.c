#include <stdlib.h>
#include <string.h>

#include "symbol.h"
#include "config.h"

symtab_t symtab[MAX_TABLE_LEN] = {
    [SYMBOL_READ] = { .symbol = "read", .lineno = -1},
    [SYMBOL_WRITE] = { .symbol = "write", .lineno = -1}
};
static int symtab_size = 2;

intconst_t intab[MAX_TABLE_LEN];
static int intab_size = 0;
floatconst_t floatab[MAX_TABLE_LEN];
static int floatab_size = 0;

sym_t installID(const char *yytext, int yyleng, int yylineno) {
    int i;
    for (i = 0; i < symtab_size; i++) {
        if (strcmp(symtab[i].symbol, yytext) == 0) {
            if (symtab[i].lineno == -1) {
                symtab[i].lineno = yylineno;
            }
            return i;
        }
    }
    symtab[symtab_size].symbol = (char *)malloc(yyleng + 1);
    strcpy(symtab[symtab_size].symbol, yytext);
    symtab[symtab_size].lineno = yylineno;
    return symtab_size++;
}

int installINT(const char *yytext, int yyleng, int yylineno) {
    intab[intab_size].value = strtol(yytext, NULL, 0);
    intab[intab_size].lineno = yylineno;
    return intab_size++;
}

int installFLOAT(const char *yytext, int yyleng, int yylineno) {
    floatab[floatab_size].value = strtof(yytext, NULL);
    floatab[floatab_size].lineno = yylineno;
    return floatab_size++;
}

void freeSymtab() {
    int i;
    for (i = 2; i < symtab_size; i++) {
        free(symtab[i].symbol);
    }
}
