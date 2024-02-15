#ifndef ACTIONS_H
#define ACTIONS_H

#include "handle_list.h"

void initialize_actions();
void finalize_actions();
char *toplevel_expression(struct HandleList composite);
struct HandleList expression_list_base(struct HandleList base);
struct HandleList expression_list_recursion(struct HandleList list, struct HandleList new_element);
struct HandleList expression_symbol(char *symbol);
struct HandleList expression_literal(char *literal);
struct HandleList expression_composite(struct HandleList composite);
char *literal_string(char *literal);
char *literal_int(long literal);
char *literal_float(double literal);

#endif
