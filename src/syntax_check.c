#include <stdio.h>
#include "actions.h"
#include "action_util.h"
#include "handle_list.h"

extern struct HandleList EMPTY_HANDLE_LIST;
extern int yylineno;
extern unsigned long INPUT_LINE_COUNT;

static unsigned int COUNT_TOPLEVEL = 1;

void initialize_actions() {
#ifndef SUPPRESS_PROGRESS_BAR
    print_progress_bar(yylineno, INPUT_LINE_COUNT, 1, 1, false);
#endif

}

void finalize_actions() {
}

void start() {
#ifndef SUPPRESS_PROGRESS_BAR
    print_progress_bar(INPUT_LINE_COUNT, INPUT_LINE_COUNT, 1, 1, true);
#endif
    printf("Syntax check OK\n");
}

void toplevel_list_base(char *handle) {
}

void toplevel_list_recursion(char *handle) {
    if ((COUNT_TOPLEVEL % 10000) == 0) {
#ifndef SUPPRESS_PROGRESS_BAR
        print_progress_bar(yylineno, INPUT_LINE_COUNT, 1, 1, false);
#endif
    } else {
        COUNT_TOPLEVEL++;
    }
}

char *toplevel_expression(struct HandleList composite) {
    return "";
}

char *function_typedef(struct HandleList composite) {
    return "";
}

char *typedef_function(struct HandleList atom_handle_list, char *function_handle) {
    return "";
}

struct HandleList type_desc_type() {
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

char *atom_typedef_atom_type(struct HandleList atom_handle_list) {
    return "";
}

char *atom_typedef_atom_atom(struct HandleList atom_handle_list, struct HandleList parent_handle_list) {
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

struct HandleList expression_empty(struct HandleList composite) {
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
