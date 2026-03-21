%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"
%}

%option noyywrap
%option yylineno

%%

"num"               { printf("%-10s %s\n", "NUM_T",    yytext); yylval.str = strdup(yytext); return T_INT; }
"float"             { printf("%-10s %s\n", "FLOAT",    yytext); yylval.str = strdup(yytext); return T_FLOAT; }
"flag"              { printf("%-10s %s\n", "FLAG",     yytext); yylval.str = strdup(yytext); return T_BOOL; }
"check"             { printf("%-10s %s\n", "CHECK",    yytext); return T_IF; }
"otherwise"         { printf("%-10s %s\n", "OTHERWISE",yytext); return T_ELSE; }
"while"             { printf("%-10s %s\n", "WHILE",    yytext); return T_WHILE; }
"for"               { printf("%-10s %s\n", "FOR",      yytext); return T_FOR; }
"break"             { printf("%-10s %s\n", "BREAK",    yytext); return T_BREAK; }
"continue"          { printf("%-10s %s\n", "CONTINUE", yytext); return T_CONTINUE; }
"return"            { printf("%-10s %s\n", "RETURN",   yytext); return T_RETURN; }
"show"              { printf("%-10s %s\n", "SHOW",     yytext); return T_PRINT; }
"watch"             { printf("%-10s %s\n", "WATCH",    yytext); return T_WATCH; }
"rewind"            { printf("%-10s %s\n", "REWIND",   yytext); return T_REWIND; }
"gc_collect"        { printf("%-10s %s\n", "GC_COLLECT", yytext); return T_GC_COLLECT; }
"true"              { printf("%-10s %s\n", "TRUE",     yytext); yylval.str = strdup("1"); return T_NUM; }
"false"             { printf("%-10s %s\n", "FALSE",    yytext); yylval.str = strdup("0"); return T_NUM; }
"start"             { printf("%-10s %s\n", "START",    yytext); yylval.str = strdup("main"); return T_ID; }

[a-zA-Z_][a-zA-Z0-9_]* { printf("%-10s %s\n", "ID",       yytext); yylval.str = strdup(yytext); return T_ID; }
[0-9]+(\.[0-9]+)?   { printf("%-10s %s\n", "NUM",      yytext); yylval.str = strdup(yytext); return T_NUM; }
\"([^\"\\]|\\.)*\"  { printf("%-10s %s\n", "STRING",   yytext); yylval.str = strdup(yytext); return T_STR; }

"|>"                { printf("%-10s %s\n", "PIPE",     yytext); return T_PIPE; }
"**"                { printf("%-10s %s\n", "EXP",      yytext); return T_EXP; }
"++"                { printf("%-10s %s\n", "INC",      yytext); return T_INC; }
"--"                { printf("%-10s %s\n", "DEC",      yytext); return T_DEC; }
"+"                 { printf("%-10s %s\n", "PLUS",     yytext); return T_PLUS; }
"-"                 { printf("%-10s %s\n", "MINUS",    yytext); return T_MINUS; }
"*"                 { printf("%-10s %s\n", "MUL",      yytext); return T_MUL; }
"/"                 { printf("%-10s %s\n", "DIV",      yytext); return T_DIV; }
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
"["                 { printf("%-10s %s\n", "LBRACKET", yytext); return T_LBRACKET; }
"]"                 { printf("%-10s %s\n", "RBRACKET", yytext); return T_RBRACKET; }
";"                 { printf("%-10s %s\n", "SEMI",     yytext); return T_SEMI; }
","                 { printf("%-10s %s\n", "COMMA",    yytext); return T_COMMA; }

[ \t\n\r]+          { }
.                   { printf("Error: Unrecognized character '%s' on line %d\n", yytext, yylineno); }

%%
