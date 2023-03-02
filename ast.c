#include "ast.h"
#include "syntax.tab.h"
#include "symbol.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

astnode_t *root;
int errorno = 0;

astnode_t *make_syntax_node(int type, const char *name, int lineno, int child_num) {
    astnode_t *p = (astnode_t *)malloc(sizeof(astnode_t));
    p->type = type;
    p->name = name;
    p->lineno = lineno;
    p->tabid = -1;
    p->child_num = child_num;
    return p;
}

astnode_t *make_lex_node(int type, const char *name, int lineno, int tabid) {
    astnode_t *p = (astnode_t *)malloc(sizeof(astnode_t));
    p->type = type;
    p->name = name;
    p->lineno = lineno;
    p->tabid = tabid;
    p->child_num = 0;
    return p;
}

void put_indent(int indent) {
    int i;
    for (i = 0; i < indent; i++) {
        putchar(' ');
    }
}

void print_ast(astnode_t *p, int indent) {
    if (p == NULL) {
        return;
    }
    put_indent(indent);
    if (p->child_num == 0) {
        switch (p->type)
        {
        case ID: printf("ID: %s\n", symtab[p->tabid].symbol); break;
        case TYPE: printf("TYPE: %s\n", p->tabid == INT ? "int" : "float"); break;
        case INT: printf("INT: %d\n", intab[p->tabid].value); break;
        case FLOAT: printf("FLOAT: %.6f\n", floatab[p->tabid].value); break;
        default: printf("%s\n", p->name);
        }
        return;
    }
    printf("%s (%d)\n", p->name, p->lineno);
    int i;
    for (i = 0; i < p->child_num; i++) {
        print_ast(p->childs[i], indent + 2);
    }
}

void free_ast(astnode_t *p) {
    if (p == NULL) {
        return;
    }
    int i;
    for (i = 0; i < p->child_num; i++) {
        free_ast(p->childs[i]);
    }
    free(p);
}
