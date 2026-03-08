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
    n->cond = NULL;
    n->body = NULL;
    n->else_body = NULL;
    n->next = NULL;
    return n;
}

void print_ast_list(FILE *out, ASTNode *node, char *prefix);
void print_ast_internal(FILE *out, ASTNode *node, char *prefix, int is_last, int is_root);

void print_pseudo_node(FILE *out, const char *label, ASTNode *child, char *prefix, int last_node, int is_lst) {
    if (!child) return;
    fprintf(out, "%s", prefix);
    
    if(last_node) fprintf(out, "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 ");
    else fprintf(out, "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 ");
    fprintf(out, "%s\n", label);

    char pfx[1024];
    if(last_node) {
        snprintf(pfx, sizeof(pfx), "%s    ", prefix);
    } else {
        snprintf(pfx, sizeof(pfx), "%s\xE2\x94\x82   ", prefix);
    }

    if (is_lst) {
        print_ast_list(out, child, pfx);
    } else {
        print_ast_internal(out, child, pfx, 1, 0); 
    }
}

void print_ast_list(FILE *out, ASTNode *node, char *prefix) {
    ASTNode *curr = node;
    while(curr != NULL) {
        int last = (curr->next == NULL);
        print_ast_internal(out, curr, prefix, last, 0);
        curr = curr->next;
    }
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
        case AST_WHILE: snprintf(buf, sizeof(buf), "While"); break;
        case AST_FOR: snprintf(buf, sizeof(buf), "For"); break;
        case AST_RETURN: snprintf(buf, sizeof(buf), "Return"); break;
        case AST_BREAK: snprintf(buf, sizeof(buf), "Break"); break;
        case AST_CONTINUE: snprintf(buf, sizeof(buf), "Continue"); break;
        case AST_BINOP: snprintf(buf, sizeof(buf), "BinOp(%s)", node->value); break;
        case AST_UNOP: snprintf(buf, sizeof(buf), "UnOp(%s)", node->value); break;
        case AST_ID: snprintf(buf, sizeof(buf), "ID(%s)", node->value); break;
        case AST_NUM: snprintf(buf, sizeof(buf), "Value(%s)", node->value); break;
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

    struct Item {
        const char *l;
        ASTNode *n;
        int list;
    } items[10];
    int count = 0;

    if (node->type == AST_PROG) {
        items[count].l = NULL;
        items[count].n = node->left;
        items[count].list = 1;
        count++;
    } else {
        if(node->cond) { items[count].l = "Condition"; items[count].n = node->cond; items[count].list = 0; count++; }
        if(node->left) { items[count].l = NULL; items[count].n = node->left; items[count].list = 0; count++; }
        if(node->right) { items[count].l = NULL; items[count].n = node->right; items[count].list = 0; count++; }
        if(node->body) { items[count].l = "Body"; items[count].n = node->body; items[count].list = 1; count++; }
        if(node->else_body) { items[count].l = "Else"; items[count].n = node->else_body; items[count].list = 1; count++; }
    }

    int i = 0;
    while(i < count) {
        int is_last = (i == count - 1);
        if(items[i].l != NULL) {
            print_pseudo_node(out, items[i].l, items[i].n, new_pfx, is_last, items[i].list);
        } else {
            if(items[i].list) {
                print_ast_list(out, items[i].n, new_pfx);
            } else {
                print_ast_internal(out, items[i].n, new_pfx, is_last, 0);
            }
        }
        i++;
    }
}

void print_ast(FILE *fp, ASTNode *n, int x) {
    print_ast_internal(fp, n, "", 1, 1);
}
