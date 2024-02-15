#ifndef ACTIONUTIL_H
#define ACTIONUTIL_H

char *string_copy(char *source);
void print_progress_bar(
        unsigned int iteration,
        unsigned int total,
        unsigned int length,
        unsigned int step,
        unsigned int max_step);

#endif
