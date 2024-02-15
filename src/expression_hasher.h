#ifndef EXPRESSIONHASHER_H
#define EXPRESSIONHASHER_H

char *compute_hash(char *input);
char *named_type_hash(char *name);
char *terminal_hash(char *type, char *name);
char *expression_hash(char *type_hash, char **elements, unsigned int nelements);
char *composite_hash(char **elements, unsigned int nelements);

#endif
