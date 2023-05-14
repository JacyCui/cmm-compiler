#include <stdio.h>

#include "symbol.h"
#include "ast.h"
#include "checker.h"
#include "semantic.h"
#include "irgen.h"
#include "ir.h"
#include "codegen.h"

void yyrestart (FILE *input_file);
astnode_t *yyparse ();

extern int yylineno;

int main(int argc, char **argv) {
    if (argc <= 1) {
        printf("Usage: %s input.cmm output.s\n", argv[0]);
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

    // print_ast(root, 0);
    check(root);
    checkFunctionTable();

    if (errorno != 0) {
        return 1;
    }

    genIR(root);
    
    if (argc >= 3) {
        f = fopen(argv[2], "w");
    } else {
        return 1;
    }
    if (!f) {
        printf("No such file \"%s\"!\n", argv[1]);
        return 1;
    }

    //dumpIR(f);

    translateIRToCode(f);

    fclose(f);
    freeSymtab();
    free_ast(root);
    return 0;
}
