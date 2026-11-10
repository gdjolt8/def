#ifndef JSON_H
#define JSON_H

#include <stdlib.h>
#include "list.h"


typedef struct {
    list_T* pairs; 
} Dict;

typedef struct {
    enum {
        NUMBER,
        STRING,
        ARRAY,
        DICT,
        BOOL,
        IDENT,
    } type;

    double number_val;
    char *string_val;
    list_T* list_val;
    Dict *dict_val;
    char *ident_val;
    unsigned int bool_val;
} Value;
typedef struct {
    char *key;
    Value *value;
} KeyValue_Pair;

typedef struct {
    char *src;
    size_t src_ptr;
} Program;
Dict *parse_json(Program* p);

Program *init_program(char *src);
char* value_to_str(Value *v);
KeyValue_Pair *search_kv(Dict *d, char* k);
void free_all(Program *p, Dict *json);
#endif