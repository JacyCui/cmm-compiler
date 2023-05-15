%{
#include <stdio.h>

#include "ir.h"
#include "lex.yy.c"

void yyerror(const char *msg);

%}

%locations

/* declared types */
%union {
    int type_int;
}

/* declared tokens */
%token TOK_LABEL
%token <type_int> TOK_ID
%token TOK_COLON
%token TOK_FUNCTION
%token TOK_ASSIGNOP
%token TOK_PLUS
%token TOK_MINUS
%token TOK_STAR
%token TOK_DIV
%token TOK_AND
%token TOK_SHARP
%token TOK_GOTO
%token TOK_IF
%token <type_int> TOK_RELOP
%token TOK_RETURN
%token TOK_DEC
%token TOK_ARG
%token TOK_CALL
%token TOK_PARAM
%token TOK_READ
%token TOK_WRITE
%token <type_int> TOK_NUM
%token TOK_LF

/* declared non-terminals */

%type <type_int> Operand

%%

Program : StmtList;

StmtList: Stmt StmtList
    | %empty

Stmt: TOK_LABEL TOK_ID TOK_COLON TOK_LF // LABEL x :
    {

    }
    | TOK_FUNCTION TOK_ID TOK_COLON TOK_LF // FUNCTION f :
    {

    }
    | Operand TOK_ASSIGNOP Operand TOK_LF // x := y
    {

    }
    | Operand TOK_ASSIGNOP Operand TOK_PLUS Operand TOK_LF // x := y + z
    {

    } 
    | Operand TOK_ASSIGNOP Operand TOK_MINUS Operand TOK_LF // x := y - z
    {
        
    }
    | Operand TOK_ASSIGNOP Operand TOK_STAR Operand TOK_LF // x := y * z
    {
        
    }
    | Operand TOK_ASSIGNOP Operand TOK_DIV Operand TOK_LF // x := y / z
    {
        
    }
    | TOK_GOTO TOK_ID TOK_LF // GOTO x
    {

    }
    | TOK_IF Operand TOK_RELOP Operand TOK_GOTO TOK_ID TOK_LF // IF x [relop] y GOTO z
    {

    }
    | TOK_RETURN Operand TOK_LF // RETURN x
    {

    }
    | TOK_DEC TOK_ID TOK_NUM TOK_LF // DEC x [size]
    {

    }
    | TOK_ARG Operand TOK_LF // ARG x
    {

    }
    | Operand TOK_ASSIGNOP TOK_CALL TOK_ID TOK_LF // x := CALL f
    {

    }
    | TOK_PARAM TOK_ID TOK_LF // PARAM x
    {

    }
    | TOK_READ Operand TOK_LF // READ x
    {

    }
    | TOK_WRITE Operand TOK_LF // WRITE x
    {

    }
    | TOK_LF // empty

Operand: TOK_ID
    {

    }
    | TOK_AND TOK_ID
    {

    }
    | TOK_STAR TOK_ID
    {

    }
    | TOK_SHARP TOK_NUM
    {

    }

%%

void yyerror(const char *msg) {
    errorno++;
    printf("At Line %d: %s near column %d '%s'\n", yylineno, msg, yycolumn, yytext);
}
