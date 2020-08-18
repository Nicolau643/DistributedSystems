//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"


struct data_t *data_create(int size){

    if (size<=0)
    {   
        printf("erro: tamanho negativo ou 0\n");
        return NULL;
    }
    

    struct data_t *dataElement;
    dataElement = malloc(sizeof(struct data_t*));
    dataElement->datasize=size;
    dataElement->data = malloc(size);

    return dataElement;

}


struct data_t *data_create2(int size, void *data){

    if (size<0)
    {
        printf("erro: tamanho negativo\n");
        return NULL;
    }

    if (data == NULL)
    {
        printf("erro: data esta a null\n");
        return NULL;
    }
    
    struct data_t *dataElement;
    dataElement = (struct data_t*) malloc(sizeof(struct data_t*));
    dataElement->datasize=size;
    dataElement->data = data;
    
    return dataElement;

}


void data_destroy(struct data_t *data){
    if (data != NULL)
    {
        free(data->data);
        free(data);
    }
    
    
}


struct data_t *data_dup(struct data_t *data){

    if (data == NULL)
    {
        printf("data esta a null\n");
        return NULL;
    }

    if (data->data == NULL || data->datasize <= 0)
    {
        printf("conteudo data esta a null\n");
        return NULL;
    
    }

    struct data_t *dataElement;
    dataElement = malloc(sizeof(struct data_t));
    dataElement->datasize = data->datasize;
    dataElement->data = malloc(data->datasize);
    memcpy(dataElement->data,data->data,data->datasize);

    return dataElement;

}