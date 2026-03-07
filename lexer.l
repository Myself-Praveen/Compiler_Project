%{
#include <stdio.h>
#include "parser.tab.h"
%}

%option noyywrap
%option yylineno

%%

"int"       { printf("Found keyword: int\n"); return T_INT; }
"float"     { printf("Found keyword: float\n"); return T_FLOAT; }
"if"        { printf("Found keyword: if\n"); return T_IF; }
"else"      { printf("Found keyword: else\n"); return T_ELSE; }
"return"    { printf("Found keyword: return\n"); return T_RETURN; }

[a-zA-Z_][a-zA-Z0-9_]*   { printf("Found identifier: %s\n", yytext); return T_ID; }
[0-9]+(\.[0-9]+)?      { printf("Found number: %s\n", yytext); return T_NUM; }

"+"         { printf("Found operator: +\n"); return T_PLUS; }
"-"         { printf("Found operator: -\n"); return T_MINUS; }
"*"         { printf("Found operator: *\n"); return T_MUL; }
"/"         { printf("Found operator: /\n"); return T_DIV; }
"=="        { printf("Found operator: ==\n"); return T_EQ; }
"="         { printf("Found operator: =\n"); return T_ASSIGN; }

"("         { printf("Found symbol: (\n"); return T_LPAREN; }
")"         { printf("Found symbol: )\n"); return T_RPAREN; }
"{"         { printf("Found symbol: {\n"); return T_LBRACE; }
"}"         { printf("Found symbol: }\n"); return T_RBRACE; }
";"         { printf("Found symbol: ;\n"); return T_SEMI; }

[ \t\n\r]+  { }
.           { printf("Error: Unrecognized character '%s' on line %d\n", yytext, yylineno); }

%%


