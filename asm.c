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
char current_function_name[64] = "";

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

int current_reg = 0;

int alloc_reg() {
    if (current_reg > 6) {
        printf("    ; ERROR: Register spill\n");
        return 6; 
    }
    return current_reg++;
}

void free_reg() {
    if (current_reg > 0) current_reg--;
}

int generateExprAsm(ASTNode *node) {
    if(!node) return -1;

    if(node->type == AST_NUM) {
        int r = alloc_reg();
        printf("    li t%d, %s\n", r, node->value);
        return r;
    }

    if(node->type == AST_ID) {
        int off = get_offset(node->value);
        int r = alloc_reg();
        printf("    lw t%d, -%d(s0)\n", r, off);
        return r;
    }

    if(node->type == AST_BINOP) {
        int r1 = generateExprAsm(node->left);
        int r2 = generateExprAsm(node->right);
        
        free_reg();
        
        if(strcmp(node->value, "+") == 0) {
            printf("    add t%d, t%d, t%d\n", r1, r1, r2);
        } else if(strcmp(node->value, "-") == 0) {
            printf("    sub t%d, t%d, t%d\n", r1, r1, r2);
        } else if(strcmp(node->value, "*") == 0) {
            printf("    mul t%d, t%d, t%d\n", r1, r1, r2);
        } else if(strcmp(node->value, "/") == 0) {
            printf("    div t%d, t%d, t%d\n", r1, r1, r2);
        } else if(strcmp(node->value, "==") == 0) {
            printf("    sub t%d, t%d, t%d\n", r1, r1, r2);
            printf("    seqz t%d, t%d\n", r1, r1);
        } else if(strcmp(node->value, "!=") == 0) {
            printf("    sub t%d, t%d, t%d\n", r1, r1, r2);
            printf("    snez t%d, t%d\n", r1, r1);
        } else if(strcmp(node->value, ">") == 0) {
            printf("    slt t%d, t%d, t%d\n", r1, r2, r1);
        } else if(strcmp(node->value, "<") == 0) {
            printf("    slt t%d, t%d, t%d\n", r1, r1, r2);
        } else if(strcmp(node->value, ">=") == 0) {
            printf("    slt t%d, t%d, t%d\n", r1, r1, r2);
            printf("    xori t%d, t%d, 1\n", r1, r1);
        } else if(strcmp(node->value, "<=") == 0) {
            printf("    slt t%d, t%d, t%d\n", r1, r2, r1);
            printf("    xori t%d, t%d, 1\n", r1, r1); 
        }
        
        return r1;
    }

    if(node->type == AST_UNOP) {
        int r = generateExprAsm(node->left);
        
        if(strcmp(node->value, "!") == 0) {
            printf("    seqz t%d, t%d\n", r, r);
        } else if(strcmp(node->value, "-") == 0) {
            printf("    neg t%d, t%d\n", r, r);
        } else if(strcmp(node->value, "++") == 0 || strcmp(node->value, "++ (post)") == 0) {
            if (strcmp(node->value, "++ (post)") == 0) {
                int r2 = alloc_reg();
                printf("    addi t%d, t%d, 1\n", r2, r);
                if(node->left->type == AST_ID) {
                    int off = get_offset(node->left->value);
                    printf("    sw t%d, -%d(s0)\n", r2, off);
                }
                free_reg();
            } else {
                printf("    addi t%d, t%d, 1\n", r, r);
                if(node->left->type == AST_ID) {
                    int off = get_offset(node->left->value);
                    printf("    sw t%d, -%d(s0)\n", r, off);
                }
            }
        } else if(strcmp(node->value, "--") == 0 || strcmp(node->value, "-- (post)") == 0) {
            if (strcmp(node->value, "-- (post)") == 0) {
                int r2 = alloc_reg();
                printf("    addi t%d, t%d, -1\n", r2, r);
                if(node->left->type == AST_ID) {
                    int off = get_offset(node->left->value);
                    printf("    sw t%d, -%d(s0)\n", r2, off);
                }
                free_reg();
            } else {
                printf("    addi t%d, t%d, -1\n", r, r);
                if(node->left->type == AST_ID) {
                    int off = get_offset(node->left->value);
                    printf("    sw t%d, -%d(s0)\n", r, off);
                }
            }
        }
        
        return r;
    }
    
    if(node->type == AST_CALL) {
        ASTNode *arg = node->left;
        int arg_idx = 0;
        int arg_regs[8]; 
        while(arg) {
            arg_regs[arg_idx++] = generateExprAsm(arg->left);
            arg = arg->next;
        }
        for(int i=0; i<arg_idx; i++) {
            printf("    mv a%d, t%d\n", i, arg_regs[i]);
            free_reg(); 
        }
        int saved_regs = current_reg;
        for(int i=0; i<saved_regs; i++) {
            printf("    addi sp, sp, -4\n");
            printf("    sw t%d, 0(sp)\n", i);
        }
        
        printf("    call %s\n", node->value);
        
        for(int i=saved_regs-1; i>=0; i--) {
            printf("    lw t%d, 0(sp)\n", i);
            printf("    addi sp, sp, 4\n");
        }
        
        int r = alloc_reg();
        printf("    mv t%d, a0\n", r);
        return r;
    }

    return -1;
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
        printf("    j main\n");
        generateStmtAsm(node->left);
        return NULL;
    }

    if(node->type == AST_FUNC) {
        printf("\n%s:\n", node->value);
        printf("    addi sp, sp, -256\n");
        printf("    sw ra, 252(sp)\n");
        printf("    sw s0, 248(sp)\n");
        printf("    addi s0, sp, 256\n");
        int old_off = current_offset;
        VarMap *old_vars = vars;
        vars = NULL;
        current_offset = 8;
        char old_func_name[64];
        strncpy(old_func_name, current_function_name, 63);
        strncpy(current_function_name, node->value, 63);
        
        ASTNode *p = node->left;
        int param_idx = 0;
        while(p) {
            int off = get_offset(p->value);
            printf("    sw a%d, -%d(s0)\n", param_idx++, off);
            p = p->next;
        }

        generateStmtAsm(node->body);

        if (strcmp(node->value, "main") == 0) {
            printf("    li a7, 10\n");
            printf("    ecall\n");
        } else {
            printf("    lw ra, 252(sp)\n");
            printf("    lw s0, 248(sp)\n");
            printf("    addi sp, sp, 256\n");
            printf("    jr ra\n");
        }
        
        vars = old_vars;
        current_offset = old_off;
        strncpy(current_function_name, old_func_name, 63);
        
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_VAR_DECL) {
        if(node->right) {
            int r = generateExprAsm(node->right);
            int off = get_offset(node->value);
            printf("    sw t%d, -%d(s0)\n", r, off);
            free_reg();
        }
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_ASSIGN) {
        int r = generateExprAsm(node->right);
        int off = get_offset(node->left->value);
        printf("    sw t%d, -%d(s0)\n", r, off);
        free_reg();
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_IF) {
        int r = generateExprAsm(node->cond);
        int l_end = asm_label_counter++;
        printf("    beqz t%d, .L%d\n", r, l_end);
        free_reg();
        generateStmtAsm(node->body);
        printf(".L%d:\n", l_end);
        return generateStmtAsm(node->next);
    }
    
    if(node->type == AST_IF_ELSE) {
        int r = generateExprAsm(node->cond);
        int l_false = asm_label_counter++;
        int l_end = asm_label_counter++;
        printf("    beqz t%d, .L%d\n", r, l_false);
        free_reg();
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
        int r = generateExprAsm(node->cond);
        printf("    beqz t%d, .L%d\n", r, l_end);
        free_reg();
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
            int r = generateExprAsm(node->cond);
            printf("    beqz t%d, .L%d\n", r, l_end);
            free_reg();
        }
        generateStmtAsm(node->body);
        
        if (node->right) { 
            if (node->right->type == AST_ASSIGN) {
                int r = generateExprAsm(node->right->right);
                int off = get_offset(node->right->left->value);
                printf("    sw t%d, -%d(s0)\n", r, off);
                free_reg();
            } else if (node->right->type == AST_UNOP) {
                int r = generateExprAsm(node->right);
                free_reg();
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
            int r = generateExprAsm(node->left);
            printf("    mv a0, t%d\n", r);
            printf("    li a7, 1\n");
            printf("    ecall\n");
            free_reg();
        }
        
        printf("    li a0, 10\n");
        printf("    li a7, 11\n");
        printf("    ecall\n");
        
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_RETURN) {
        if (node->left) {
            int r = generateExprAsm(node->left);
            printf("    mv a0, t%d\n", r);
            free_reg();
        }
        
        if(strcmp(current_function_name, "main") == 0) {
            printf("    li a7, 10\n");
            printf("    ecall\n");
        } else {
            printf("    lw ra, 252(sp)\n");
            printf("    lw s0, 248(sp)\n");
            printf("    addi sp, sp, 256\n");
            printf("    jr ra\n");
        }
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_UNOP || node->type == AST_BINOP || node->type == AST_CALL) {
        int r = generateExprAsm(node);
        if (r != -1) {
            free_reg();
        }
        return generateStmtAsm(node->next);
    }

    generateStmtAsm(node->next);
    return NULL;
}

void generateAssembly(ASTNode *node) {
    vars = NULL;
    current_offset = 8;
    asm_label_counter = 0;
    str_count = 0;
    collectStrings(node);
    generateStmtAsm(node);
}
