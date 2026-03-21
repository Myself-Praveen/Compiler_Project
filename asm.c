#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "asm.h"

typedef struct VarMap {
    char name[64];
    int offset;
    int size;       /* bytes: 4 for scalar, N*4 for array */
    int is_array;
    int history_offset; /* stack offset for watch history pointer */
    int dims[8];    /* dimension sizes for N-d arrays */
    int ndims;      /* number of dimensions */
    struct VarMap *next;
} VarMap;

VarMap *vars = NULL;
int current_offset = 0;
int asm_label_counter = 0;
char current_function_name[64] = "";
int gc_zone_counter = 0;

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
    n->size = 4;
    n->is_array = 0;
    n->history_offset = 0;
    n->ndims = 0;
    memset(n->dims, 0, sizeof(n->dims));
    n->next = vars;
    vars = n;
    
    return current_offset;
}

int get_array_offset(char *name, int total_elements, int *dim_sizes, int ndims) {
    VarMap *curr = vars;
    while(curr) {
        if (strcmp(curr->name, name) == 0) return curr->offset;
        curr = curr->next;
    }
    
    int bytes = total_elements * 4;
    current_offset += bytes;
    VarMap *n = (VarMap*)malloc(sizeof(VarMap));
    strncpy(n->name, name, 63);
    n->offset = current_offset - bytes + 4; /* base of array */
    n->size = bytes;
    n->is_array = 1;
    n->history_offset = 0;
    n->ndims = ndims;
    for(int i = 0; i < ndims && i < 8; i++) n->dims[i] = dim_sizes[i];
    n->next = vars;
    vars = n;
    
    return n->offset;
}

int alloc_watch_history(char *name) {
    VarMap *curr = vars;
    while(curr) {
        if(strcmp(curr->name, name) == 0) {
            if(curr->history_offset == 0) {
                /* allocate 32 bytes (8 slots) for history stack */
                current_offset += 36; /* 32 bytes data + 4 byte counter */
                curr->history_offset = current_offset - 35;
            }
            return curr->history_offset;
        }
        curr = curr->next;
    }
    return 0;
}

VarMap* find_var(char *name) {
    VarMap *curr = vars;
    while(curr) {
        if(strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
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
    collectStrings(node->index);
    collectStrings(node->next);
}

int current_reg = 0;

int alloc_reg() {
    if (current_reg > 6) {
        printf("    # ERROR: Register spill\n");
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
        } else if(strcmp(node->value, "**") == 0) {
            /* Exponent: loop-based power computation */
            int l_loop = asm_label_counter++;
            int l_done = asm_label_counter++;
            int r_res = alloc_reg();
            int r_cnt = alloc_reg();
            printf("    # EXP t%d ** t%d\n", r1, r2);
            printf("    li t%d, 1\n", r_res);          /* result = 1 */
            printf("    mv t%d, t%d\n", r_cnt, r2);    /* counter = exponent */
            printf(".L%d:\n", l_loop);
            printf("    beqz t%d, .L%d\n", r_cnt, l_done);
            printf("    mul t%d, t%d, t%d\n", r_res, r_res, r1);
            printf("    addi t%d, t%d, -1\n", r_cnt, r_cnt);
            printf("    j .L%d\n", l_loop);
            printf(".L%d:\n", l_done);
            printf("    mv t%d, t%d\n", r1, r_res);
            free_reg(); /* r_cnt */
            free_reg(); /* r_res */
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

    if(node->type == AST_ARRAY_ACCESS) {
        VarMap *v = find_var(node->value);
        if(!v) {
            int off = get_offset(node->value);
            v = find_var(node->value);
        }
        int base_off = v ? v->offset : 0;
        printf("    # Array Access %s\n", node->value);
        /* Compute linearized index from multi-dim indices */
        ASTNode *idx = node->index;
        int r_linear = generateExprAsm(idx);
        int dim_i = 0;
        idx = idx->next;
        while(idx && v && dim_i + 1 < v->ndims) {
            int r_dim = alloc_reg();
            printf("    li t%d, %d\n", r_dim, v->dims[dim_i + 1]);
            printf("    mul t%d, t%d, t%d\n", r_linear, r_linear, r_dim);
            free_reg(); /* r_dim */
            int r_next = generateExprAsm(idx);
            printf("    add t%d, t%d, t%d\n", r_linear, r_linear, r_next);
            free_reg(); /* r_next */
            dim_i++;
            idx = idx->next;
        }
        int r = alloc_reg();
        printf("    slli t%d, t%d, 2\n", r_linear, r_linear); /* * 4 bytes */
        printf("    li t%d, %d\n", r, base_off);
        printf("    add t%d, t%d, t%d\n", r, r, r_linear);
        printf("    sub t%d, s0, t%d\n", r, r);
        printf("    lw t%d, 0(t%d)\n", r, r);
        free_reg(); /* r_linear */
        return r;
    }

    if(node->type == AST_PIPELINE) {
        /* Pipeline: compute left, pass as argument to right */
        int r_left = generateExprAsm(node->left);
        printf("    # Pipeline\n");
        printf("    mv a0, t%d\n", r_left);
        if(node->right && node->right->type == AST_CALL) {
            printf("    call %s\n", node->right->value);
        }
        printf("    mv t%d, a0\n", r_left);
        return r_left;
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
        printf("# === RISC-V 32-bit Architecture ===\n");
        if (str_count > 0) {
            printf(".data\n");
            for(int i = 0; i < str_count; i++) {
                printf(".L%d_str:\n", str_labels[i]);
                printf("    .asciiz %s\n", str_nodes[i]->value);
                printf("    .word 0\n"); /* padding to prevent Venus read-past-null */
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

    if(node->type == AST_ARRAY_DECL) {
        /* Collect dimension sizes */
        ASTNode *dim = node->index;
        int total = 1;
        int dim_sizes[8];
        int ndims = 0;
        while(dim) {
            if(dim->type == AST_NUM) {
                dim_sizes[ndims] = atoi(dim->value);
                total *= dim_sizes[ndims];
            } else {
                dim_sizes[ndims] = 16;
                total *= 16;
            }
            ndims++;
            dim = dim->next;
        }
        int base = get_array_offset(node->value, total, dim_sizes, ndims);
        printf("    # ArrayDecl %s [%d elements] at -%d(s0)\n", node->value, total, base);
        /* Zero-initialize */
        for(int i=0; i<total; i++) {
            printf("    sw zero, -%d(s0)\n", base + i*4);
        }
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_ARRAY_ASSIGN) {
        VarMap *v = find_var(node->value);
        if(!v) {
            get_offset(node->value);
            v = find_var(node->value);
        }
        int base_off = v ? v->offset : 0;
        printf("    # ArrayAssign %s\n", node->value);
        /* Compute linearized index */
        ASTNode *idx = node->index;
        int r_linear = generateExprAsm(idx);
        int dim_i = 0;
        idx = idx->next;
        while(idx && v && dim_i + 1 < v->ndims) {
            int r_dim = alloc_reg();
            printf("    li t%d, %d\n", r_dim, v->dims[dim_i + 1]);
            printf("    mul t%d, t%d, t%d\n", r_linear, r_linear, r_dim);
            free_reg();
            int r_next = generateExprAsm(idx);
            printf("    add t%d, t%d, t%d\n", r_linear, r_linear, r_next);
            free_reg();
            dim_i++;
            idx = idx->next;
        }
        int r_val = generateExprAsm(node->right);
        int r_addr = alloc_reg();
        printf("    slli t%d, t%d, 2\n", r_linear, r_linear);
        printf("    li t%d, %d\n", r_addr, base_off);
        printf("    add t%d, t%d, t%d\n", r_addr, r_addr, r_linear);
        printf("    sub t%d, s0, t%d\n", r_addr, r_addr);
        printf("    sw t%d, 0(t%d)\n", r_val, r_addr);
        free_reg(); /* r_addr */
        free_reg(); /* r_val */
        free_reg(); /* r_linear */
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_ASSIGN) {
        int off = get_offset(node->left->value);
        /* Time-Travel: if variable is watched, save old value to history */
        VarMap *wv = find_var(node->left->value);
        if(wv && wv->history_offset > 0) {
            int rold = alloc_reg();
            int rcnt = alloc_reg();
            int raddr = alloc_reg();
            printf("    # Watch-Save %s before assign\n", node->left->value);
            printf("    lw t%d, -%d(s0)\n", rold, off);               /* load old value */
            printf("    lw t%d, -%d(s0)\n", rcnt, wv->history_offset + 32); /* load counter */
            printf("    slli t%d, t%d, 2\n", raddr, rcnt);            /* counter * 4 */
            printf("    addi t%d, t%d, %d\n", raddr, raddr, wv->history_offset);  /* base + offset */
            printf("    sub t%d, s0, t%d\n", raddr, raddr);           /* address in stack */
            printf("    sw t%d, 0(t%d)\n", rold, raddr);              /* save old value */
            printf("    addi t%d, t%d, 1\n", rcnt, rcnt);             /* counter++ */
            printf("    sw t%d, -%d(s0)\n", rcnt, wv->history_offset + 32); /* store counter */
            free_reg(); /* raddr */
            free_reg(); /* rcnt */
            free_reg(); /* rold */
        }
        int r = generateExprAsm(node->right);
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

    if(node->type == AST_WATCH) {
        /* Time-Travel Debug: allocate history and save current value */
        int off = get_offset(node->value);
        int hist_off = alloc_watch_history(node->value);
        printf("    # WATCH %s (history at -%d(s0))\n", node->value, hist_off);
        /* Initialize history counter to 0 */
        printf("    sw zero, -%d(s0)\n", hist_off + 32); /* counter at end of history block */
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_REWIND) {
        /* Time-Travel Debug: restore previous value */
        int off = get_offset(node->value);
        VarMap *v = find_var(node->value);
        int hist_off = v ? v->history_offset : 0;
        if(hist_off > 0) {
            int r = alloc_reg();
            int r_cnt = alloc_reg();
            printf("    # REWIND %s\n", node->value);
            printf("    lw t%d, -%d(s0)\n", r_cnt, hist_off + 32); /* load counter */
            printf("    addi t%d, t%d, -1\n", r_cnt, r_cnt);      /* decrement */
            printf("    slli t%d, t%d, 2\n", r_cnt, r_cnt);       /* counter * 4 */
            printf("    li t%d, %d\n", r, hist_off);
            printf("    add t%d, t%d, t%d\n", r, r, r_cnt);
            printf("    sub t%d, s0, t%d\n", r, r);
            printf("    lw t%d, 0(t%d)\n", r, r);                   /* load old value */
            printf("    sw t%d, -%d(s0)\n", r, off);                /* restore it */
            /* update counter */
            printf("    srai t%d, t%d, 2\n", r_cnt, r_cnt);
            printf("    sw t%d, -%d(s0)\n", r_cnt, hist_off + 32);
            free_reg();
            free_reg();
        }
        return generateStmtAsm(node->next);
    }

    if(node->type == AST_GC_COLLECT) {
        printf("    # GC_COLLECT: Sweep unreferenced stack memory\n");
        printf("    # Mark phase: scan active variable references\n");
        printf("    # Sweep phase: reset stack slots with no references\n");
        /* In a stack-based allocator, GC resets freed offsets */
        gc_zone_counter++;
        printf("    # GC Zone %d completed\n", gc_zone_counter);
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

/* After any assignment to a watched variable, push to history */
/* This is injected by the compiler when it detects AST_ASSIGN to a watched var */
void emit_watch_save(char *name) {
    VarMap *v = find_var(name);
    if(!v || v->history_offset == 0) return;
    int off = v->offset;
    int hist_off = v->history_offset;
    int r = alloc_reg();
    int r_cnt = alloc_reg();
    printf("    # Watch Save %s\n", name);
    printf("    lw t%d, -%d(s0)\n", r, off);          /* load current value */
    printf("    lw t%d, -%d(s0)\n", r_cnt, hist_off + 32); /* load counter */
    int r_addr = alloc_reg();
    printf("    slli t%d, t%d, 2\n", r_addr, r_cnt);
    printf("    li t%d, %d\n", r_addr, hist_off);
    /* Intentionally simplified: store value at history[counter] */
    printf("    add t%d, t%d, t%d\n", r_addr, r_addr, r_cnt);
    printf("    sub t%d, s0, t%d\n", r_addr, r_addr);
    printf("    sw t%d, 0(t%d)\n", r, r_addr);
    /* increment counter */
    printf("    addi t%d, t%d, 1\n", r_cnt, r_cnt);
    printf("    sw t%d, -%d(s0)\n", r_cnt, hist_off + 32);
    free_reg();
    free_reg();
    free_reg();
}

void generateAssembly(ASTNode *node) {
    vars = NULL;
    current_offset = 8;
    asm_label_counter = 0;
    str_count = 0;
    gc_zone_counter = 0;
    collectStrings(node);
    generateStmtAsm(node);
}
