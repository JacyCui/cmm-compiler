#include <stdio.h>

#include "optimization.h"

void yyrestart(FILE *input_file);
void yyparse();

extern int yylineno;

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s input.ir output.ir\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "a");
    if (!f) {
        printf("No such file \"%s\"!\n", argv[1]);
        return 1;
    }
    fprintf(f, "\n");
    fclose(f);
    f = fopen(argv[1], "r");
    yylineno = 1;
    yyrestart(f);
    yyparse();
    fclose(f);
    
    if (errorno != 0) {
        return 1;
    }

    buildBasicBlocks();
    
    bb_t b;
    for (b = 0; b < bbtabSize; b++) {
        flodTemp(b);
        rmLocalCommonSubexpression(b);
        rmDeadCode(b);
        constFolding(b);
    }
    for (b = 0; b < bbtabSize; b++) {
        flodTemp(b);
        rmLocalCommonSubexpression(b);
        rmDeadCode(b);
        constFolding(b);
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
