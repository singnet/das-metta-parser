#ifndef ACTIONS_H
#define ACTIONS_H

#include "handle_list.h"

void initialize_actions();
void finalize_actions();
void start();
void toplevel_list_base(char *handle);
void toplevel_list_recursion(char *handle);
char *toplevel_expression(struct HandleList composite);
char *function_typedef(struct HandleList composite);
struct HandleList type_desc_symbol(char *symbol);
struct HandleList type_desc_function(char *handle);
char *typedef_function(char *typedef_mark, struct HandleList atom_handle_list, char *function_handle);
char *typedef_expression_expression(char *typedef_mark, struct HandleList atom_handle_list, struct HandleList parent_handle_list);
struct HandleList type_desc_list_base(struct HandleList base);
struct HandleList type_desc_list_recursion(struct HandleList list, struct HandleList new_element);
struct HandleList expression_list_base(struct HandleList base);
struct HandleList expression_list_recursion(struct HandleList list, struct HandleList new_element);
struct HandleList expression_symbol(char *symbol);
struct HandleList expression_literal(char *literal);
struct HandleList expression_composite(struct HandleList composite);
struct HandleList expression_empty();
char *literal_string(char *literal);
char *literal_int(long literal);
char *literal_float(double literal);

#endif
