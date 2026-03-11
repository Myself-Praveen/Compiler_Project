%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"
%}

%option noyywrap
%option yylineno

%%

"int"               { printf("%-10s %s\n", "INT",      yytext); yylval.str = strdup(yytext); return T_INT; }
"float"             { printf("%-10s %s\n", "FLOAT",    yytext); yylval.str = strdup(yytext); return T_FLOAT; }
"if"                { printf("%-10s %s\n", "IF",       yytext); return T_IF; }
"else"              { printf("%-10s %s\n", "ELSE",     yytext); return T_ELSE; }
"while"             { printf("%-10s %s\n", "WHILE",    yytext); return T_WHILE; }
"for"               { printf("%-10s %s\n", "FOR",      yytext); return T_FOR; }
"break"             { printf("%-10s %s\n", "BREAK",    yytext); return T_BREAK; }
"continue"          { printf("%-10s %s\n", "CONTINUE", yytext); return T_CONTINUE; }
"return"            { printf("%-10s %s\n", "RETURN",   yytext); return T_RETURN; }
"print"             { printf("%-10s %s\n", "PRINT",    yytext); return T_PRINT; }

[a-zA-Z_][a-zA-Z0-9_]* { printf("%-10s %s\n", "ID",       yytext); yylval.str = strdup(yytext); return T_ID; }
[0-9]+(\.[0-9]+)?   { printf("%-10s %s\n", "NUM",      yytext); yylval.str = strdup(yytext); return T_NUM; }
\"([^\"\\]|\\.)*\"  { printf("%-10s %s\n", "STRING",   yytext); yylval.str = strdup(yytext); return T_STR; }

"+"                 { printf("%-10s %s\n", "PLUS",     yytext); return T_PLUS; }
"-"                 { printf("%-10s %s\n", "MINUS",    yytext); return T_MINUS; }
"*"                 { printf("%-10s %s\n", "MUL",      yytext); return T_MUL; }
"/"                 { printf("%-10s %s\n", "DIV",      yytext); return T_DIV; }
"++"                { printf("%-10s %s\n", "INC",      yytext); return T_INC; }
"--"                { printf("%-10s %s\n", "DEC",      yytext); return T_DEC; }
"=="                { printf("%-10s %s\n", "EQ",       yytext); return T_EQ; }
"!="                { printf("%-10s %s\n", "NEQ",      yytext); return T_NEQ; }
">="                { printf("%-10s %s\n", "GE",       yytext); return T_GE; }
"<="                { printf("%-10s %s\n", "LE",       yytext); return T_LE; }
">"                 { printf("%-10s %s\n", "GT",       yytext); return T_GT; }
"<"                 { printf("%-10s %s\n", "LT",       yytext); return T_LT; }
"&&"                { printf("%-10s %s\n", "AND",      yytext); return T_AND; }
"||"                { printf("%-10s %s\n", "OR",       yytext); return T_OR; }
"!"                 { printf("%-10s %s\n", "NOT",      yytext); return T_NOT; }
"="                 { printf("%-10s %s\n", "ASSIGN",   yytext); return T_ASSIGN; }

"("                 { printf("%-10s %s\n", "LPAREN",   yytext); return T_LPAREN; }
")"                 { printf("%-10s %s\n", "RPAREN",   yytext); return T_RPAREN; }
"{"                 { printf("%-10s %s\n", "LBRACE",   yytext); return T_LBRACE; }
"}"                 { printf("%-10s %s\n", "RBRACE",   yytext); return T_RBRACE; }
";"                 { printf("%-10s %s\n", "SEMI",     yytext); return T_SEMI; }

[ \t\n\r]+          { }
.                   { printf("Error: Unrecognized character '%s' on line %d\n", yytext, yylineno); }

%%
