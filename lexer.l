%{
#include <stdio.h>

#define T_INT 258
#define T_ID 259
#define T_NUM 260
#define T_ASSIGN 261
#define T_SEMI 262
#define T_IF 263
#define T_ELSE 264
#define T_PLUS 265
#define T_MINUS 266
#define T_MUL 267
#define T_DIV 268
#define T_LPAREN 269
#define T_RPAREN 270
#define T_LBRACE 271
#define T_RBRACE 272
%}

%option noyywrap
%option yylineno

%%

"int"       { printf("Found keyword: int\n"); return T_INT; }
"if"        { printf("Found keyword: if\n"); return T_IF; }
"else"      { printf("Found keyword: else\n"); return T_ELSE; }

[a-zA-Z]+   { printf("Found identifier: %s\n", yytext); return T_ID; }
[0-9]+      { printf("Found number: %s\n", yytext); return T_NUM; }

"+"         { printf("Found operator: +\n"); return T_PLUS; }
"-"         { printf("Found operator: -\n"); return T_MINUS; }
"*"         { printf("Found operator: *\n"); return T_MUL; }
"/"         { printf("Found operator: /\n"); return T_DIV; }
"="         { printf("Found operator: =\n"); return T_ASSIGN; }

"("         { printf("Found symbol: (\n"); return T_LPAREN; }
")"         { printf("Found symbol: )\n"); return T_RPAREN; }
"{"         { printf("Found symbol: {\n"); return T_LBRACE; }
"}"         { printf("Found symbol: }\n"); return T_RBRACE; }
";"         { printf("Found symbol: ;\n"); return T_SEMI; }

[ \t\n]+    { }
.           { printf("Error: Unrecognized character '%s' on line %d\n", yytext, yylineno); }

%%

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
    }

    while (yylex() != 0) {
    }

    return 0;
}
