#ifndef ACTIONUTIL_H
#define ACTIONUTIL_H

#include <stdbool.h>

char *string_copy(char *source);
void print_progress_bar(
        unsigned int iteration,
        unsigned int total,
        unsigned int step,
        unsigned int max_step,
        bool print_eol);
unsigned long get_line_count(char *fname);

#endif
