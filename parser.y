%{
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

extern int yylex();
extern int yylineno;
extern FILE *yyin;

#include "ast.h"
#include "tac.h"
#include "symtab.h"
#include "opt.h"
#include "asm.h"

void yyerror(const char *msg);
extern ASTNode *root;


%}

%union {
    char *str;
    struct ASTNode *node;
}

%token <str> T_ID T_NUM T_INT T_FLOAT T_STR
%token T_RETURN T_PRINT T_WHILE T_FOR T_BREAK T_CONTINUE
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

%type <str> type
%type <node> program functions function statements statement declaration if_statement while_statement for_statement jump_statement return_statement expr_opt expression

%%

program:
    functions { root = create_node(AST_PROG, "Program"); root->left = $1; }
    ;

functions:
    function functions { $$ = $1; $$->next = $2; }
    | { $$ = NULL; }
    ;

function:
    type T_ID T_LPAREN T_RPAREN T_LBRACE statements T_RBRACE { 
        $$ = create_node(AST_FUNC, $2); 
        $$->body = $6; 
    }
    ;

type:
    T_INT { $$ = "int"; }
    | T_FLOAT { $$ = "float"; }
    ;

statements:
    statement statements { 
        $$ = $1; 
        if($$) {
            $$->next = $2;
        } else {
            $$ = $2;
        }
    }
    | { $$ = NULL; }
    ;

statement:
    declaration { $$ = $1; }
    | if_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | for_statement { $$ = $1; }
    | jump_statement { $$ = $1; }
    | return_statement { $$ = $1; }
    | T_PRINT expression T_SEMI { $$ = create_node(AST_PRINT, "print"); $$->left = $2; }
    | expr_opt T_SEMI { $$ = $1; }
    ;

declaration:
    type T_ID T_SEMI { 
        $$ = create_node(AST_VAR_DECL, $2); 
        $$->left = create_node(AST_TYPE, $1);
    }
    | type T_ID T_ASSIGN expression T_SEMI { 
        $$ = create_node(AST_VAR_DECL, $2); 
        $$->left = create_node(AST_TYPE, $1);
        $$->right = $4; 
    }
    ;

if_statement:
    T_IF T_LPAREN expression T_RPAREN T_LBRACE statements T_RBRACE {
        $$ = create_node(AST_IF, "if");
        $$->cond = $3;
        $$->body = $6;
    }
    | T_IF T_LPAREN expression T_RPAREN T_LBRACE statements T_RBRACE T_ELSE T_LBRACE statements T_RBRACE {
        $$ = create_node(AST_IF_ELSE, "if-else");
        $$->cond = $3;
        $$->body = $6;
        $$->else_body = $10;
    }
    ;

while_statement:
    T_WHILE T_LPAREN expression T_RPAREN T_LBRACE statements T_RBRACE {
        $$ = create_node(AST_WHILE, "while");
        $$->cond = $3;
        $$->body = $6;
    }
    ;

for_statement:
    T_FOR T_LPAREN expr_opt T_SEMI expr_opt T_SEMI expr_opt T_RPAREN T_LBRACE statements T_RBRACE {
        $$ = create_node(AST_FOR, "for");
        $$->left = $3; 
        $$->cond = $5;
        $$->right = $7;
        $$->body = $10;
    }
    | T_FOR T_LPAREN declaration expr_opt T_SEMI expr_opt T_RPAREN T_LBRACE statements T_RBRACE {
        $$ = create_node(AST_FOR, "for");
        $$->left = $3; 
        $$->cond = $4;
        $$->right = $6;
        $$->body = $9;
    }
    ;

jump_statement:
    T_BREAK T_SEMI { $$ = create_node(AST_BREAK, "break"); }
    | T_CONTINUE T_SEMI { $$ = create_node(AST_CONTINUE, "continue"); }
    ;

return_statement:
    T_RETURN expression T_SEMI { 
        $$ = create_node(AST_RETURN, "return");
        $$->left = $2;
    }
    ;

expr_opt:
    expression { $$ = $1; }
    | { $$ = NULL; }
    ;

expression:
    T_ID T_ASSIGN expression { 
        $$ = create_node(AST_ASSIGN, "="); 
        $$->left = create_node(AST_ID, $1);
        $$->right = $3;
    }
    | expression T_PLUS expression   { $$ = create_node(AST_BINOP, "+"); $$->left = $1; $$->right = $3; }
    | expression T_MINUS expression  { $$ = create_node(AST_BINOP, "-"); $$->left = $1; $$->right = $3; }
    | expression T_MUL expression    { $$ = create_node(AST_BINOP, "*"); $$->left = $1; $$->right = $3; }
    | expression T_DIV expression    { $$ = create_node(AST_BINOP, "/"); $$->left = $1; $$->right = $3; }
    | expression T_EQ expression     { $$ = create_node(AST_BINOP, "=="); $$->left = $1; $$->right = $3; }
    | expression T_NEQ expression    { $$ = create_node(AST_BINOP, "!="); $$->left = $1; $$->right = $3; }
    | expression T_GT expression     { $$ = create_node(AST_BINOP, ">"); $$->left = $1; $$->right = $3; }
    | expression T_LT expression     { $$ = create_node(AST_BINOP, "<"); $$->left = $1; $$->right = $3; }
    | expression T_GE expression     { $$ = create_node(AST_BINOP, ">="); $$->left = $1; $$->right = $3; }
    | expression T_LE expression     { $$ = create_node(AST_BINOP, "<="); $$->left = $1; $$->right = $3; }
    | expression T_AND expression    { $$ = create_node(AST_BINOP, "&&"); $$->left = $1; $$->right = $3; }
    | expression T_OR expression     { $$ = create_node(AST_BINOP, "||"); $$->left = $1; $$->right = $3; }
    | T_NOT expression               { $$ = create_node(AST_UNOP, "!"); $$->left = $2; }
    | T_MINUS expression %prec UMINUS{ $$ = create_node(AST_UNOP, "-"); $$->left = $2; }
    | T_INC T_ID { $$ = create_node(AST_UNOP, "++"); $$->left = create_node(AST_ID, $2); }
    | T_ID T_INC { $$ = create_node(AST_UNOP, "++ (post)"); $$->left = create_node(AST_ID, $1); }
    | T_DEC T_ID { $$ = create_node(AST_UNOP, "--"); $$->left = create_node(AST_ID, $2); }
    | T_ID T_DEC { $$ = create_node(AST_UNOP, "-- (post)"); $$->left = create_node(AST_ID, $1); }
    | T_LPAREN expression T_RPAREN   { $$ = $2; }
    | T_ID                           { $$ = create_node(AST_ID, $1); }
    | T_NUM                          { $$ = create_node(AST_NUM, $1); }
    | T_STR                          { $$ = create_node(AST_STR, $1); }
    ;

%%

ASTNode *root = NULL;

void yyerror(const char *msg) {
    printf("Syntax Error on line %d: %s\n", yylineno, msg);
}

int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            printf("Could not open file %s\n", argv[1]);
            return 1;
        }
        yyin = f;
    }

    printf("=========================================\n");
    printf("   PHASE 1: LEXICAL ANALYSIS (TOKENS)\n");
    printf("=========================================\n");

    yyparse();

    printf("\n=========================================\n");
    printf("   PHASE 2: SYNTAX ANALYSIS (PARSING)\n");
    printf("=========================================\n");
    
    if (root) {
        printf("Grammar validated successfully. No syntax errors found.\n");
    }

    printf("\n=========================================\n");
    printf("   PHASE 3: ABSTRACT SYNTAX TREE (AST)\n");
    printf("=========================================\n");
    
    if (root) {
        print_ast(stdout, root, 0);
    }
    
    printf("\n=========================================\n");
    printf("   PHASE 4: SEMANTIC ANALYSIS (SYMBOL)\n");
    printf("=========================================\n");
    
    if (root) {
        init_symtab();
        check_semantics(root);
        print_symtab();

        if(get_semantic_errors() > 0) {
            printf("\nCompilation halted: %d semantic errors.\n", get_semantic_errors());
            return 1;
        } else {
            printf("\nSemantics validated successfully. No errors found.\n");
        }
    }

    printf("\n=========================================\n");
    printf("   PHASE 5: INTERMEDIATE CODE (TAC)\n");
    printf("=========================================\n");
    
    if (root) {
        generateTAC(root);
    }

    printf("\n=========================================\n");
    printf("   PHASE 6: CODE OPTIMIZATION (AST/TAC)\n");
    printf("=========================================\n");
    
    if (root) {
        printf("Applying Constant Folding Optimization...\n\n");
        optimize_ast(root);
        
        printf("[ Optimized AST Structure ]\n");
        print_ast(stdout, root, 0);

        printf("\n[ Optimized Intermediate Code (TAC) ]\n");
        generateTAC(root);
    }

    printf("\n=========================================\n");
    printf("   PHASE 7: TARGET CODE GENERATION (ASM)\n");
    printf("=========================================\n");
    
    if (root) {
        generateAssembly(root);
    }

    printf("\n-----------------------------------------\n");
    printf("Compilation pipeline executed successfully.\n");
    return 0;
}
