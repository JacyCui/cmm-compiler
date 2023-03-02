#include <stdlib.h>
#include <string.h>

#include "symbol.h"

#define MAX_NUM 1024

symtab_t symtab[MAX_NUM];
static int symtab_size = 0;
intconst_t intab[MAX_NUM];
static int intab_size = 0;
floatconst_t floatab[MAX_NUM];
static int floatab_size = 0;

int installID(const char *yytext, int yyleng, int yylineno) {
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
    for (i = 0; i < symtab_size; i++) {
        free(symtab[i].symbol);
    }
}
