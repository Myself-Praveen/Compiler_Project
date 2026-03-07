%{
#include <stdio.h>
#include <stdlib.h>

// Declare external functions and variables from Lexer
extern int yylex();
extern int yylineno;
extern FILE *yyin;

// Function to handle syntax errors
void yyerror(const char *s);
%}

/* Define Tokens. Bison will automatically generate numerical values for these. */
%token T_INT T_FLOAT T_ID T_NUM T_RETURN
%token T_ASSIGN T_SEMI T_EQ
%token T_IF T_ELSE 
%token T_PLUS T_MINUS T_MUL T_DIV 
%token T_LPAREN T_RPAREN T_LBRACE T_RBRACE

/* Define Operator Precedence to avoid shift/reduce conflicts (math rules) */
%left T_EQ
%left T_PLUS T_MINUS
%left T_MUL T_DIV

/* The Grammar Rules */
%%

program:
    functions { printf("Program successfully parsed!\n"); }
    ;

functions:
    function functions
    | /* empty */
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
    | /* empty */
    ;

statement:
    declaration
    | assignment
    | if_statement
    | return_statement
    ;

declaration:
    type T_ID T_SEMI { printf("Parsed: Variable Declaration\n"); }
    | type T_ID T_ASSIGN expression T_SEMI { printf("Parsed: Variable Declaration with Initialization\n"); }
    ;

assignment:
    T_ID T_ASSIGN expression T_SEMI { printf("Parsed: Variable Assignment\n"); }
    ;

if_statement:
    T_IF T_LPAREN expression T_RPAREN T_LBRACE statements T_RBRACE { printf("Parsed: If Block\n"); }
    | T_IF T_LPAREN expression T_RPAREN T_LBRACE statements T_RBRACE T_ELSE T_LBRACE statements T_RBRACE { printf("Parsed: If-Else Block\n"); }
    ;

return_statement:
    T_RETURN expression T_SEMI { printf("Parsed: Return Statement\n"); }
    ;

expression:
    expression T_PLUS expression   { /* We are just parsing, not evaluating yet! */ }
    | expression T_MINUS expression
    | expression T_MUL expression
    | expression T_DIV expression
    | expression T_EQ expression
    | T_LPAREN expression T_RPAREN
    | T_ID
    | T_NUM
    ;

%%

/* The C Code Section */

// If Bison finds a syntax error, it calls this function
void yyerror(const char *s) {
    printf("Syntax Error on line %d: %s\n", yylineno, s);
}

// Our main function moves from the Lexer to the Parser
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
    
    // yyparse() will keep asking yylex() for tokens and testing them against our grammar rules
    yyparse();
    
    printf("Finished Parsing.\n");
    return 0;
}
