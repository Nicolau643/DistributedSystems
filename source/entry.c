//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "entry.h"


struct entry_t *entry_create(char *key, struct data_t *data){

    if (key == NULL)
    {
        printf("chave esta a null\n");
        return NULL;
    }

    if (data == NULL)
    {
        printf("data esta a null\n");
        return NULL;
    }
    
    

    struct entry_t *entry;
    entry = (struct entry_t*) malloc(sizeof(struct entry_t));
    entry->key = key;
    entry->value = data;

    return entry;

}


void entry_initialize(struct entry_t *entry){

    if (entry == NULL)
    {
        printf("entry esta a null");
        return;
    }
    
    entry -> key = NULL;
    entry -> value = NULL;

}


void entry_destroy(struct entry_t *entry){

    if (entry == NULL)
    {
        printf("entry esta a null\n");
        return;
    }
    

    data_destroy(entry->value);
    free(entry->key);
    free(entry);

}


struct entry_t *entry_dup(struct entry_t *entry){

    if (entry == NULL)
    {
        printf("entry esta a null\n");
        return NULL;
    }

    struct entry_t *newEntry = malloc(sizeof(struct entry_t));
    newEntry->key = malloc(strlen(entry->key)+1);
    strcpy(newEntry->key,entry->key);
    newEntry->value = data_dup(entry->value);

    return newEntry;

}