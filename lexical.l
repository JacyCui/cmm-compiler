%{

#include <stdio.h>
#include "ast.h"
#include "symbol.h"

#include "syntax.tab.h"


int yycolumn = 1;
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

dec_pos         [1-9]
dec_digit       [0-9]
hex_digit       [0-9a-fA-F]
oct_pos         [1-7]
oct_digit       [0-7]
letter          [_a-zA-Z]
dlim            [ \f\r\t\v]

DEC_NUM         (0|{dec_pos}{dec_digit}*)
HEX_NUM         0[xX]{hex_digit}+
OCT_NUM         0{oct_pos}{oct_digit}+

%option yylineno

%%

{dlim}                              {                  }
{DEC_NUM}|{HEX_NUM}|{OCT_NUM}       { yylval.type_int = installINT(yytext, yyleng, yylineno);   return INT;   }
{DEC_NUM}\.{dec_digit}+             { yylval.type_int = installFLOAT(yytext, yyleng, yylineno); return FLOAT; }
int                                 { yylval.type_int = INT;   return TYPE; }    
float                               { yylval.type_int = FLOAT; return TYPE; }
struct                              { return STRUCT;   }
return                              { return RETURN;   }
if                                  { return IF;       }
else                                { return ELSE;     }
while                               { return WHILE;    }
{letter}({letter}|{dec_digit})*     { yylval.type_int = installID(yytext, yyleng, yylineno);    return ID;   }
";"                                 { return SEMI;     }
","                                 { return COMMA;    }
">="                                { yylval.type_int = GE; return RELOP; }
"<="                                { yylval.type_int = LE; return RELOP; }
"=="                                { yylval.type_int = EQ; return RELOP; }
"!="                                { yylval.type_int = NE; return RELOP; }
"<"                                 { yylval.type_int = LT; return RELOP; }
">"                                 { yylval.type_int = GT; return RELOP; }
"="                                 { return ASSIGNOP; }
"+"                                 { return PLUS;     }
"-"                                 { return MINUS;    }
"*"                                 { return STAR;     }
"/"                                 { return DIV;      }
"&&"                                { return AND;      }
"||"                                { return OR;       }
"."                                 { return DOT;      }
"!"                                 { return NOT;      }
"("                                 { return LP;       }
")"                                 { return RP;       }
"["                                 { return LB;       }
"]"                                 { return RB;       }
"{"                                 { return LC;       }
"}"                                 { return RC;       }
\n                                  { yycolumn = 1;    }
.                                   { errorno++; printf("Error type A at Line %d: Mysterious character '%c'\n", yylineno, yytext[0]); }

%%

