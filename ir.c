#include <string.h>
#include <stdlib.h>

#include "ir.h"

int errorno = 0;

char *symtab[MAX_TABLE_LEN]; ///< symbol table

int symtabSize = 0; ///< the number of symbols

sym_t installID(const char* yytext, int yyleng) {
    sym_t i;
    for (i = 0; i < symtabSize; i++) {
        if (strcmp(symtab[i], yytext) == 0) {
            return i;
        }
    }
    symtab[symtabSize] = (char *)malloc(yyleng + 1);
    strcpy(symtab[symtabSize], yytext);
    return symtabSize++;
}

void freeSymtab() {
    sym_t i;
    for (i = 0; i < symtabSize; i++) {
        free(symtab[i]);
    }
}

