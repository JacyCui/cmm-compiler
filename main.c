#include <stdio.h>

#include "ir.h"

void yyrestart(FILE *input_file);
void yyparse();

extern int yylineno;

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s input.ir output.ir\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        printf("No such file \"%s\"!\n", argv[1]);
        return 1;
    }
    yylineno = 1;
    yyrestart(f);
    yyparse();
    fclose(f);
    
    if (errorno != 0) {
        return 1;
    }
    
    f = fopen(argv[2], "w");
    if (!f) {
        printf("No such file \"%s\"!\n", argv[1]);
        return 1;
    }

    dumpIR(f);

    fclose(f);
    freeSymtab();
    return 0;
}
