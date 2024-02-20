#include "actions.h"
#include "handle_list.h"

extern struct HandleList EMPTY_HANDLE_LIST;

void initialize_actions() {
}

void finalize_actions() {
}

void typedef_base(char *handle) {
}

void typedef_inherited(char *handle) {
}

char *toplevel_expression(struct HandleList composite) {
    return "";
}

char *inherited_typedef(char *symbol, char *parent_type) {
    return "";
}

char *function_typedef(struct HandleList composite) {
    return "";
}

char *base_typedef_function(char *symbol, char *function_handle) {
    return "";
}

struct HandleList type_desc_type(char *type_symbol) {
    return EMPTY_HANDLE_LIST;
}

struct HandleList type_desc_symbol(char *symbol) {
    return EMPTY_HANDLE_LIST;
}

struct HandleList type_desc_function(char *handle) {
    return EMPTY_HANDLE_LIST;
}

struct HandleList type_desc_list_base(struct HandleList base) {
    return EMPTY_HANDLE_LIST;
}

struct HandleList type_desc_list_recursion(struct HandleList list, struct HandleList new_element) {
    return EMPTY_HANDLE_LIST;
}

char *symbol_typedef_symbol_type(char *symbol) {
    return "";
}

char *symbol_typedef_symbol_symbol(char *symbol, char *parent_type) {
    return "";
}

char *symbol_typedef_literal_type(char *literal) {
    return "";
}

char *symbol_typedef_literal_symbol(char *literal, char *parent_type) {
    return "";
}

struct HandleList expression_list_base(struct HandleList base) {
    return EMPTY_HANDLE_LIST;
}

struct HandleList expression_list_recursion(struct HandleList list, struct HandleList new_element) {
    return EMPTY_HANDLE_LIST;
}

struct HandleList expression_symbol(char *symbol) {
    return EMPTY_HANDLE_LIST;
}

struct HandleList expression_literal(char *literal) {
    return EMPTY_HANDLE_LIST;
}

struct HandleList expression_composite(struct HandleList composite) {
    return EMPTY_HANDLE_LIST;
}

char *literal_string(char *literal) {
    return "";
}

char *literal_int(long literal) {
    return "";
}

char *literal_float(double literal) {
    return "";
}
