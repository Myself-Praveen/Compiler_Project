#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "opt.h"

void optimize_ast(ASTNode *node) {
    if (!node) return;

    optimize_ast(node->left);
    optimize_ast(node->right);
    optimize_ast(node->cond);
    optimize_ast(node->body);
    optimize_ast(node->else_body);

    if (node->type == AST_BINOP) {
        if (node->left && node->left->type == AST_NUM &&
            node->right && node->right->type == AST_NUM) {

            int is_float = (strchr(node->left->value, '.') != NULL) || 
                           (strchr(node->right->value, '.') != NULL);

            if (!is_float) {
                int left_val = atoi(node->left->value);
                int right_val = atoi(node->right->value);
                int result = 0;
                int valid = 1;

                if (strcmp(node->value, "+") == 0) result = left_val + right_val;
                else if (strcmp(node->value, "-") == 0) result = left_val - right_val;
                else if (strcmp(node->value, "*") == 0) result = left_val * right_val;
                else if (strcmp(node->value, "/") == 0 && right_val != 0) result = left_val / right_val;
                else valid = 0;

                if (valid) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%d", result);
                    
                    node->type = AST_NUM;
                    free(node->value);
                    node->value = strdup(buf);
                    
                    node->left = NULL;
                    node->right = NULL;
                }
            } else {
                float left_val = atof(node->left->value);
                float right_val = atof(node->right->value);
                float result = 0.0f;
                int valid = 1;

                if (strcmp(node->value, "+") == 0) result = left_val + right_val;
                else if (strcmp(node->value, "-") == 0) result = left_val - right_val;
                else if (strcmp(node->value, "*") == 0) result = left_val * right_val;
                else if (strcmp(node->value, "/") == 0 && right_val != 0.0f) result = left_val / right_val;
                else valid = 0;

                if (valid) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%.2f", result);
                    
                    node->type = AST_NUM;
                    free(node->value);
                    node->value = strdup(buf);
                    
                    node->left = NULL;
                    node->right = NULL;
                }
            }
        }
    }

    optimize_ast(node->next);
}
