#include <stdio.h>
#include <string.h>
#include "action_util.h"
#include "limits.h"

extern void yyerror(const char* s);

char *string_copy(char *source) {
    if (strlen(source) >= MAX_LITERAL_OR_SYMBOL_SIZE) {
        yyerror("Literal is too long.");
    }
    char *answer = strndup(source, MAX_LITERAL_OR_SYMBOL_SIZE);
    if (answer == NULL) {
        yyerror("Error copying literal string.");
    }
    return answer;
}

void print_progress_bar(
        unsigned int iteration, 
        unsigned int total, 
        unsigned int length, 
        unsigned int step, 
        unsigned int max_step) {

    unsigned int filled_length = (unsigned int) (((float) iteration / total) * length);
    unsigned int previous = (unsigned int) ((((float) iteration - 1) / total) * length);
    if (iteration == 1 || filled_length > previous || iteration >= total) {
        if (iteration > total) {
            iteration = total;
        }
        char percent[16];
        sprintf(percent, "%.0f", (float) 100 * ((float) iteration / (float) total));
        char fill='#';
        char line[1024];
        unsigned int cursor = 0;
        for (unsigned int i = 0; i < filled_length; i++) {
            line[cursor++] = fill;
        }
        for (unsigned int i = 0; i < length - filled_length; i++) {
            line[cursor++] = '-';
        }
        printf("\r STEP %d/%d Progress: |%s| %s%% complete (%d/%d)", step, max_step, line, percent, iteration, total);
        fflush(stdout);
    }
}
