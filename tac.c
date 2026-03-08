#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "tac.h"

int temp_cnt = 0;
int lbl_cnt = 0;

char* newTemp() {
    char* t = (char*)malloc(16);
    snprintf(t, 16, "t%d", temp_cnt++);
    return t;
}

char* newLabel() {
    char* l = (char*)malloc(16);
    snprintf(l, 16, "L%d", lbl_cnt++);
    return l;
}

char* generateExprTAC(ASTNode *node) {
    if(!node) return NULL;

    if(node->type == AST_ID || node->type == AST_NUM) {
        return node->value;
    }

    if(node->type == AST_BINOP) {
        if(strcmp(node->value, "+") == 0 || 
           strcmp(node->value, "-") == 0 || 
           strcmp(node->value, "*") == 0 ||
           strcmp(node->value, "/") == 0 || 
           strcmp(node->value, ">") == 0 ||
           strcmp(node->value, "<") == 0 ||
           strcmp(node->value, ">=") == 0 ||
           strcmp(node->value, "<=") == 0 ||
           strcmp(node->value, "==") == 0 ||
           strcmp(node->value, "!=") == 0 ) {
            
            char* left = generateExprTAC(node->left);
            char* right = generateExprTAC(node->right);

            char* temp = newTemp();
            printf("%s = %s %s %s\n", temp, left, node->value, right);

            return temp;
        }
    }

    if(node->type == AST_UNOP) {
        char* left = generateExprTAC(node->left);
        char* temp = newTemp();
        printf("%s = %s%s\n", temp, node->value, left);
        return temp;
    }

    return NULL;
}

char* generateStmtTAC(ASTNode *node) {
    if(!node) return NULL;

    if(node->type == AST_SEQ || node->type == AST_PROG) {
        generateStmtTAC(node->left);
        generateStmtTAC(node->right);
        return NULL;
    }

    if(node->type == AST_FUNC) {
        printf("\n%s:\n", node->value);
        printf("BeginFunc\n");
        generateStmtTAC(node->right);
        printf("EndFunc\n");
        return NULL;
    }

    if(node->type == AST_ASSIGN) {
        char* right = generateExprTAC(node->right);
        printf("%s = %s\n", node->left->value, right);
        return node->left->value;
    }

    else if(node->type == AST_VAR_DECL) {
        if(node->left) {
            char* right = generateExprTAC(node->left);
            printf("%s = %s\n", node->value, right);
        }
        return node->value;
    }

    else if(node->type == AST_IF) {
        char* condTemp = generateExprTAC(node->left);
        char* label = newLabel();

        printf("ifFalse %s goto %s\n", condTemp, label);
        generateStmtTAC(node->right);
        printf("%s:\n", label);

        return NULL;
    }
    
    else if(node->type == AST_IF_ELSE) {
        char* condTemp = generateExprTAC(node->left);
        char* lfalse = newLabel();
        char* lend = newLabel();

        printf("ifFalse %s goto %s\n", condTemp, lfalse);
        generateStmtTAC(node->right->left); // true block
        printf("goto %s\n", lend);
        
        printf("%s:\n", lfalse);
        generateStmtTAC(node->right->right); // else block
        printf("%s:\n", lend);

        return NULL;
    }

    else if(node->type == AST_WHILE) {
        char* lstart = newLabel();
        char* lend = newLabel();

        printf("%s:\n", lstart);
        char* cond = generateExprTAC(node->left);
        printf("ifFalse %s goto %s\n", cond, lend);

        generateStmtTAC(node->right);
        
        printf("goto %s\n", lstart);
        printf("%s:\n", lend);

        return NULL;
    }
    
    else if(node->type == AST_FOR) {
        generateStmtTAC(node->left); // initialization
        
        char* lstart = newLabel();
        char* lend = newLabel();

        printf("%s:\n", lstart);
        
        char* cond = NULL;
        if(node->right->left) { // condition exists
            cond = generateExprTAC(node->right->left);
            printf("ifFalse %s goto %s\n", cond, lend);
        }

        generateStmtTAC(node->right->right->right); // body
        
        if (node->right->right->left) { // update exists
            if (node->right->right->left->type == AST_ASSIGN) {
                char* r = generateExprTAC(node->right->right->left->right);
                printf("%s = %s\n", node->right->right->left->left->value, r);
            } else if (node->right->right->left->type == AST_UNOP) {
                generateExprTAC(node->right->right->left);
            } else {
                generateStmtTAC(node->right->right->left);
            }
        }
        
        printf("goto %s\n", lstart);
        if(cond) {
            printf("%s:\n", lend);
        }

        return NULL;
    }

    else if(node->type == AST_RETURN) {
        if(node->left) {
            char* right = generateExprTAC(node->left);
            printf("Return %s\n", right);
        } else {
            printf("Return\n");
        }
        return NULL;
    }

    generateStmtTAC(node->left);
    generateStmtTAC(node->right);

    return NULL;
}

void generateTAC(ASTNode *n) {
    temp_cnt = 0;
    lbl_cnt = 0;
    generateStmtTAC(n);
}
