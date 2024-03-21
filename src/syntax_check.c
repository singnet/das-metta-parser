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

char *typedef_base(char *handle) {
    return "";
}

char *typedef_inherited(char *handle) {
    return "";
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

char *symbol_typedef_symbol_type(char *symbol) {
    return "";
}

char *symbol_typedef_symbol_symbol(char *symbol, char *parent_type) {
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
