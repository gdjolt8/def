#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Value *parse_value(Program *p)
{
    Value *va = (Value *)calloc(1, sizeof(Value));

    char *s = (char *)calloc(1, sizeof(char));
    size_t s_l = 0;
    if (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
    {
        while (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
        {
            p->src++;
        }
    }
    if (*(p->src) == '[')
    {
        list_T *ls = init_list();
        va->type = ARRAY;
        va->list_val = ls;
        (p->src)++;
        while (*(p->src) != ']')
        {
            if (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
            {
                while (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
                {
                    (p->src)++;
                }
            }
            add_element(ls, parse_value(p));
            if (*(p->src) == ',')
            {
                (p->src)++;
            }
        }
        (p->src)++;
        return va;
    }
    if (*(p->src) == '\'' || *(p->src) == '\"')
    {
        va->type = STRING;
        char start_q = *(p->src);
        (p->src)++;
        while (*(p->src) != start_q)
        {
            if (*(p->src) == '\\' && *(p->src + 1) == start_q)
            {
                (p->src)++;
                s_l++;
                s = realloc(s, (s_l + 1) * sizeof(char));
                strcat(s, (char[]){*(p->src), '\0'});
                (p->src)++;
                continue;
            }
            s_l++;
            s = realloc(s, (s_l + 1) * sizeof(char));
            strcat(s, (char[]){*(p->src), '\0'});
            (p->src)++;
        }
        (p->src)++;
        va->string_val = s;
        return va;
    }
    if ((*(p->src) >= '0' && *(p->src) <= '9') || *(p->src) == '.' || *(p->src) == '-' || *(p->src) == '+')
    {
        va->type = NUMBER;
        while ((*(p->src) >= '0' && *(p->src) <= '9') || *(p->src) == '.' || *(p->src) == 'e' || *(p->src) == '-' || *(p->src) == '+')
        {
            if (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
            {
                printf("unexpected break in number\n");
                exit(1);
            }
            s_l++;
            s = realloc(s, (s_l + 1) * sizeof(char));
            strcat(s, (char[]){*(p->src), '\0'});
            (p->src)++;
        }
        va->number_val = atof(s);
        return va;
    }
    if (*(p->src) == '{')
    {
        p->src++;
        va->type = DICT;
        va->dict_val = parse_json(p);
        if (*(p->src) == '}')
        {
            p->src++;
        }
        return va;
    }

    if (*(p->src) == 't' || *(p->src) == 'f')
    {
        while (isalnum(*(p->src)))
        {
            s_l++;
            s = realloc(s, (s_l + 1) * sizeof(char));
            strcat(s, (char[]){*(p->src), '\0'});
            p->src++;
        }
        va->type = BOOL;
        if (strcmp(s, "true") == 0 || strcmp(s, "false") == 0)
        {
            va->bool_val = (strcmp(s, "true") == 0);
            return va;
        }
        else if (strcmp(s, "null") == 0)
        {
            va->type = IDENT;
            va->ident_val = s;
        }
        else
        {
            printf("expected boolean value for identifier, got %s\n", s);
            exit(1);
        }
    }
    return NULL;
}

Dict *parse_json(Program *p)
{
    list_T *kv = init_list();

    char *k = (char *)calloc(1, sizeof(char));
    size_t k_l = 0;
    Value *va;

    Dict *d = (Dict *)calloc(1, sizeof(Dict));
    while (*(p->src) != '\0')
    {
        //printf("heyo %c%c%c%c\n", *(p->src - 2), *(p->src - 1), *(p->src), *(p->src+1));
        if (*(p->src) == '{')
        {
            (p->src)++;
        }

        if (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
        {
            while (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
            {
                (p->src)++;
            }
        }
        if (*(p->src) == '}')
        {
            d->pairs = kv;
            return d;
        }

        if (*(p->src) == ',')
        {
            (p->src)++;
        }

        if (*(p->src) == '\'' || *(p->src) == '\"')
        {
            char start_q = *(p->src);
            (p->src)++;
            while (*(p->src) != start_q)
            {
                k_l++;
                k = realloc(k, (k_l + 1) * sizeof(char));
                strcat(k, (char[]){*(p->src), '\0'});
                (p->src)++;
            }
            (p->src)++;
            (p->src)++;
            if (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
            {
                while (*(p->src) == '\n' || *(p->src) == ' ' || *(p->src) == '\t')
                {
                    (p->src)++;
                }
            }
            va = parse_value(p);
            KeyValue_Pair *kvp = (KeyValue_Pair *)calloc(1, sizeof(KeyValue_Pair));
            kvp->key = k;
            kvp->value = va;
            k = calloc(1, sizeof(char));
            add_element(kv, kvp);
        }
    }
    return NULL;
}
KeyValue_Pair *search_kv(Dict *d, char *k)
{
    for (size_t i = 0; i < d->pairs->length; i++)
    {
        KeyValue_Pair *p = d->pairs->elements[i];
        if (strcmp(p->key, k) == 0)
        {
            return p;
        }
    }
    return NULL;
}

list_T *search_all_kv(Dict *d, char *k)
{
    list_T* ls = init_list();
    for (size_t i = 0; i < d->pairs->length; i++)
    {
        KeyValue_Pair *p = d->pairs->elements[i];
        if (strcmp(p->key, k) == 0)
        {
            add_element(ls, p);
        }
    }
    return ls;
}

char *value_to_str(Value *v)
{
    char *str = (char *)calloc(1, sizeof(char));
    switch (v->type)
    {
    case NUMBER:
    {
        str = realloc(str, 16 * sizeof(char));
        sprintf(str, "%f", v->number_val);
        break;
    }
    case STRING:
    {
        str = realloc(str, 3 + strlen(v->string_val) * sizeof(char));
        sprintf(str, "\"%s\"", v->string_val);
        break;
    }
    case ARRAY:
    {
        list_T *ls = init_list();
        size_t total_len = 0;
        for (size_t i = 0; i < v->list_val->length; i++)
        {
            add_element(ls, value_to_str(v->list_val->elements[i]));
            total_len += strlen((char *)ls->elements[i]);
            total_len += 2;
        }
        str = realloc(str, total_len + 3);
        strcat(str, "[");
        for (size_t i = 0; i < ls->length; i++)
        {
            char *el = ls->elements[i];
            strcat(str, el);
            if (i == ls->length - 1)
                break;
            else
                strcat(str, ", ");
        }
        strcat(str, "]");
        for (size_t i = 0; i < ls->length; i++)
        {
            free(ls->elements[i]);
        }
        free_list(ls);
        break;
    }
    case BOOL:
    {
        if (v->bool_val == 1)
        {
            str = realloc(str, strlen("true") + 1);
            strcpy(str, "true");
        }
        else
        {
            str = realloc(str, strlen("false") + 1);
            strcpy(str, "false");
        }
        break;
    }
    case DICT:
    {
        size_t cur_len = 2;
        str = (char *)realloc(str, cur_len * sizeof(char));
        strcat(str, "{");
        for (size_t i = 0; i < v->dict_val->pairs->length; i++)
        {
            KeyValue_Pair *kv = (KeyValue_Pair *)v->dict_val->pairs->elements[i];
            cur_len += strlen(kv->key) + 6;
            str = realloc(str, cur_len * sizeof(char));
            strcat(str, kv->key);
            strcat(str, ": ");
            char *s = value_to_str(kv->value);
            cur_len += strlen(s) + 4;
            str = realloc(str, cur_len * sizeof(char));
            strcat(str, s);
            if (i == v->dict_val->pairs->length - 1)
                break;
            else
            {
                cur_len += 4;
                str = realloc(str, cur_len * sizeof(char));
                strcat(str, ", ");
            }
        }
        cur_len++;
        str = realloc(str, cur_len * sizeof(char));
        strcat(str, "}");
        break;
    }
    case IDENT:
    {
        str = realloc(str, strlen(v->ident_val) + 1);
        strcpy(str, v->ident_val);
        break;
    }
    }
    return str;
}

Program *init_program(char *src)
{
    Program *p = (Program *)calloc(1, sizeof(Program));
    p->src = src;
    p->src_ptr = 0;
    return p;
}

void free_value(Value *v)
{
    switch (v->type)
    {
    case BOOL:
    {
        free(v);
        break;
    }
    case NUMBER:
    {
        free(v);
        break;
    }
    case STRING:
    {
        free(v->string_val);
        free(v);
        break;
    }
    case ARRAY:
    {
        for (size_t i = 0; i < v->list_val->length; i++)
        {
            free(v->list_val->elements[i]);
        }
        free_list(v->list_val);
        free(v);
        break;
    }
    case DICT:
    {
        for (size_t i = 0; i < v->dict_val->pairs->length; i++)
        {
            KeyValue_Pair *kv = v->dict_val->pairs->elements[i];
            free(kv->key);
            free_value(kv->value);
        }
        free_list(v->dict_val->pairs);
        free(v->dict_val);
        free(v);
        break;
    }
    case IDENT:
    {
        free(v->ident_val);
        free(v);
    }
    default:
    {
        break;
    }
    }
}

void free_all(Program *p, Dict *json)
{
    free(p);
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = DICT;
    v->dict_val = json;
    free_value(v);
}