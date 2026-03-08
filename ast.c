#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* create_node(ASTNodeType t, char *val) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    n->type = t;
    if(val) {
        n->value = strdup(val);
    } else {
        n->value = NULL;
    }
    n->left = NULL;
    n->right = NULL;
    return n;
}

void print_ast_internal(FILE *out, ASTNode *node, char *prefix, int last_node, int root_node) {
    if (node == NULL) return;

    if (!root_node) {
        fprintf(out, "%s", prefix);
        if(last_node) fprintf(out, "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 ");
        else fprintf(out, "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 ");
    }
    
    char buf[256];
    switch(node->type) {
        case AST_PROG: snprintf(buf, sizeof(buf), "Program"); break;
        case AST_FUNC: snprintf(buf, sizeof(buf), "Function(%s)", node->value); break;
        case AST_VAR_DECL: snprintf(buf, sizeof(buf), "Decl(%s)", node->value); break;
        case AST_ASSIGN: snprintf(buf, sizeof(buf), "Assign"); break;
        case AST_IF: snprintf(buf, sizeof(buf), "If"); break;
        case AST_IF_ELSE: snprintf(buf, sizeof(buf), "IfElse"); break;
        case AST_WHILE: snprintf(buf, sizeof(buf), "While"); break;
        case AST_FOR: snprintf(buf, sizeof(buf), "For"); break;
        case AST_RETURN: snprintf(buf, sizeof(buf), "Return"); break;
        case AST_BREAK: snprintf(buf, sizeof(buf), "Break"); break;
        case AST_CONTINUE: snprintf(buf, sizeof(buf), "Continue"); break;
        case AST_BINOP: snprintf(buf, sizeof(buf), "BinOp(%s)", node->value); break;
        case AST_UNOP: snprintf(buf, sizeof(buf), "UnOp(%s)", node->value); break;
        case AST_ID: snprintf(buf, sizeof(buf), "ID(%s)", node->value); break;
        case AST_NUM: snprintf(buf, sizeof(buf), "Value(%s)", node->value); break;
        case AST_SEQ: snprintf(buf, sizeof(buf), "Block"); break;
        default: snprintf(buf, sizeof(buf), "Unknown"); break;
    }
    fprintf(out, "%s\n", buf);

    char new_pfx[1024];
    if(root_node) {
        strcpy(new_pfx, "");
    } else {
        if(last_node) snprintf(new_pfx, sizeof(new_pfx), "%s    ", prefix);
        else snprintf(new_pfx, sizeof(new_pfx), "%s\xE2\x94\x82   ", prefix);
    }

    if(node->left && !node->right) {
        print_ast_internal(out, node->left, new_pfx, 1, 0);
    } else if(!node->left && node->right) {
        print_ast_internal(out, node->right, new_pfx, 1, 0);
    } else if(node->left && node->right) {
        print_ast_internal(out, node->left, new_pfx, 0, 0);
        print_ast_internal(out, node->right, new_pfx, 1, 0);
    }
}

void print_ast(FILE *fp, ASTNode *n, int x) {
    print_ast_internal(fp, n, "", 1, 1);
}
