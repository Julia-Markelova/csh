%{
#include <stdio.h>
#include "y.tab.h"
%}
%%
[0-9]+                  return NUMBER;
"|"                     return PIPE;
"="                     yylval.sign = strdup(yytext); return EQUALS;
";"                     return SEMICOLON;
[a-zA-Z/_.-]+           yylval.str = strdup(yytext); return WORD;
[$][a-zA-Z0-9_]+        yylval.str = strdup(yytext); return VARIABLE;
">"                     yylval.str = strdup(yytext); return GREAT;
"<"                     yylval.str = strdup(yytext); return LESS;
">>"                    yylval.str = strdup(yytext); return GREAT_GREAT;
>&|&>                   yylval.str = strdup(yytext); return GREAT_AMP;
\n                      return NEWLINE;
[ \t]+                  /* игнорируем пробелы и символы табуляции */;
%%