#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "json.h"

struct MemoryStruct
{
    char *memory;
    size_t size;
};

// 1. The custom write callback function
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    // Expand the buffer to fit the new chunk of data
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
    {
        /* out of memory! */
        fprintf(stderr, "not enough memory (realloc returned NULL)\n");
        return 0; // Returning 0 signals an error to libcurl and aborts the transfer
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; // Null-terminate the string

    return realsize; // Return the exact number of bytes processed
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("welcome to def v0.0.1\n");
        printf("usage:\n\tdef [word] [flags]\n\npost definition of a word\nflags: -[de/fr/es/jp] changes target language to another country (gives English translation)\n");
        return 1;
    }

    char *target = "-en";
    int t_i = -1;
    list_T *words = init_list();
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (*argv[i] != '-')
            {
                add_element(words, argv[i]);
            }
            else
            {
                target = argv[i];
                t_i = -1;
            }
        }
        if (*target != '-')
        {
            printf("expected correct header; try adding - before %s (ex. -%s)\n", argv[t_i], argv[t_i]);
            return 0;
        }
    }
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    if (curl)
    {
        char *url_frag = "https://freedictionaryapi.com/api/v1/entries/";
        size_t total_len = 0;
        for (size_t i = 0; i < words->length; i++)
        {
            total_len += strlen(words->elements[i]);
            total_len += 4;
        }
        char *url = (char *)calloc(strlen(url_frag) + strlen(target) + 3 + total_len, sizeof(char));
        sprintf(url, "%s%s/", url_frag, target + 1);
        for (size_t i = 0; i < words->length; i++)
        {
            char *word = words->elements[i];
            strcat(url, word);
            if (i != words->length - 1)
                strcat(url, "%20");
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        // Pass our 'chunk' struct pointer to the callback function via the userp parameter
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        // Optional: Provide a user-agent string to be polite to the server
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK)
        {
            fprintf(stderr, "def: ran into error (err code %d)\n", res);
            free(url);
            curl_easy_cleanup(curl);
            free(chunk.memory);
            free_list(words);
            return 1;
        }

        char *json = chunk.memory;

        Program *p = init_program(json);
        Dict *d = parse_json(p);

        Value *vl = (Value *)calloc(1, sizeof(Value));
        vl->type = DICT;
        vl->dict_val = d;
        KeyValue_Pair *s = search_kv(d, "entries");
        if (s->value->list_val->length < 1)
        {
            fprintf(stderr, "def: no definition for %s", (char *)words->elements[0]);
            for (size_t i = 1; i < words->length; i++)
            {
                if (i == words->length - 1)
                    fprintf(stderr, " %s", (char *)words->elements[i]);
                else
                    fprintf(stderr, " %s", (char *)words->elements[i]);
            }
            fprintf(stderr, "\n");
            free_list(words);
            exit(1);
        }
        for (size_t l = 0; l < s->value->list_val->length; l++)
        {
            Value *vl = (Value*)s->value->list_val->elements[l];
            KeyValue_Pair *p_o_s = search_kv(((Value *)s->value->list_val->elements[l])->dict_val, "partOfSpeech");
            if (p_o_s)
            {
                printf("(%s)\n", p_o_s->value->string_val);
            }

            KeyValue_Pair *forms = search_kv(vl->dict_val, "forms");
            for (size_t a = 0; a < forms->value->list_val->length; a++) {
                if (a > 4) {
                    break;
                }
                Value *el = forms->value->list_val->elements[a];
                KeyValue_Pair *word = search_kv(el->dict_val, "word");
                list_T* tags = search_kv(el->dict_val, "tags")->value->list_val;
                printf("%s: ", word->value->string_val);
                for (size_t b = 0; b < tags->length; b++) {
                    Value *ss = (Value*)tags->elements[b];
                    if (b == tags->length - 1) {
                        printf("%s\n", ss->string_val);
                    }
                    else printf("%s ", ss->string_val);
                }
            }
            KeyValue_Pair *senses = search_kv(((Value *)s->value->list_val->elements[l])->dict_val, "senses");

            for (size_t i = 0; i < senses->value->list_val->length; i++)
            {
                KeyValue_Pair *def = search_kv(((Value *)senses->value->list_val->elements[i])->dict_val, "definition");
                KeyValue_Pair *_ex = search_kv(((Value *)senses->value->list_val->elements[i])->dict_val, "examples");
                char *to_s = value_to_str(def->value);
                printf("%zu. %s\n", i + 1, def->value->string_val);
                for (size_t k = 0; k < _ex->value->list_val->length; k++)
                {
                    Value *kvv = _ex->value->list_val->elements[k];
                    printf("\t(ex. \"%s\")\n", kvv->string_val);
                }

                KeyValue_Pair *sub_sen = search_kv(((Value *)senses->value->list_val->elements[i])->dict_val, "subsenses");
                if (sub_sen)
                {
                    for (size_t j = 0; j < sub_sen->value->list_val->length; j++)
                    {
                        Dict *dd = ((Value *)sub_sen->value->list_val->elements[j])->dict_val;
                        KeyValue_Pair *ss = search_kv(dd, "definition");
                        KeyValue_Pair *ex = search_kv(dd, "examples");
                        char *to_ss = value_to_str(ss->value);
                        printf("\t%zu. %s\n", j + 1, ss->value->string_val);
                        for (size_t k = 0; k < ex->value->list_val->length; k++)
                        {
                            Value *kvv = ex->value->list_val->elements[k];
                            printf("\t\t(ex. \"%s\")\n", kvv->string_val);
                        }
                        free(to_ss);
                    }
                }
                free(to_s);
            }
        }
        free_all(p, d);
        free(url);
        free_list(words);

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return 0;
}
