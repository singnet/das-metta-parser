#ifndef ACTIONS_H
#define ACTIONS_H

#include "handle_list.h"

void initialize_actions();
void finalize_actions();
void typedef_base(char *handle);
void typedef_inherited(char *handle);
char *toplevel_expression(struct HandleList composite);
char *base_typedef_function(char *symbol, char *function_handle);
char *inherited_typedef(char *symbol, char *parent_type);
char *function_typedef(struct HandleList composite);
struct HandleList type_desc_type(char *type_symbol);
struct HandleList type_desc_symbol(char *symbol);
struct HandleList type_desc_function(char *handle);
char *symbol_typedef_symbol_type(char *symbol);
char *symbol_typedef_symbol_symbol(char *symbol, char *parent_type);
char *symbol_typedef_literal_type(char *literal);
char *symbol_typedef_literal_symbol(char *literal, char *parent_type);
struct HandleList type_desc_list_base(struct HandleList base);
struct HandleList type_desc_list_recursion(struct HandleList list, struct HandleList new_element);
struct HandleList expression_list_base(struct HandleList base);
struct HandleList expression_list_recursion(struct HandleList list, struct HandleList new_element);
struct HandleList expression_symbol(char *symbol);
struct HandleList expression_literal(char *literal);
struct HandleList expression_composite(struct HandleList composite);
char *literal_string(char *literal);
char *literal_int(long literal);
char *literal_float(double literal);

#endif
