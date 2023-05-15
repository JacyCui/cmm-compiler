%{

#include <stdio.h>
#include "syntax.tab.h"
#include "ir.h"

int yycolumn = 1;
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

digit           [0-9]
letter          [_a-zA-Z]
dlim            [ \f\r\t\v]       

%option yylineno

%%

{dlim}                              {                      }
"LABEL"                             { return TOK_LABEL;    }
"FUNCTION"                          { return TOK_FUNCTION; }
"GOTO"                              { return TOK_GOTO;     }
"IF"                                { return TOK_IF;       }
"RETURN"                            { return TOK_RETURN;   }
"DEC"                               { return TOK_DEC;      }
"ARG"                               { return TOK_ARG;      }
"CALL"                              { return TOK_CALL;     }
"PARAM"                             { return TOK_PARAM;    }
"READ"                              { return TOK_READ;     }
"WRITE"                             { return TOK_WRITE;    }
{letter}({letter}|{digit})*         { yylval.type_int = installID(yytext, yyleng); return TOK_ID; }
{digit}+                            { yylval.type_int = atoi(yytext);  return TOK_NUM; }
":="                                { return TOK_ASSIGNOP; }
">="                                { yylval.type_int = GE; return TOK_RELOP; }
"<="                                { yylval.type_int = LE; return TOK_RELOP; }
"=="                                { yylval.type_int = EQ; return TOK_RELOP; }
"!="                                { yylval.type_int = NE; return TOK_RELOP; }
"<"                                 { yylval.type_int = LT; return TOK_RELOP; }
">"                                 { yylval.type_int = GT; return TOK_RELOP; }
":"                                 { return TOK_COLON;    }
"+"                                 { return TOK_PLUS;     }
"-"                                 { return TOK_MINUS;    }
"*"                                 { return TOK_STAR;     }
"/"                                 { return TOK_DIV;      }
"&"                                 { return TOK_AND;      }
"#"                                 { return TOK_SHARP;    }
\n                                  { yycolumn = 1;   return TOK_LF; }
.                                   { errorno++; printf("Error at Line %d: Mysterious character '%c'\n", yylineno, yytext[0]); }

%%

