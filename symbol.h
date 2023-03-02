#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct symtab_t {
    char *symbol;
    int lineno;
} symtab_t;

typedef struct intconst_t {
    int value;
    int lineno;
} intconst_t;

typedef struct floatconst_t {
    float value;
    int lineno;
} floatconst_t;

extern symtab_t symtab[];
extern intconst_t intab[];
extern floatconst_t floatab[];

int installID(const char *yytext, int yyleng, int yylineno);
int installINT(const char *yytext, int yyleng, int yylineno);
int installFLOAT(const char *yytext, int yyleng, int yylineno);

void freeSymtab();

#endif
