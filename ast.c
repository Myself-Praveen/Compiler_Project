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
    n->index = NULL;
    return n;
}

/* UTF-8 tree drawing characters */
#define BRANCH  "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 "   /* ├── */
#define CORNER  "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 "   /* └── */
#define PIPE    "\xE2\x94\x82   "                           /* │   */
#define SPACE   "    "

/* Forward declarations */
void print_node(FILE *out, ASTNode *node, char *prefix, int is_last);
void print_index_list(FILE *out, ASTNode *idx, char *prefix, int is_last);

/* Collect all siblings via ->next into a flat list, returns count */
int collect_chain(ASTNode *node, ASTNode **out, int max) {
    int n = 0;
    while(node && n < max) {
        out[n++] = node;
        node = node->next;
    }
    return n;
}

/* Get the label string for a node */
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
        case AST_ARRAY_DECL: {
            /* Show dimensions inline: ArrayDecl(grid[3,3]) */
            char dims[128] = "";
            ASTNode *d = node->index;
            while(d) {
                char tmp[32];
                if(strlen(dims) > 0) strcat(dims, ",");
                if(d->value) strcat(dims, d->value);
                else strcat(dims, "?");
                d = d->next;
            }
            snprintf(buf, sz, "ArrayDecl(%s[%s])", node->value, dims);
            break;
        }
        case AST_ARRAY_ACCESS: {
            /* Show indices inline: ArrayAccess(grid[0,1]) */
            char idxs[128] = "";
            ASTNode *d = node->index;
            while(d) {
                if(strlen(idxs) > 0) strcat(idxs, ",");
                if(d->value) strcat(idxs, d->value);
                else strcat(idxs, "expr");
                d = d->next;
            }
            snprintf(buf, sz, "ArrayAccess(%s[%s])", node->value, idxs);
            break;
        }
        case AST_ARRAY_ASSIGN: {
            /* Show indices inline: ArrayAssign(grid[0,1]) */
            char idxs[128] = "";
            ASTNode *d = node->index;
            while(d) {
                if(strlen(idxs) > 0) strcat(idxs, ",");
                if(d->value) strcat(idxs, d->value);
                else strcat(idxs, "expr");
                d = d->next;
            }
            snprintf(buf, sz, "ArrayAssign(%s[%s])", node->value, idxs);
            break;
        }
        case AST_WATCH:        snprintf(buf, sz, "Watch(%s)", node->value); break;
        case AST_REWIND:       snprintf(buf, sz, "Rewind(%s)", node->value); break;
        case AST_PIPELINE:     snprintf(buf, sz, "Pipeline"); break;
        case AST_GC_COLLECT:   snprintf(buf, sz, "GC_Collect"); break;
        default:               snprintf(buf, sz, "Unknown"); break;
    }
}

/* Print a single node and its structural children (NOT ->next) */
void print_node(FILE *out, ASTNode *node, char *prefix, int is_last) {
    if(!node) return;

    fprintf(out, "%s", prefix);
    fprintf(out, "%s", is_last ? CORNER : BRANCH);

    char label[512];
    get_label(node, label, sizeof(label));
    fprintf(out, "%s\n", label);

    /* Build child prefix */
    char child_pfx[1024];
    snprintf(child_pfx, sizeof(child_pfx), "%s%s", prefix, is_last ? SPACE : PIPE);

    /* Collect structural children (NOT next - next is handled by the caller) */
    ASTNode *children[8];
    int cnt = 0;

    if(node->type == AST_PROG || node->type == AST_FUNC) {
        /* For Program/Function: print body statements as flat list */
        ASTNode *body = node->body ? node->body : node->left;
        if(node->type == AST_FUNC && node->left) {
            /* Parameters first */
            ASTNode *param = node->left;
            while(param) {
                if(param->type == AST_PARAM) {
                    int has_more = (param->next != NULL);
                    /* Check if body also follows */
                    if(!has_more && body && body != node->left) has_more = 1;
                    print_node(out, param, child_pfx, !has_more);
                }
                param = param->next;
            }
        }
        /* Print body as flat statement list */
        if(body) {
            ASTNode *stmts[256];
            int stmt_cnt = collect_chain(body, stmts, 256);
            for(int i = 0; i < stmt_cnt; i++) {
                print_node(out, stmts[i], child_pfx, (i == stmt_cnt - 1));
            }
        }
        return;
    }

    if(node->type == AST_VAR_DECL) {
        if(node->left) children[cnt++] = node->left;  /* Type */
        if(node->right) children[cnt++] = node->right; /* Init value */
    }
    else if(node->type == AST_ASSIGN) {
        /* Don't show left (ID) again since it's in the label */
        if(node->right) children[cnt++] = node->right;
    }
    else if(node->type == AST_ARRAY_DECL) {
        if(node->left) children[cnt++] = node->left;  /* Type */
        /* Dimensions already shown in label */
    }
    else if(node->type == AST_ARRAY_ASSIGN) {
        /* Indices already shown in label, show value */
        if(node->right) children[cnt++] = node->right;
    }
    else if(node->type == AST_ARRAY_ACCESS) {
        /* Indices already shown in label */
    }
    else if(node->type == AST_IF || node->type == AST_IF_ELSE) {
        if(node->cond) children[cnt++] = node->cond;
        /* Print body statements as flat list under "Then:" */
        if(node->body) {
            ASTNode *body_stmts[64];
            int body_cnt = collect_chain(node->body, body_stmts, 64);
            int has_else = (node->type == AST_IF_ELSE && node->else_body);

            fprintf(out, "%s%s", child_pfx, has_else ? BRANCH : CORNER);
            fprintf(out, "Then:\n");
            char then_pfx[1024];
            snprintf(then_pfx, sizeof(then_pfx), "%s%s", child_pfx, has_else ? PIPE : SPACE);
            for(int i = 0; i < body_cnt; i++) {
                print_node(out, body_stmts[i], then_pfx, (i == body_cnt - 1));
            }

            if(has_else) {
                ASTNode *else_stmts[64];
                int else_cnt = collect_chain(node->else_body, else_stmts, 64);
                fprintf(out, "%s%s", child_pfx, CORNER);
                fprintf(out, "Otherwise:\n");
                char else_pfx[1024];
                snprintf(else_pfx, sizeof(else_pfx), "%s%s", child_pfx, SPACE);
                for(int i = 0; i < else_cnt; i++) {
                    print_node(out, else_stmts[i], else_pfx, (i == else_cnt - 1));
                }
            }
            /* Print the condition child only */
            for(int i = 0; i < cnt; i++) {
                print_node(out, children[i], child_pfx, 1);
            }
            return;
        }
    }
    else if(node->type == AST_WHILE || node->type == AST_FOR) {
        if(node->cond) children[cnt++] = node->cond;
        /* Print loop body as flat list */
        if(node->body) {
            fprintf(out, "%s%s", child_pfx, CORNER);
            fprintf(out, "Body:\n");
            char body_pfx[1024];
            snprintf(body_pfx, sizeof(body_pfx), "%s%s", child_pfx, SPACE);
            ASTNode *body_stmts[64];
            int body_cnt = collect_chain(node->body, body_stmts, 64);
            for(int i = 0; i < body_cnt; i++) {
                print_node(out, body_stmts[i], body_pfx, (i == body_cnt - 1));
            }
            /* Print condition before Body */
            for(int i = 0; i < cnt; i++) {
                /* Already printed above via children array, print now */
            }
        }
    }
    else if(node->type == AST_PRINT) {
        if(node->left) children[cnt++] = node->left;
    }
    else if(node->type == AST_RETURN) {
        if(node->left) children[cnt++] = node->left;
    }
    else if(node->type == AST_BINOP) {
        if(node->left) children[cnt++] = node->left;
        if(node->right) children[cnt++] = node->right;
    }
    else if(node->type == AST_UNOP) {
        if(node->left) children[cnt++] = node->left;
    }
    else if(node->type == AST_PIPELINE) {
        if(node->left) children[cnt++] = node->left;
        if(node->right) children[cnt++] = node->right;
    }
    else if(node->type == AST_CALL) {
        if(node->left) children[cnt++] = node->left;
    }
    else {
        /* Generic: show all non-null pointers */
        if(node->left) children[cnt++] = node->left;
        if(node->right) children[cnt++] = node->right;
        if(node->index) children[cnt++] = node->index;
        if(node->cond) children[cnt++] = node->cond;
        if(node->body) children[cnt++] = node->body;
        if(node->else_body) children[cnt++] = node->else_body;
    }

    for(int i = 0; i < cnt; i++) {
        print_node(out, children[i], child_pfx, (i == cnt - 1));
    }
}

void print_ast(FILE *fp, ASTNode *n, int x) {
    if(!n) return;

    if(n->type == AST_PROG) {
        char label[256];
        get_label(n, label, sizeof(label));
        fprintf(fp, "%s\n", label);

        /* Print all functions as flat siblings */
        ASTNode *funcs[32];
        int func_cnt = collect_chain(n->left, funcs, 32);
        for(int i = 0; i < func_cnt; i++) {
            print_node(fp, funcs[i], "", (i == func_cnt - 1));
        }
    } else {
        print_node(fp, n, "", 1);
    }
}
