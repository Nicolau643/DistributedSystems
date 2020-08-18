//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>

#include "data.h"
#include "entry.h"


int data_to_buffer(struct data_t *data, char **data_buf){

    if (data == NULL) {
        printf("data to buffer data a null\n");
        return -1;
    }

    if (data_buf == NULL) {
        printf("data to buffer buffer a null\n");
        return -1;
    }

    
    int len = data->datasize;

    *data_buf= malloc(sizeof(int)+len);
    
    int aux = htonl(data->datasize);

    memcpy(*data_buf,&aux,sizeof(int));
    
    
    memcpy(*data_buf+sizeof(int),data->data,len);
    
    return len + sizeof(int);

}

struct data_t *buffer_to_data(char *data_buf, int data_buf_size){

    if (data_buf == NULL) {
        return NULL;
    }

    if (data_buf_size <= 0) {
        return NULL;
    }

    struct data_t *data = malloc(sizeof(struct data_t));
    int size_aux;
    memcpy(&size_aux,data_buf,sizeof(int)); 
    
    size_aux = ntohl(size_aux);
    data->datasize = size_aux;

    data->data = malloc(data_buf_size - sizeof(int));

    memcpy(data->data,data_buf+sizeof(int),data_buf_size - sizeof(int));

    return data;


}

int entry_to_buffer(struct entry_t *data, char **entry_buf){

    if (data == NULL) {
        return -1;
    }

    if (entry_buf == NULL) {
        return -1;
    }


    char *entryData;
    int index = 0;
    int lenData = data_to_buffer(data->value,&entryData);
   
    *entry_buf = malloc(sizeof(int) + strlen(data->key) + 1 + lenData);

    int ksize_net = htonl(strlen(data->key));

    memcpy(*entry_buf+index,&ksize_net,sizeof(int));
    index+=sizeof(int);

    memcpy(*entry_buf+index,&data->key,strlen(data->key));
    index+=strlen(data->key) + 1;

    memcpy(*entry_buf+index,entryData,lenData);
    index+=lenData;

    free(entryData);

    return index;
    
}

struct entry_t *buffer_to_entry(char *entry_buf, int entry_buf_size){

    if (entry_buf == NULL) {
        return NULL;
    }

    if (entry_buf_size <= 0) {
        return NULL;
    }

    struct entry_t *entry = malloc(sizeof(entry)); 
    int index = 0;

    int sizek;
    memcpy(&sizek,entry_buf,sizeof(int));
    index+=sizeof(int);
    sizek = ntohl(sizek);

    entry->key = malloc(sizek + 1);
    memcpy(&entry->key,entry_buf+index,sizek);
    index += sizek + 1;

    char *data_buf = malloc(entry_buf_size-index);
    memcpy(data_buf,entry_buf+index,entry_buf_size-index);

    entry->value = buffer_to_data(data_buf,entry_buf_size-index);

    free(data_buf);

    return entry;

}
