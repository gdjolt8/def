#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

list_T* init_list() {
    list_T *l = (list_T*)calloc(1, sizeof(list_T));
    l->elements = NULL;
    l->length = 0;
    return l;
}

void add_element(list_T* l, void *element) {
    l->length++;
    l->elements = realloc(l->elements, (l->length) * sizeof(void*));
    l->elements[l->length - 1] = element;
}

void free_list(list_T* l) {
    if (l == NULL) return;
    if(l->elements) free(l->elements);
    free(l);
}

void free_list_el(list_T* l) {
    if (l == NULL) return;
    for(size_t i = 0; i < l->length; i++) {
        free(l->elements[i]);
    }
    if(l->elements) free(l->elements);
    free(l);
}