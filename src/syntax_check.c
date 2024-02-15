#include "actions.h"
#include "handle_list.h"

extern struct HandleList EMPTY_HANDLE_LIST;

void initialize_actions() {
}

void finalize_actions() {
}

char *toplevel_expression(struct HandleList composite) {
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
