%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern int yylineno;
extern FILE *yyin;

void yyerror(const char *s);
%}

%token T_INT T_FLOAT T_ID T_NUM T_RETURN
%token T_WHILE T_FOR T_BREAK T_CONTINUE
%token T_ASSIGN T_SEMI T_EQ T_NEQ T_GT T_LT T_GE T_LE
%token T_IF T_ELSE 
%token T_PLUS T_MINUS T_MUL T_DIV T_INC T_DEC
%token T_AND T_OR T_NOT
%token T_LPAREN T_RPAREN T_LBRACE T_RBRACE

%right T_ASSIGN
%left T_OR
%left T_AND
%left T_EQ T_NEQ
%left T_GT T_LT T_GE T_LE
%left T_PLUS T_MINUS
%left T_MUL T_DIV
%right T_NOT T_INC T_DEC UMINUS
%nonassoc T_ELSE

%%

program:
    functions { printf("Program successfully parsed!\n"); }
    ;

functions:
    function functions
    | 
    ;

function:
    type T_ID T_LPAREN T_RPAREN T_LBRACE statements T_RBRACE { printf("Parsed: Function Definition\n"); }
    ;

type:
    T_INT
    | T_FLOAT
    ;

statements:
    statement statements
    | 
    ;

statement:
    declaration
    | if_statement
    | while_statement
    | for_statement
    | jump_statement
    | return_statement
    | expr_opt T_SEMI { printf("Parsed: Expression Statement\n"); }
    ;

declaration:
    type T_ID T_SEMI { printf("Parsed: Variable Declaration\n"); }
    | type T_ID T_ASSIGN expression T_SEMI { printf("Parsed: Variable Declaration with Initialization\n"); }
    ;

if_statement:
    T_IF T_LPAREN expression T_RPAREN T_LBRACE statements T_RBRACE { printf("Parsed: If Block\n"); }
    | T_IF T_LPAREN expression T_RPAREN T_LBRACE statements T_RBRACE T_ELSE T_LBRACE statements T_RBRACE { printf("Parsed: If-Else Block\n"); }
    ;

while_statement:
    T_WHILE T_LPAREN expression T_RPAREN T_LBRACE statements T_RBRACE { printf("Parsed: While Loop\n"); }
    ;

for_statement:
    T_FOR T_LPAREN expr_opt T_SEMI expr_opt T_SEMI expr_opt T_RPAREN T_LBRACE statements T_RBRACE { printf("Parsed: For Loop\n"); }
    | T_FOR T_LPAREN declaration expr_opt T_SEMI expr_opt T_RPAREN T_LBRACE statements T_RBRACE { printf("Parsed: For Loop with declaration\n"); }
    ;

jump_statement:
    T_BREAK T_SEMI { printf("Parsed: Break Statement\n"); }
    | T_CONTINUE T_SEMI { printf("Parsed: Continue Statement\n"); }
    ;

return_statement:
    T_RETURN expression T_SEMI { printf("Parsed: Return Statement\n"); }
    ;

expr_opt:
    expression
    | 
    ;

expression:
    T_ID T_ASSIGN expression       { printf("Parsed: Assignment\n"); }
    | expression T_PLUS expression
    | expression T_MINUS expression
    | expression T_MUL expression
    | expression T_DIV expression
    | expression T_EQ expression
    | expression T_NEQ expression
    | expression T_GT expression
    | expression T_LT expression
    | expression T_GE expression
    | expression T_LE expression
    | expression T_AND expression
    | expression T_OR expression
    | T_NOT expression
    | T_MINUS expression %prec UMINUS
    | T_INC T_ID { printf("Parsed: Pre-Increment\n"); }
    | T_ID T_INC { printf("Parsed: Post-Increment\n"); }
    | T_DEC T_ID { printf("Parsed: Pre-Decrement\n"); }
    | T_ID T_DEC { printf("Parsed: Post-Decrement\n"); }
    | T_LPAREN expression T_RPAREN
    | T_ID
    | T_NUM
    ;

%%

void yyerror(const char *s) {
    printf("Syntax Error on line %d: %s\n", yylineno, s);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            printf("Could not open file %s\n", argv[1]);
            return 1;
        }
        yyin = file;
    }

    printf("Starting Syntax Analysis (Parsing)...\n");
    
    yyparse();
    
    printf("Finished Parsing.\n");
    return 0;
}
