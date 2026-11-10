#ifndef LIST_H
#define LIST_H
#include <stdlib.h>

typedef struct {
    void **elements;
    size_t length;
} list_T;

list_T* init_list();
void add_element(list_T* l, void *element);
void free_list(list_T* l);
void free_list_el(list_T* l);
#endif