#ifndef HANDLELIST_H
#define HANDLELIST_H

struct HandleList {
    unsigned int size;
    char *expression_type_hash;
    char **elements;
    char **elements_type;
};

#endif
