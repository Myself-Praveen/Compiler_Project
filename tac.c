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

    if(node->type == AST_ID || node->type == AST_NUM || node->type == AST_STR || node->type == AST_BOOL) {
        return node->value;
    }

    if(node->type == AST_BINOP) {
        if(strcmp(node->value, "+") == 0 || 
           strcmp(node->value, "-") == 0 || 
           strcmp(node->value, "*") == 0 ||
           strcmp(node->value, "/") == 0 || 
           strcmp(node->value, "**") == 0 ||
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

    if(node->type == AST_ARRAY_ACCESS) {
        ASTNode *idx = node->index;
        /* Print all indices comma-separated */
        char idx_buf[256] = "";
        while(idx) {
            char *ix = generateExprTAC(idx);
            if(strlen(idx_buf) > 0) strcat(idx_buf, ",");
            strcat(idx_buf, ix);
            idx = idx->next;
        }
        char* temp = newTemp();
        printf("%s = %s[%s]\n", temp, node->value, idx_buf);
        return temp;
    }

    if(node->type == AST_PIPELINE) {
        char* left_val = generateExprTAC(node->left);
        char* temp = newTemp();
        if(node->right && node->right->type == AST_CALL) {
            printf("%s = %s |> %s()\n", temp, left_val, node->right->value);
        } else {
            char* right_val = generateExprTAC(node->right);
            printf("%s = %s |> %s\n", temp, left_val, right_val);
        }
        return temp;
    }

    if(node->type == AST_CALL) {
        ASTNode *arg = node->left;
        int idx = 0;
        while(arg) {
            char *a = generateExprTAC(arg->left);
            printf("Param %s\n", a);
            arg = arg->next;
            idx++;
        }
        char *temp = newTemp();
        printf("%s = Call %s, %d\n", temp, node->value, idx);
        return temp;
    }

    return NULL;
}

char* generateStmtTAC(ASTNode *node) {
    if(!node) return NULL;

    if(node->type == AST_PROG) {
        generateStmtTAC(node->left);
        return NULL;
    }

    if(node->type == AST_FUNC) {
        printf("\n%s:\n", node->value);
        printf("BeginFunc\n");
        generateStmtTAC(node->body);
        printf("EndFunc\n");
        return generateStmtTAC(node->next);
    }

    if(node->type == AST_ASSIGN) {
        char* right = generateExprTAC(node->right);
        printf("%s = %s\n", node->left->value, right);
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_RETURN) {
        if(node->left) {
            char* retTemp = generateExprTAC(node->left);
            printf("Return %s\n", retTemp);
        } else {
            printf("Return\n");
        }
        return generateStmtTAC(node->next);
    }
    else if(node->type == AST_PRINT) {
        char* valTemp = generateExprTAC(node->left);
        printf("Show %s\n", valTemp);
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_VAR_DECL) {
        if(node->right) {
            char* right = generateExprTAC(node->right);
            printf("%s = %s\n", node->value, right);
        }
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_ARRAY_DECL) {
        ASTNode *dim = node->index;
        int d = 0;
        while(dim) {
            char *sz = generateExprTAC(dim);
            printf("ArrayAlloc %s, dim[%d] = %s\n", node->value, d, sz);
            d++;
            dim = dim->next;
        }
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_ARRAY_ASSIGN) {
        ASTNode *idx = node->index;
        char idx_buf[256] = "";
        while(idx) {
            char *ix = generateExprTAC(idx);
            if(strlen(idx_buf) > 0) strcat(idx_buf, ",");
            strcat(idx_buf, ix);
            idx = idx->next;
        }
        char *val = generateExprTAC(node->right);
        printf("%s[%s] = %s\n", node->value, idx_buf, val);
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_IF) {
        char* condTemp = generateExprTAC(node->cond);
        char* label = newLabel();

        printf("ifFalse %s goto %s\n", condTemp, label);
        generateStmtTAC(node->body);
        printf("%s:\n", label);

        return generateStmtTAC(node->next);
    }
    
    else if(node->type == AST_IF_ELSE) {
        char* condTemp = generateExprTAC(node->cond);
        char* lfalse = newLabel();
        char* lend = newLabel();

        printf("ifFalse %s goto %s\n", condTemp, lfalse);
        generateStmtTAC(node->body);
        printf("goto %s\n", lend);
        
        printf("%s:\n", lfalse);
        generateStmtTAC(node->else_body);
        printf("%s:\n", lend);

        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_WHILE) {
        char* lstart = newLabel();
        char* lend = newLabel();

        printf("%s:\n", lstart);
        char* cond = generateExprTAC(node->cond);
        printf("ifFalse %s goto %s\n", cond, lend);

        generateStmtTAC(node->body);
        
        printf("goto %s\n", lstart);
        printf("%s:\n", lend);

        return generateStmtTAC(node->next);
    }
    
    else if(node->type == AST_FOR) {
        generateStmtTAC(node->left);
        
        char* lstart = newLabel();
        char* lend = newLabel();

        printf("%s:\n", lstart);
        
        char* cond = NULL;
        if(node->cond) {
            cond = generateExprTAC(node->cond);
            printf("ifFalse %s goto %s\n", cond, lend);
        }

        generateStmtTAC(node->body);
        
        if (node->right) {
            if (node->right->type == AST_ASSIGN) {
                char* r = generateExprTAC(node->right->right);
                printf("%s = %s\n", node->right->left->value, r);
            } else if (node->right->type == AST_UNOP) {
                generateExprTAC(node->right);
            } else {
                generateStmtTAC(node->right);
            }
        }
        
        printf("goto %s\n", lstart);
        if(cond) {
            printf("%s:\n", lend);
        }

        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_WATCH) {
        printf("Watch %s\n", node->value);
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_REWIND) {
        printf("Rewind %s\n", node->value);
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_GC_COLLECT) {
        printf("GC_Collect\n");
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_RETURN) {
        if(node->left) {
            char* right = generateExprTAC(node->left);
            printf("Return %s\n", right);
        } else {
            printf("Return\n");
        }
        return generateStmtTAC(node->next);
    }

    else if(node->type == AST_UNOP || node->type == AST_BINOP || node->type == AST_CALL) {
        generateExprTAC(node);
        return generateStmtTAC(node->next);
    }

    generateStmtTAC(node->next);
    return NULL;
}

void generateTAC(ASTNode *n) {
    temp_cnt = 0;
    lbl_cnt = 0;
    generateStmtTAC(n);
}
