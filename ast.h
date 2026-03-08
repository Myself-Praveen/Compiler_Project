#ifndef AST_H
#define AST_H

#include <stdio.h>

typedef enum {
    AST_PROG, AST_FUNC, AST_VAR_DECL,
    AST_ASSIGN, AST_IF, AST_WHILE, AST_FOR, AST_RETURN,
    AST_BREAK, AST_CONTINUE, AST_BINOP, AST_UNOP,
    AST_ID, AST_NUM, AST_SEQ, AST_IF_ELSE, AST_STR, AST_TYPE
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *value;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

ASTNode* create_node(ASTNodeType t, char *val);
void print_ast(FILE *fp, ASTNode *n, int i);

#endif
