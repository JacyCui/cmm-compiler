#ifndef AST_H
#define AST_H

#include "config.h"

typedef struct astnode_t {
    int type;           // ast-node type
    const char *name;   // ast-node name
    int lineno;         // lexeme location
    int tabid;             // symtab, intable or floatable index
    struct astnode_t *childs[MAX_AST_CHILDNUM];
    int child_num;
} astnode_t;

enum {
    Program = 600,   ExtDefList = 601,      ExtDef = 602,  ExtDecList = 603,
    Specifier = 604, StructSpecifier = 605, OptTag = 606,  Tag = 607,
    VarDec = 608,    FunDec = 609,         VarList = 610, ParamDec = 611,
    CompSt = 612,    StmtList = 613,        Stmt = 614,
    DefList = 615,   Def = 616,             DecList = 617, Dec = 618,
    Exp = 619,       Args = 620,
    LT  = 621,       LE = 622,       EQ = 623,       NE = 624,       GT = 625,       GE = 626
};

extern astnode_t *root;
extern int errorno;

astnode_t *make_syntax_node(int type, const char *name, int lineno, int child_num);
astnode_t *make_lex_node(int type, const char *name, int lineno, int tabid);

void print_ast(astnode_t *p, int indent);
void free_ast(astnode_t *p);

#endif
