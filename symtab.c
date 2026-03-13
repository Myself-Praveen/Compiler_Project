#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"

typedef struct Symbol {
    char name[64];
    char type[32];
    int scope;
    int is_active;
    struct Symbol *next;
} Symbol;

Symbol *symtab_head = NULL;
int current_scope = 0;
int semantic_errors = 0;

void init_symtab() {
    symtab_head = NULL;
    current_scope = 0;
    semantic_errors = 0;
}

void enter_scope() {
    current_scope++;
}

void exit_scope() {
    Symbol *curr = symtab_head;
    while(curr != NULL) {
        if(curr->scope == current_scope && curr->is_active) {
            curr->is_active = 0;
        }
        curr = curr->next;
    }
    current_scope--;
}

int add_symbol(const char *name, const char *type) {
    Symbol *curr = symtab_head;
    while(curr != NULL) {
        if(strcmp(curr->name, name) == 0 && curr->scope == current_scope && curr->is_active) {
            return 0; 
        }
        curr = curr->next;
    }
    
    Symbol *new_sym = (Symbol*)malloc(sizeof(Symbol));
    strncpy(new_sym->name, name, 63);
    strncpy(new_sym->type, type, 31);
    new_sym->scope = current_scope;
    new_sym->is_active = 1;
    new_sym->next = symtab_head;
    symtab_head = new_sym;
    
    return 1;
}

Symbol* find_symbol(const char *name) {
    Symbol *curr = symtab_head;
    Symbol *found = NULL;
    int max_scope = -1;
    while(curr != NULL) {
        if(strcmp(curr->name, name) == 0 && curr->is_active) {
            if(curr->scope > max_scope) {
                max_scope = curr->scope;
                found = curr;
            }
        }
        curr = curr->next;
    }
    return found;
}

const char* get_expr_type(ASTNode *node) {
    if(!node) return "unknown";
    if(node->type == AST_NUM) {
        if(strchr(node->value, '.')) return "float";
        return "int";
    }
    if(node->type == AST_STR) {
        return "string";
    }
    if(node->type == AST_ID) {
        Symbol *sym = find_symbol(node->value);
        if(sym) return sym->type;
        return "unknown";
    }
    if(node->type == AST_BINOP) {
        const char *ltype = get_expr_type(node->left);
        const char *rtype = get_expr_type(node->right);
        if(strcmp(ltype, "float") == 0 || strcmp(rtype, "float") == 0) return "float";
        return "int";
    }
    if(node->type == AST_UNOP) {
        return get_expr_type(node->left);
    }
    return "unknown";
}


void check_semantics(ASTNode *node) {
    if(!node) return;

    if(node->type == AST_PROG) {
        check_semantics(node->left);
        return;
    }

    if(node->type == AST_RETURN) {
        check_semantics(node->left);
        return;
    }
    if(node->type == AST_PRINT) {
        check_semantics(node->left);
        return;
    }

    if(node->type == AST_FUNC) {
        add_symbol(node->value, "Function");
        enter_scope();
        ASTNode *p = node->left;
        while(p) {
            if (p->type == AST_PARAM) {
                add_symbol(p->value, p->left ? p->left->value : "unknown");
            }
            p = p->next;
        }
        check_semantics(node->body);
        exit_scope();
        check_semantics(node->next);
        return;
    }
    
    if (node->type == AST_CALL) {
        if(!find_symbol(node->value)) {
            // Optionally check if function exists, but we skip strict checks right now
        }
        ASTNode *arg = node->left;
        while(arg) {
            check_semantics(arg->left);
            arg = arg->next;
        }
        check_semantics(node->next);
        return;
    }
    
    if(node->type == AST_FOR || node->type == AST_WHILE || node->type == AST_IF || node->type == AST_IF_ELSE) {
        check_semantics(node->left);
        check_semantics(node->cond); 
        check_semantics(node->right);
        
        enter_scope();
        check_semantics(node->body);
        check_semantics(node->else_body);
        exit_scope();
        
        check_semantics(node->next);
        return;
    }

    if(node->type == AST_VAR_DECL) {
        char *decl_type = node->left ? node->left->value : "unknown";
        if(!add_symbol(node->value, decl_type)) {
            printf("Semantic Error: Multiple declarations of '%s' in same scope.\n", node->value);
            semantic_errors++;
        }
        
        if(node->right) {
            const char* expr_type = get_expr_type(node->right);
            if(strcmp(expr_type, "string") == 0 && strcmp(decl_type, "string") != 0) {
                printf("Semantic Error: Type Mismatch - Cannot assign 'string' to '%s' variable '%s'.\n", decl_type, node->value);
                semantic_errors++;
            }
        }

        check_semantics(node->left);
        check_semantics(node->right);
        check_semantics(node->next);
        return;
    }

    if(node->type == AST_ASSIGN) {
        if(node->left && node->left->type == AST_ID) {
            Symbol *sym = find_symbol(node->left->value);
            if(!sym) {
                printf("Semantic Error: Assignment to undeclared variable '%s'.\n", node->left->value);
                semantic_errors++;
            } else {
                const char* expr_type = get_expr_type(node->right);
                if(strcmp(expr_type, "string") == 0 && strcmp(sym->type, "string") != 0) {
                    printf("Semantic Error: Type Mismatch - Cannot assign 'string' to '%s' variable '%s'.\n", sym->type, sym->name);
                    semantic_errors++;
                }
            }
        }
        check_semantics(node->right);
        check_semantics(node->next);
        return;
    }

    if(node->type == AST_ID) {
        if(!find_symbol(node->value)) {
            printf("Semantic Error: Usage of undeclared identifier '%s'.\n", node->value);
            semantic_errors++;
        }
        check_semantics(node->next);
        return;
    }

    check_semantics(node->left);
    check_semantics(node->right);
    check_semantics(node->cond);
    check_semantics(node->body);
    check_semantics(node->else_body);
    check_semantics(node->next);
}

int get_semantic_errors() {
    return semantic_errors;
}

void print_symtab() {
    printf("%-20s %-15s %-10s %-10s\n", "NAME", "KIND", "SCOPE", "STATUS");
    printf("----------------------------------------------------------\n");
    
    Symbol *curr = symtab_head;
    int count = 0;
    while(curr) { count++; curr = curr->next; }
    
    if(count == 0) return;

    Symbol **arr = (Symbol**)malloc(count * sizeof(Symbol*));
    curr = symtab_head;
    for(int i=count-1; i>=0; i--) { arr[i] = curr; curr = curr->next; }
    
    for(int i=0; i<count; i++) {
        printf("%-20s %-15s %-10d %-10s\n", arr[i]->name, arr[i]->type, arr[i]->scope, arr[i]->is_active ? "Active" : "Closed");
    }
    free(arr);
}
