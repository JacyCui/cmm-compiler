%{
#include <stdio.h>

#include "ir.h"
#include "lex.yy.c"

void yyerror(const char *msg);

static func_t currentFunction = -1;

static ref_t argBuffer[MAX_PARAM_NUM];
static int argBufferSize = 0;

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

Program : StmtList
    {
        functab[currentFunction].irEnd = stmtabSize - 1;
    }

StmtList: Stmt StmtList
    | %empty

Stmt: TOK_FUNCTION TOK_ID TOK_COLON TOK_LF // FUNCTION f :
    {
        if (currentFunction != -1) {
            functab[currentFunction].irEnd = stmtabSize - 1;
        }
        currentFunction = createEmptyFunction($2);
        functab[currentFunction].irBegin = stmtabSize;
    }
    | TOK_PARAM TOK_ID TOK_LF // PARAM x
    {
        functab[currentFunction].params[functab[currentFunction].paramNum++] 
            = getOrCreateByName($2);
    }
    | TOK_DEC TOK_ID TOK_NUM TOK_LF // DEC x [size]
    {
        addDEC(createArray($2, $3));
    }
    | TOK_ARG Operand TOK_LF // ARG x
    {
        argBuffer[argBufferSize++] = $2;
    }
    | Operand TOK_ASSIGNOP TOK_CALL TOK_ID TOK_LF // x := CALL f
    {
        addCALL($4, argBufferSize, argBuffer, $1);
        argBufferSize = 0;
    }
    | TOK_LABEL TOK_ID TOK_COLON TOK_LF // LABEL x :
    {
        addLABEL($2);
    }
    | Operand TOK_ASSIGNOP Operand TOK_LF // x := y
    {
        addASSIGN($1, $3);
    }
    | Operand TOK_ASSIGNOP Operand TOK_PLUS Operand TOK_LF // x := y + z
    {
        addADD($1, $3, $5);
    } 
    | Operand TOK_ASSIGNOP Operand TOK_MINUS Operand TOK_LF // x := y - z
    {
        addSUB($1, $3, $5);
    }
    | Operand TOK_ASSIGNOP Operand TOK_STAR Operand TOK_LF // x := y * z
    {
        addMUL($1, $3, $5);
    }
    | Operand TOK_ASSIGNOP Operand TOK_DIV Operand TOK_LF // x := y / z
    {
        addDIV($1, $3, $5);
    }
    | TOK_GOTO TOK_ID TOK_LF // GOTO x
    {
        addGOTO($2);
    }
    | TOK_IF Operand TOK_RELOP Operand TOK_GOTO TOK_ID TOK_LF // IF x [relop] y GOTO z
    {
        addIFGOTO($2, $3, $4, $6);
    }
    | TOK_RETURN Operand TOK_LF // RETURN x
    {
        addRETURN($2);
    }
    | TOK_READ Operand TOK_LF // READ x
    {
        addREAD($2);
    }
    | TOK_WRITE Operand TOK_LF // WRITE x
    {
        addWRITE($2);
    }
    | TOK_LF // empty

Operand: TOK_ID
    {
        $$ = getOrCreateDirectRef(getOrCreateByName($1));
    }
    | TOK_AND TOK_ID
    {
        $$ = getOrCreateAddressRef(getOrCreateByName($2));
    }
    | TOK_STAR TOK_ID
    {
        $$ = getOrCreateDereferenceRef(getOrCreateByName($2));
    }
    | TOK_SHARP TOK_NUM
    {
        $$ = getOrCreateDirectRef(getOrCreateConstant($2));
    }

%%

void yyerror(const char *msg) {
    errorno++;
    printf("At Line %d: %s near column %d '%s'\n", yylineno, msg, yycolumn, yytext);
}
