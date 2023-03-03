%{
#include <stdio.h>

#include "ast.h"
#include "lex.yy.c"

void yyerror(const char *msg);

%}

%locations

/* declared types */
%union {
    int type_int;
    astnode_t *type_ast;
}

/* declared tokens */
%token <type_int> INT      FLOAT    ID
%token SEMI     COMMA
%token <type_int> TYPE
%token LC       RC
%token STRUCT   RETURN   IF       WHILE

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left  OR
%left  AND
%left  <type_int> RELOP
%left  PLUS     MINUS
%left  STAR     DIV
%right NOT
%left  DOT      LP       RP       LB       RB

/* declared non-terminals */
%type <type_ast> Program   ExtDefList      ExtDef  ExtDecList
%type <type_ast> Specifier StructSpecifier OptTag  Tag
%type <type_ast> VarDec    FunDec         VarList ParamDec
%type <type_ast> CompSt    StmtList        Stmt
%type <type_ast> DefList   Def             DecList Dec
%type <type_ast> Exp       Args

%%

/* High-level Definitions */
// a c-- program is composed of several external definitions
Program : ExtDefList
    {
        root = make_syntax_node(Program, "Program", @$.first_line, 1);
        root->childs[0] = $1;
    }
    ;
// ExtDefList is zero or more ExtDef.
ExtDefList: %empty { $$ = NULL; }
    | ExtDef ExtDefList
    { 
        $$ = make_syntax_node(ExtDefList, "ExtDefList", @$.first_line, 2); 
        $$->childs[0] = $1;
        $$->childs[1] = $2; 
    }
    ;
ExtDef: Specifier ExtDecList SEMI // for global variables
    {
        $$ = make_syntax_node(ExtDef, "ExtDef", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = $2;
        $$->childs[2] = make_lex_node(SEMI, "SEMI", @3.first_line, -1);
    }
    | Specifier SEMI // for structs
    {
        $$ = make_syntax_node(ExtDef, "ExtDef", @$.first_line, 1);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(SEMI, "SEMI", @2.first_line, -1);
    }
    | Specifier FunDec CompSt // for functions
    {
        $$ = make_syntax_node(ExtDef, "ExtDef", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = $2;
        $$->childs[2] = $3;
    }
    | Specifier error SEMI { $$ = NULL; }
    ;
// ExtDecList is zero or more VarDec.
ExtDecList: VarDec
    {
        $$ = make_syntax_node(ExtDecList, "ExtDecList", @$.first_line, 1);
        $$->childs[0] = $1;
    }
    | VarDec COMMA ExtDecList
    {
        $$ = make_syntax_node(ExtDecList, "ExtDecList", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(COMMA, "COMMA", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    ;


/* Specifiers */
Specifier: TYPE
    {
        $$ = make_syntax_node(Specifier, "Specifier", @$.first_line, 1);
        $$->childs[0] = make_lex_node(TYPE, "TYPE", @1.first_line, $1);
    }
    | StructSpecifier
    {
        $$ = make_syntax_node(Specifier, "Specifier", @$.first_line, 1);
        $$->childs[0] = $1;
    }
    ;
StructSpecifier: STRUCT OptTag LC DefList RC
    {
        $$ = make_syntax_node(StructSpecifier, "StructSpecifier", @$.first_line, 5);
        $$->childs[0] = make_lex_node(STRUCT, "STRUCT", @1.first_line, -1);
        $$->childs[1] = $2;
        $$->childs[2] = make_lex_node(LC, "LC", @3.first_line, -1);
        $$->childs[3] = $4;
        $$->childs[4] = make_lex_node(RC, "RC", @5.first_line, -1);
    }
    | STRUCT Tag
    {
        $$ = make_syntax_node(StructSpecifier, "StructSpecifier", @$.first_line, 2);
        $$->childs[0] = make_lex_node(STRUCT, "STRUCT", @1.first_line, -1);
        $$->childs[1] = $2;
    }
    ;
// opt means optional
OptTag: %empty { $$ = NULL; }
    | ID
    {
        $$ = make_syntax_node(OptTag, "OptTag", @$.first_line, 1);
        $$->childs[0] = make_lex_node(ID, "ID", @1.first_line, $1);
    }
    ;
Tag: ID
    {
        $$ = make_syntax_node(Tag, "Tag", @$.first_line, 1);
        $$->childs[0] = make_lex_node(ID, "ID", @1.first_line, $1);
    }
    ;


/* Declarations */
VarDec: ID
    {
        $$ = make_syntax_node(VarDec, "VarDec", @$.first_line, 1);
        $$->childs[0] = make_lex_node(ID, "ID", @1.first_line, $1);
    }
    | VarDec LB INT RB // for array
    {
        $$ = make_syntax_node(VarDec, "VarDec", @$.first_line, 4);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(LB, "LB", @2.first_line, -1);
        $$->childs[2] = make_lex_node(INT, "INT", @3.first_line, $3);
        $$->childs[3] = make_lex_node(RB, "RB", @4.first_line, -1);
    }
    ;
FunDec: ID LP VarList RP
    {
        $$ = make_syntax_node(FunDec, "FunDec", @$.first_line, 4);
        $$->childs[0] = make_lex_node(ID, "ID", @1.first_line, $1);
        $$->childs[1] = make_lex_node(LP, "LP", @2.first_line, -1);
        $$->childs[2] = $3;
        $$->childs[3] = make_lex_node(RP, "RP", @4.first_line, -1);
    }
    | ID LP RP
    {
        $$ = make_syntax_node(FunDec, "FunDec", @$.first_line, 3);
        $$->childs[0] = make_lex_node(ID, "ID", @1.first_line, $1);
        $$->childs[1] = make_lex_node(LP, "LP", @2.first_line, -1);
        $$->childs[2] = make_lex_node(RP, "RP", @3.first_line, -1);
    }
    | ID LP error RP { $$ = NULL; }
    | error LP VarList RP { $$ = NULL; }
    ;
VarList: ParamDec COMMA VarList
    {
        $$ = make_syntax_node(VarList, "VarList", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(COMMA, "COMMA", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | ParamDec
    {
        $$ = make_syntax_node(VarList, "VarList", @$.first_line, 1);
        $$->childs[0] = $1;
    }
    ;
ParamDec: Specifier VarDec
    {
        $$ = make_syntax_node(ParamDec, "ParamDec", @$.first_line, 2);
        $$->childs[0] = $1;
        $$->childs[1] = $2;
    }
    ;

/* Statements */
// CompSt stands for compound statement
CompSt: LC DefList StmtList RC
    {
        $$ = make_syntax_node(CompSt, "CompSt", @$.first_line, 4);
        $$->childs[0] = make_lex_node(LC, "LC", @1.first_line, -1);
        $$->childs[1] = $2;
        $$->childs[2] = $3;
        $$->childs[3] = make_lex_node(RC, "RC", @4.first_line, -1);
    }
    | error RC { $$ = NULL; }
    ;
StmtList: %empty { $$ = NULL; }
    | Stmt StmtList
    {
        $$ = make_syntax_node(StmtList, "StmtList", @$.first_line, 2);
        $$->childs[0] = $1;
        $$->childs[1] = $2;
    }
    ;
Stmt: Exp SEMI
    {
        $$ = make_syntax_node(Stmt, "Stmt", @$.first_line, 2);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(SEMI, "SEMI", @2.first_line, -1);
    }
    | CompSt
    {
        $$ = make_syntax_node(Stmt, "Stmt", @$.first_line, 1);
        $$->childs[0] = $1;
    }
    | RETURN Exp SEMI
    {
        $$ = make_syntax_node(Stmt, "Stmt", @$.first_line, 3);
        $$->childs[0] = make_lex_node(RETURN, "RETURN", @1.first_line, -1);
        $$->childs[1] = $2;
        $$->childs[2] = make_lex_node(SEMI, "SEMI", @3.first_line, -1);
    }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
    {
        $$ = make_syntax_node(Stmt, "Stmt", @$.first_line, 5);
        $$->childs[0] = make_lex_node(IF, "IF", @1.first_line, -1);
        $$->childs[1] = make_lex_node(LP, "LP", @2.first_line, -1);
        $$->childs[2] = $3;
        $$->childs[3] = make_lex_node(RP, "RP", @4.first_line, -1);
        $$->childs[4] = $5;
    }
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE { $$ = NULL; }
    | IF LP Exp RP Stmt ELSE Stmt
    {
        $$ = make_syntax_node(Stmt, "Stmt", @$.first_line, 7);
        $$->childs[0] = make_lex_node(IF, "IF", @1.first_line, -1);
        $$->childs[1] = make_lex_node(LP, "LP", @2.first_line, -1);
        $$->childs[2] = $3;
        $$->childs[3] = make_lex_node(RP, "RP", @4.first_line, -1);
        $$->childs[4] = $5;
        $$->childs[5] = make_lex_node(ELSE, "ELSE", @6.first_line, -1);
        $$->childs[6] = $7;
    }
    | IF LP error RP Stmt ELSE Stmt { $$ = NULL; }
    | WHILE LP Exp RP Stmt
    {
        $$ = make_syntax_node(Stmt, "Stmt", @$.first_line, 5);
        $$->childs[0] = make_lex_node(WHILE, "WHILE", @1.first_line, -1);
        $$->childs[1] = make_lex_node(LP, "LP", @2.first_line, -1);
        $$->childs[2] = $3;
        $$->childs[3] = make_lex_node(RP, "RP", @4.first_line, -1);
        $$->childs[4] = $5;
    }
    | error SEMI { $$ = NULL; }
    ;

/* Local Definitions */
DefList: %empty { $$ = NULL; }
    | Def DefList
    {
        $$ = make_syntax_node(DefList, "DefList", @$.first_line, 2);
        $$->childs[0] = $1;
        $$->childs[1] = $2;
    }
    ;
Def: Specifier DecList SEMI
    {
        $$ = make_syntax_node(Def, "Def", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = $2;
        $$->childs[2] = make_lex_node(SEMI, "SEMI", @3.first_line, -1);
    }
    | Specifier error SEMI { $$ = NULL; }
    ;
DecList: Dec
    {
        $$ = make_syntax_node(DecList, "DecList", @$.first_line, 1);
        $$->childs[0] = $1;
    }
    | Dec COMMA DecList
    {
        $$ = make_syntax_node(DecList, "DecList", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(COMMA, "COMMA", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    ;
Dec: VarDec
    {
        $$ = make_syntax_node(Dec, "Dec", @$.first_line, 1);
        $$->childs[0] = $1;
    }
    | VarDec ASSIGNOP Exp
    {
        $$ = make_syntax_node(Dec, "Dec", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(ASSIGNOP, "ASSIGNOP", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    ;

/* Expressions */

Exp: Exp ASSIGNOP Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(ASSIGNOP, "ASSIGNOP", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | Exp AND Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(AND, "AND", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | Exp OR Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(OR, "OR", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | Exp RELOP Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(RELOP, "RELOP", @2.first_line, $2);
        $$->childs[2] = $3;
    }
    | Exp PLUS Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(PLUS, "PLUS", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | Exp MINUS Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(MINUS, "MINUS", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | Exp STAR Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(STAR, "STAR", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | Exp DIV Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(DIV, "DIV", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | LP Exp RP
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = make_lex_node(LP, "LP", @1.first_line, -1);
        $$->childs[1] = $2;
        $$->childs[2] = make_lex_node(RP, "RP", @3.first_line, -1);
    }
    | MINUS Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 2);
        $$->childs[0] = make_lex_node(MINUS, "MINUS", @1.first_line, -1);
        $$->childs[1] = $2;
    }
    | NOT Exp
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 2);
        $$->childs[0] = make_lex_node(NOT, "NOT", @1.first_line, -1);
        $$->childs[1] = $2;
    }
    | ID LP Args RP
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 4);
        $$->childs[0] = make_lex_node(ID, "ID", @1.first_line, $1);
        $$->childs[1] = make_lex_node(LP, "LP", @2.first_line, -1);
        $$->childs[2] = $3;
        $$->childs[3] = make_lex_node(RP, "RP", @4.first_line, -1);
    }
    | ID LP RP
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = make_lex_node(ID, "ID", @1.first_line, $1);
        $$->childs[1] = make_lex_node(LP, "LP", @2.first_line, -1);
        $$->childs[2] = make_lex_node(RP, "RP", @3.first_line, -1);
    }
    | Exp LB Exp RB
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 4);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(LB, "LB", @2.first_line, -1);
        $$->childs[2] = $3;
        $$->childs[3] = make_lex_node(RB, "RB", @4.first_line, -1);
    }
    | Exp DOT ID
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(DOT, "DOT", @2.first_line, -1);
        $$->childs[2] = make_lex_node(ID, "ID", @3.first_line, $3);
    }
    | ID
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 1);
        $$->childs[0] = make_lex_node(ID, "ID", @1.first_line, $1);
    }
    | INT
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 1);
        $$->childs[0] = make_lex_node(INT, "INT", @1.first_line, $1);
    }
    | FLOAT
    {
        $$ = make_syntax_node(Exp, "Exp", @$.first_line, 1);
        $$->childs[0] = make_lex_node(FLOAT, "FLOAT", @1.first_line, $1);
    }
    ;
Args: Exp COMMA Args
    {
        $$ = make_syntax_node(Args, "Args", @$.first_line, 3);
        $$->childs[0] = $1;
        $$->childs[1] = make_lex_node(COMMA, "COMMA", @2.first_line, -1);
        $$->childs[2] = $3;
    }
    | Exp
    {
        $$ = make_syntax_node(Args, "Args", @$.first_line, 1);
        $$->childs[0] = $1;
    }
    ;

%%

void yyerror(const char *msg) {
    errorno++;
    printf("Error type B at Line %d: %s near column %d '%s'\n", yylineno, msg, yycolumn, yytext);
}
