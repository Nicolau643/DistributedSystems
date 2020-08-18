//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "entry.h"
#include "list.h"
#include "table.h"
#include "table-private.h"
#include "list-private.h"


int calcHash(char *key,int size){
    
    int i;
    int res = 0;
    
    for ( i = 0; i < strlen(key); i++)
    {
        res *= (int)key[i]; 
    }
    
    return res % size;
}

void table_print(struct table_t *table){
    int i = 0;
    
    while(table->list[i] != NULL){
        printf("table %d\n",i);
        list_print(table->list[i]);
        i++;
    }

    printf("-------\n");
}
///////////////////////  TABLE  /////////////////////////

struct table_t* table_create(int n) {
    int i;
// Verifica se o o inteiro inserido é inferior a 0
    if (n<=0)
    {
        printf("parametro de numero de listas na table negativo\n");
        return NULL;
    }
// Vai reservar espaço em memória para a estrutura    
    struct table_t* table = malloc(sizeof(struct table_t));
    table -> size = n;
    table -> list = malloc(sizeof(struct list_t*) * n + 1);
// Vai reservar memória para n listas
    
    for ( i = 0; i < n; i++) {
        table -> list[i] = list_create();
       
    }

    table -> list[n] = NULL;
    
    //table_print(table);
   
    return table;
}

void table_destroy(struct table_t *table){
    
    if (table == NULL)
    {
        printf("table esta a null\n");
        return;
    }


    int i = 0;

    for ( i = 0; i < table->size; i++)
    {
       list_destroy(table->list[i]);
    }

    free(table);
}

int table_put(struct table_t *table, char *key, struct data_t *value){

    if (table == NULL)
    {
        printf("table esta a null\n");
        return -1;
    }

    if (key == NULL)
    {
        printf("key esta a null\n");
        return -1;
    }

    if (value == NULL)
    {
        printf("value esta a null\n");
        return -1;
    }


    struct entry_t *newE = entry_create(strdup(key),data_dup(value));
    
    return list_add(table->list[calcHash(key,table->size)],newE);

}


struct data_t *table_get(struct table_t *table, char *key){

    if (table == NULL)
    {
        printf("table estah a null\n");
        return NULL;
    }
    if (key == NULL)
    {
        printf("key esta a null\n");
        return NULL;
    }

    struct entry_t *entryAux = list_get(table->list[calcHash(key,table->size)],key);

    if (entryAux == NULL)
    {
       return NULL;
    }

    printf("data table_get ->%p\n",entryAux->value->data);

    /*if (strcmp( entryAux -> key, "") == 0)
    {
        return data_create2(1," ");
    }*/
    
    return data_dup(entryAux->value);

}

int table_del(struct table_t *table, char *key){

     if (table == NULL)
    {
        printf("table estah a null\n");
        return -1;
    }
    if (key == NULL)
    {
        printf("key esta a null\n");
        return -1;
    }

    return list_remove(table->list[calcHash(key,table->size)],key);

}

int table_size(struct table_t *table){

     if (table == NULL)
    {
        printf("table esta a null\n");
        return -1;
    }

    int size = 0;

    for (int i = 0; i < table->size; i++)
    {
        size += list_size(table->list[i]);
    }
    
    return size;

}

char **table_get_keys(struct table_t *table){

    if (table == NULL)
    {
        printf("table esta a null\n");
        return NULL;
    }

    int size = table_size(table);
    int i = 0;

    int cont = 0;
    int contk;
    char **res ; 
    res = malloc((size+1) * sizeof(char *)); 
    char **ks;

    struct list_t *l;
    while ((l = table->list[i]) != NULL)
    {
    
        contk = 0;
        ks = list_get_keys(l);
        
        if (ks != NULL){


        
            while (ks[contk] != NULL){
            
            res[cont] = malloc(strlen(ks[contk])+1);
            strcpy(res[cont],ks[contk]);
            cont++;
            contk++;
            }

            list_free_keys(ks);
        }

        i++;
    }

   
    res[cont] = NULL;
    
    return res;

    

}

void table_free_keys(char **keys){

    if (keys == NULL)
    {
        printf("keys esta a null");
        return;
    }
    
    int i = 0;

    while (keys[i] != NULL)
    {
        free(keys[i]);
        i++;
    }

    free(keys);
    

}
