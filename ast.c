#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* create_node(ASTNodeType t, char *val) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    n->type = t;
    n->value = val ? strdup(val) : NULL;
    n->left = NULL;
    n->right = NULL;
    n->cond = NULL;
    n->body = NULL;
    n->else_body = NULL;
    n->next = NULL;
    return n;
}

#define BRANCH  "|-- "
#define CORNER  "\\-- "
#define PIPE    "|   "
#define SPACE   "    "

void print_node(FILE *out, ASTNode *node, char *prefix, int is_last);

int collect_chain(ASTNode *node, ASTNode **out, int max) {
    int n = 0;
    while(node && n < max) {
        out[n++] = node;
        node = node->next;
    }
    return n;
}

void get_label(ASTNode *node, char *buf, int sz) {
    switch(node->type) {
        case AST_PROG:         snprintf(buf, sz, "Program"); break;
        case AST_FUNC:         snprintf(buf, sz, "Function(%s)", node->value); break;
        case AST_VAR_DECL:     snprintf(buf, sz, "Decl(%s)", node->value); break;
        case AST_ASSIGN:       snprintf(buf, sz, "Assign(%s)", node->left ? node->left->value : ""); break;
        case AST_IF:           snprintf(buf, sz, "Check"); break;
        case AST_IF_ELSE:      snprintf(buf, sz, "CheckOtherwise"); break;
        case AST_WHILE:        snprintf(buf, sz, "While"); break;
        case AST_FOR:          snprintf(buf, sz, "For"); break;
        case AST_RETURN:       snprintf(buf, sz, "Return"); break;
        case AST_BREAK:        snprintf(buf, sz, "Break"); break;
        case AST_CONTINUE:     snprintf(buf, sz, "Continue"); break;
        case AST_BINOP:        snprintf(buf, sz, "BinOp(%s)", node->value); break;
        case AST_UNOP:         snprintf(buf, sz, "UnOp(%s)", node->value); break;
        case AST_ID:           snprintf(buf, sz, "ID(%s)", node->value); break;
        case AST_NUM:          snprintf(buf, sz, "Value(%s)", node->value); break;
        case AST_SEQ:          snprintf(buf, sz, "Block"); break;
        case AST_STR:          snprintf(buf, sz, "String(%s)", node->value); break;
        case AST_TYPE:         snprintf(buf, sz, "Type(%s)", node->value); break;
        case AST_PRINT:        snprintf(buf, sz, "Show"); break;
        case AST_PARAM:        snprintf(buf, sz, "Param(%s)", node->value); break;
        case AST_CALL:         snprintf(buf, sz, "Call(%s)", node->value); break;
        case AST_ARG:          snprintf(buf, sz, "Arg"); break;
        case AST_BOOL:         snprintf(buf, sz, "Flag(%s)", node->value); break;
        case AST_WATCH:        snprintf(buf, sz, "Watch(%s)", node->value); break;
        case AST_REWIND:       snprintf(buf, sz, "Rewind(%s)", node->value); break;
        default:               snprintf(buf, sz, "Unknown"); break;
    }
}

void print_node(FILE *out, ASTNode *node, char *prefix, int is_last) {
    if(!node) return;

    fprintf(out, "%s%s", prefix, is_last ? CORNER : BRANCH);

    char label[512];
    get_label(node, label, sizeof(label));
    fprintf(out, "%s\n", label);

    char child_pfx[1024];
    snprintf(child_pfx, sizeof(child_pfx), "%s%s", prefix, is_last ? SPACE : PIPE);

    ASTNode *children[8];
    int cnt = 0;

    if(node->type == AST_PROG || node->type == AST_FUNC) {
        if(node->type == AST_FUNC && node->left) {
            ASTNode *param = node->left;
            while(param) {
                print_node(out, param, child_pfx, (!param->next && !node->body));
                param = param->next;
            }
        }
        ASTNode *body = node->body ? node->body : (node->type == AST_PROG ? node->left : NULL);
        if(body) {
            ASTNode *stmts[256];
            int stmt_cnt = collect_chain(body, stmts, 256);
            for(int i = 0; i < stmt_cnt; i++)
                print_node(out, stmts[i], child_pfx, (i == stmt_cnt - 1));
        }
        return;
    }

    if(node->type == AST_ASSIGN) {
        if(node->right) children[cnt++] = node->right;
    }
    else if(node->type == AST_VAR_DECL) {
        if(node->left) children[cnt++] = node->left;
        if(node->right) children[cnt++] = node->right;
    }
    else if(node->type == AST_IF || node->type == AST_IF_ELSE) {
        if(node->cond) children[cnt++] = node->cond;

        int has_else = (node->type == AST_IF_ELSE && node->else_body);

        fprintf(out, "%s%s", child_pfx, has_else ? BRANCH : CORNER);
        fprintf(out, "Then:\n");
        char then_pfx[1024];
        snprintf(then_pfx, sizeof(then_pfx), "%s%s", child_pfx, has_else ? PIPE : SPACE);
        ASTNode *body_stmts[64];
        int body_cnt = collect_chain(node->body, body_stmts, 64);
        for(int i = 0; i < body_cnt; i++)
            print_node(out, body_stmts[i], then_pfx, (i == body_cnt - 1));

        if(has_else) {
            fprintf(out, "%s%s", child_pfx, CORNER);
            fprintf(out, "Otherwise:\n");
            char else_pfx[1024];
            snprintf(else_pfx, sizeof(else_pfx), "%s%s", child_pfx, SPACE);
            ASTNode *else_stmts[64];
            int else_cnt = collect_chain(node->else_body, else_stmts, 64);
            for(int i = 0; i < else_cnt; i++)
                print_node(out, else_stmts[i], else_pfx, (i == else_cnt - 1));
        }

        for(int i = 0; i < cnt; i++)
            print_node(out, children[i], child_pfx, 1);
        return;
    }
    else if(node->type == AST_WHILE || node->type == AST_FOR) {
        if(node->cond) children[cnt++] = node->cond;
        fprintf(out, "%s%s", child_pfx, CORNER);
        fprintf(out, "Body:\n");
        char body_pfx[1024];
        snprintf(body_pfx, sizeof(body_pfx), "%s%s", child_pfx, SPACE);
        ASTNode *body_stmts[64];
        int body_cnt = collect_chain(node->body, body_stmts, 64);
        for(int i = 0; i < body_cnt; i++)
            print_node(out, body_stmts[i], body_pfx, (i == body_cnt - 1));
    }
    else if(node->type == AST_PRINT || node->type == AST_RETURN) {
        if(node->left) children[cnt++] = node->left;
    }
    else if(node->type == AST_BINOP) {
        if(node->left) children[cnt++] = node->left;
        if(node->right) children[cnt++] = node->right;
    }
    else if(node->type == AST_UNOP) {
        if(node->left) children[cnt++] = node->left;
    }
    else if(node->type == AST_CALL) {
        if(node->left) children[cnt++] = node->left;
    }
    else {
        if(node->left) children[cnt++] = node->left;
        if(node->right) children[cnt++] = node->right;
        if(node->cond) children[cnt++] = node->cond;
        if(node->body) children[cnt++] = node->body;
        if(node->else_body) children[cnt++] = node->else_body;
    }

    for(int i = 0; i < cnt; i++)
        print_node(out, children[i], child_pfx, (i == cnt - 1));
}

void print_ast(FILE *fp, ASTNode *n, int x) {
    if(!n) return;
    if(n->type == AST_PROG) {
        char label[256];
        get_label(n, label, sizeof(label));
        fprintf(fp, "%s\n", label);
        ASTNode *funcs[32];
        int func_cnt = collect_chain(n->left, funcs, 32);
        for(int i = 0; i < func_cnt; i++)
            print_node(fp, funcs[i], "", (i == func_cnt - 1));
    } else {
        print_node(fp, n, "", 1);
    }
}
