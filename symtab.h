#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"

void init_symtab();
void check_semantics(ASTNode *node);
int get_semantic_errors();
void print_symtab();

#endif
