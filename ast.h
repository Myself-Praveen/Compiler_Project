#ifndef AST_H
#define AST_H

#include <stdio.h>

typedef enum {
    AST_PROG, AST_FUNC, AST_VAR_DECL,
    AST_ASSIGN, AST_IF, AST_IF_ELSE, AST_WHILE, AST_FOR, AST_RETURN,
    AST_BREAK, AST_CONTINUE, AST_BINOP, AST_UNOP,
    AST_ID, AST_NUM, AST_SEQ, AST_STR, AST_TYPE, AST_PRINT,
    AST_PARAM, AST_CALL, AST_ARG
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *value;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *cond;
    struct ASTNode *body;
    struct ASTNode *else_body;
    struct ASTNode *next;
} ASTNode;

ASTNode* create_node(ASTNodeType t, char *val);
void print_ast(FILE *fp, ASTNode *n, int i);

#endif
