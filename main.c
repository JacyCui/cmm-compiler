#include <stdio.h>

#include "symbol.h"
#include "ast.h"
#include "checker.h"
#include "semantic.h"

void yyrestart (FILE *input_file);
astnode_t *yyparse ();

extern int yylineno;

int main(int argc, char **argv) {
    if (argc <= 1) {
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
    
    if (errorno == 0) {
        // print_ast(root, 0);
        check(root);
        checkFunctionTable();
    }

    freeSymtab();
    free_ast(root);
    return 0;
}
