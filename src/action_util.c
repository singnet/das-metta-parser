#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "action_util.h"
#include "limits.h"

#define PROGRESS_BAR_LENGTH ((unsigned int) 60)

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

static unsigned int PREVIOUS_FILLED_LENGTH = 0;
void print_progress_bar(
        unsigned int iteration, 
        unsigned int total, 
        unsigned int step, 
        unsigned int max_step,
        bool print_eol) {

    bool LOG_STYLE = true;

    if (total <= PROGRESS_BAR_LENGTH) {
        return;
    }
    unsigned int filled_length = (unsigned int) (((float) iteration / total) * PROGRESS_BAR_LENGTH);
    if (iteration == 1 || filled_length > PREVIOUS_FILLED_LENGTH || iteration >= total) {
        PREVIOUS_FILLED_LENGTH = filled_length;
        if (iteration > total) {
            iteration = total;
        }
        char percent[16];
        sprintf(percent, "%.0f", (float) 100 * ((float) iteration / (float) total));
        char fill = '#';
        char line[1024];
        unsigned int cursor = 0;
        for (unsigned int i = 0; i < filled_length; i++) {
            line[cursor++] = fill;
        }
        for (unsigned int i = 0; i < PROGRESS_BAR_LENGTH - filled_length; i++) {
            line[cursor++] = '-';
        }
        line[cursor] = '\0';
        if (LOG_STYLE) {
            printf("\r STEP %d/%d %s%% complete (%d/%d) \n", step, max_step, percent, iteration, total);
        } else {
            printf("\r STEP %d/%d Progress: |%s| %s%% complete (%d/%d)", step, max_step, line, percent, iteration, total);
            if (print_eol) {
                printf("\n");
            }
        }
        fflush(stdout);
    }
}

unsigned long get_line_count(char *fname) {

    unsigned int MAX_PATH_LENGTH = 1024;
    FILE *fp;
    char command_line[MAX_PATH_LENGTH];
    char output[MAX_PATH_LENGTH];
  
    if ((strlen(fname) + 20) > MAX_PATH_LENGTH) {
        yyerror("Input file path is too long.");
        exit(1);
    }

    sprintf(command_line, "wc -l %s", fname);
    fp = popen(command_line, "r");
    if (fp == NULL) {
        yyerror("Failed to run wc on input file\n");
        exit(1);
    }
    fgets(output, sizeof(output), fp);
    pclose(fp);
    return atoi(output);
}

