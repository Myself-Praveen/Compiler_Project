#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "asm.h"

typedef struct VarMap {
    char name[64];
    int offset;
    struct VarMap *next;
} VarMap;

VarMap *vars = NULL;
int current_offset = 0;
int asm_label_counter = 0;

int get_offset(char *name) {
    VarMap *curr = vars;
    while(curr) {
        if (strcmp(curr->name, name) == 0) return curr->offset;
        curr = curr->next;
    }
    
    current_offset += 4;
    VarMap *n = (VarMap*)malloc(sizeof(VarMap));
    strncpy(n->name, name, 63);
    n->offset = current_offset;
    n->next = vars;
    vars = n;
    
    return current_offset;
}

ASTNode *str_nodes[128];
int str_labels[128];
int str_count = 0;

void collectStrings(ASTNode *node) {
    if(!node) return;
    if(node->type == AST_PRINT && node->left && node->left->type == AST_STR) {
        str_nodes[str_count] = node->left;
        str_labels[str_count] = asm_label_counter++;
        str_count++;
    }
    collectStrings(node->left);
    collectStrings(node->right);
    collectStrings(node->cond);
    collectStrings(node->body);
    collectStrings(node->else_body);
    collectStrings(node->next);
}

void generateExprAsm(ASTNode *node) {
    if(!node) return;

    if(node->type == AST_NUM) {
        printf("    li a0, %s\n", node->value);
        printf("    addi sp, sp, -4\n");
        printf("    sw a0, 0(sp)\n"); // push a0
        return;
    }

    if(node->type == AST_ID) {
        int off = get_offset(node->value);
        printf("    lw a0, -%d(s0)\n", off);
        printf("    addi sp, sp, -4\n");
        printf("    sw a0, 0(sp)\n"); // push a0
        return;
    }

    if(node->type == AST_BINOP) {
        generateExprAsm(node->left);
        generateExprAsm(node->right);
        
        printf("    lw a1, 0(sp)\n"); // pop right into a1
        printf("    addi sp, sp, 4\n");
        printf("    lw a0, 0(sp)\n"); // pop left into a0
        printf("    addi sp, sp, 4\n");
        
        if(strcmp(node->value, "+") == 0) {
            printf("    add a0, a0, a1\n");
        } else if(strcmp(node->value, "-") == 0) {
            printf("    sub a0, a0, a1\n");
        } else if(strcmp(node->value, "*") == 0) {
            printf("    mul a0, a0, a1\n");
        } else if(strcmp(node->value, "/") == 0) {
            printf("    div a0, a0, a1\n");
        } else if(strcmp(node->value, "==") == 0) {
            printf("    sub a0, a0, a1\n");
            printf("    seqz a0, a0\n");
        } else if(strcmp(node->value, "!=") == 0) {
            printf("    sub a0, a0, a1\n");
            printf("    snez a0, a0\n");
        } else if(strcmp(node->value, ">") == 0) {
            printf("    slt a0, a1, a0\n");
        } else if(strcmp(node->value, "<") == 0) {
            printf("    slt a0, a0, a1\n");
        } else if(strcmp(node->value, ">=") == 0) {
            printf("    slt a0, a0, a1\n");
            printf("    xori a0, a0, 1\n");
        } else if(strcmp(node->value, "<=") == 0) {
            printf("    slt a0, a1, a0\n");
            printf("    xori a0, a0, 1\n"); 
        }
        
        printf("    addi sp, sp, -4\n");
        printf("    sw a0, 0(sp)\n"); // push result
        return;
    }

    if(node->type == AST_UNOP) {
        generateExprAsm(node->left);
        printf("    lw a0, 0(sp)\n");
        printf("    addi sp, sp, 4\n");
        
        if(strcmp(node->value, "!") == 0) {
            printf("    seqz a0, a0\n");
        } else if(strcmp(node->value, "-") == 0) {
            printf("    neg a0, a0\n");
        }
        
        printf("    addi sp, sp, -4\n");
        printf("    sw a0, 0(sp)\n"); // push result
        return;
    }
}

char* generateStmtAsm(ASTNode *node) {
    if(!node) return NULL;

    if(node->type == AST_PROG) {
        printf("; === RISC-V 32-bit Architecture ===\n");
        if (str_count > 0) {
            printf(".data\n");
            for(int i = 0; i < str_count; i++) {
                printf(".L%d_str:\n", str_labels[i]);
                printf("    .asciiz %s\n", str_nodes[i]->value);
                printf("    .byte 0\n");
            }
        }
        printf(".text\n");
        printf(".globl main\n");
        generateStmtAsm(node->left);
        return NULL;
    }

    if(node->type == AST_FUNC) {
        printf("\n%s:\n", node->value);
        printf("    addi sp, sp, -256\n"); // Allocate 256 byte stack frame
        printf("    sw s0, 252(sp)\n");    // Save old frame pointer
        printf("    addi s0, sp, 256\n");  // Set new frame pointer
        generateStmtAsm(node->body);
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_VAR_DECL) {
        if(node->right) {
            generateExprAsm(node->right);
            int off = get_offset(node->value);
            printf("    lw a0, 0(sp)\n");
            printf("    addi sp, sp, 4\n"); // pop
            printf("    sw a0, -%d(s0)\n", off);
        }
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_ASSIGN) {
        generateExprAsm(node->right);
        int off = get_offset(node->left->value);
        printf("    lw a0, 0(sp)\n");
        printf("    addi sp, sp, 4\n"); // pop
        printf("    sw a0, -%d(s0)\n", off);
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_IF) {
        generateExprAsm(node->cond);
        int l_end = asm_label_counter++;
        printf("    lw a0, 0(sp)\n");
        printf("    addi sp, sp, 4\n");
        printf("    beqz a0, .L%d\n", l_end);
        generateStmtAsm(node->body);
        printf(".L%d:\n", l_end);
        return generateStmtAsm(node->next);
    }
    
    if(node->type == AST_IF_ELSE) {
        generateExprAsm(node->cond);
        int l_false = asm_label_counter++;
        int l_end = asm_label_counter++;
        printf("    lw a0, 0(sp)\n");
        printf("    addi sp, sp, 4\n");
        printf("    beqz a0, .L%d\n", l_false);
        generateStmtAsm(node->body); 
        printf("    j .L%d\n", l_end);
        printf(".L%d:\n", l_false);
        generateStmtAsm(node->else_body); 
        printf(".L%d:\n", l_end);
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_WHILE) {
        int l_start = asm_label_counter++;
        int l_end = asm_label_counter++;
        printf(".L%d:\n", l_start);
        generateExprAsm(node->cond);
        printf("    lw a0, 0(sp)\n");
        printf("    addi sp, sp, 4\n");
        printf("    beqz a0, .L%d\n", l_end);
        generateStmtAsm(node->body);
        printf("    j .L%d\n", l_start);
        printf(".L%d:\n", l_end);
        return generateStmtAsm(node->next);
    }
    
    if(node->type == AST_FOR) {
        generateStmtAsm(node->left); 
        int l_start = asm_label_counter++;
        int l_end = asm_label_counter++;
        printf(".L%d:\n", l_start);
        if(node->cond) {
            generateExprAsm(node->cond);
            printf("    lw a0, 0(sp)\n");
            printf("    addi sp, sp, 4\n");
            printf("    beqz a0, .L%d\n", l_end);
        }
        generateStmtAsm(node->body);
        
        if (node->right) { 
            if (node->right->type == AST_ASSIGN) {
                generateExprAsm(node->right->right);
                int off = get_offset(node->right->left->value);
                printf("    lw a0, 0(sp)\n");
                printf("    addi sp, sp, 4\n");
                printf("    sw a0, -%d(s0)\n", off);
            } else if (node->right->type == AST_UNOP) {
                generateExprAsm(node->right);
                printf("    lw a0, 0(sp)\n");
                printf("    addi sp, sp, 4\n"); // drop the value
            } else {
                generateStmtAsm(node->right);
            }
        }
        printf("    j .L%d\n", l_start);
        printf(".L%d:\n", l_end);
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_PRINT) {
        if (node->left && node->left->type == AST_STR) {
            int str_idx = -1;
            for(int i=0; i<str_count; i++) {
                if(str_nodes[i] == node->left) {
                    str_idx = str_labels[i];
                    break;
                }
            }
            printf("    la a0, .L%d_str\n", str_idx);
            printf("    li a7, 4\n");
            printf("    ecall\n");
        } else {
            generateExprAsm(node->left);
            printf("    lw a0, 0(sp)\n");
            printf("    addi sp, sp, 4\n");
            printf("    li a7, 1\n");
            printf("    ecall\n");
        }
        
        printf("    li a0, 10\n");
        printf("    li a7, 11\n");
        printf("    ecall\n");
        
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_RETURN) {
        printf("    li a7, 10\n");
        printf("    ecall\n");
        return generateStmtAsm(node->next);
    }

    generateStmtAsm(node->next);
    return NULL;
}

void generateAssembly(ASTNode *node) {
    vars = NULL;
    current_offset = 0;
    asm_label_counter = 0;
    str_count = 0;
    collectStrings(node);
    generateStmtAsm(node);
}
