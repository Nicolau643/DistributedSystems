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
#include "list-private.h"


struct node_t *node_create(struct entry_t* entry) {
    struct node_t *node;

    node = malloc(sizeof(struct node_t));
    node -> entry = entry;
    node -> next = NULL;

    return node;
}

void node_destroy(struct node_t* node) {
    node -> next = NULL;
    entry_destroy(node -> entry);
    free(node);
}

void list_print(struct list_t *list) {
   
    if (list -> first == NULL || list == NULL) {
        printf("nao tem elementos\n");
        return;
    }
   
    struct node_t* current = list -> first;
   
    while (current != NULL) {
        printf("Key: %s\nValue: %s\n", current -> entry -> key, (char *)current -> entry -> value -> data);
        current = current -> next;
    }
    printf("Tamanho da lista: %d\n", list -> size);
}

///////////////////////  LIST  /////////////////////////

struct list_t *list_create() {

    struct list_t *list;

    list = malloc(sizeof(struct list_t));
    // if (list == NULL){
    //     return NULL;
    // }
    list -> size = 0;
    list -> first = NULL;
    list -> last = NULL;

    return list;
}


void list_destroy(struct list_t *list) {
    
    if (list != NULL) {
        if (list -> size == 1) {
            node_destroy(list -> first);
        }

        if (list -> size > 1) {
            struct node_t *aux;

            while (list -> first != NULL) {
                aux = list -> first;
                list -> first = list -> first -> next;
                node_destroy(aux);
            }
        }

        list -> last = NULL;
        free(list);
    }
}

int list_add(struct list_t* list, struct entry_t* entry) {
    
    if (entry == NULL) {
        return -1;
    }

    struct entry_t *e = list_get(list, entry -> key);


    if(e == NULL){
        return -1;
    }

   
    if (strcmp(e -> value -> data," ") != 0) {
        struct node_t* aux = list -> first;
        
        while (aux != NULL) {
            if (strcmp(aux -> entry -> key, entry -> key) == 0) {
                
                //substituir data
                entry_destroy(aux->entry);
                aux->entry = entry;
                
                //remover e adicionar
                // list_remove(list,entry->key);
                // list->last->next = node_create(entry);
                // list->last = list -> last -> next;
                // list->size += 1; 
               
                break;
               
            }
            aux = aux -> next;
        }
    } else {

        struct node_t* node = node_create(entry); 

        if (list == NULL) {
            list = list_create();
        }

        if (list -> size == 0) {

            list -> first = node;
            list -> last = node;

        } else {

            list -> last -> next = node;
            list -> last = node;
        }

        list -> size = (list -> size) + 1;
    }

    return 0;
}

int list_remove(struct list_t* list, char* key) {

    if (key == NULL) {
        printf("Key to remove is NULL\n");
        return -1;
    }

    if (list -> first == NULL) {
        printf("List to remove from is empty\n");
        return -1;
    }

    struct entry_t *e = list_get(list, key);

    if(e == NULL){
        return -1;
    }

    if (strcmp(e->value->data, " ") == 0) {
        printf("Key to remove doesn't exist\n");
        return -1;
    }

    if (strcmp(list-> first -> entry -> key, key) == 0) { //se a entrada a eliminar for a primeira
        struct node_t* current = list -> first;
        list -> first = list -> first -> next;
        node_destroy(current);
        list -> size = (list -> size) - 1;
        return 0;
    }else if (strcmp(list-> last -> entry -> key, key) == 0)
    {
        struct node_t* current = list -> first;
        while (current->next != list->last){
            current = current->next;
        }

        struct node_t *aux = list->last;
        node_destroy(aux);
        list->last = current;
        list->last->next = NULL;
        list -> size = (list -> size) - 1;

        return 0;
        
    }
    
    
    // se elemento a remover estiver no meio da lista
    struct node_t* current = list -> first;
    while (current != NULL && strcmp(current -> next -> entry -> key, key) != 0) {
        current = current -> next;
    }

    if (current == NULL || current -> next == NULL) {
        return -1;
    }

    struct node_t* next = current -> next -> next;
    node_destroy(current -> next);
    current -> next = next;
    list -> size = (list -> size) - 1;

    return 0;
}

struct entry_t *list_get(struct list_t* list, char* key){
    if (list == NULL || key == NULL) {
        return NULL;
    }

    if (list -> size == 0)
    {
        return entry_create("",data_create2(1," "));
    }
    

    struct node_t* current = list -> first;

    while (current != NULL) {
        if (strcmp(current -> entry -> key, key) == 0) {
            return current -> entry;
        }

        current = current -> next;
    }

    return entry_create("",data_create2(1," "));
}

int list_size(struct list_t* list) {
    if (list != NULL) {
        return list -> size;
    }

    printf("Lista a null\n");
    return -1;
}

char **list_get_keys(struct list_t* list) {

    
    
    if (list == NULL || list -> first == NULL) {
        printf("Lista a null ou sem elementos\n");
        return NULL;
    }
   
    char** aux;
    struct node_t* current = list -> first;
     
    aux = (char **) malloc((list_size(list) + 1) * sizeof(char*));

    for (int i = 0; i < list -> size; i++) {
        aux[i] = strdup(current->entry->key);
        current = current -> next;
    }
   
    aux[list -> size] = NULL;
 
    return aux;
}

void list_free_keys(char** keys) {
    
    if (keys == NULL) {
        printf("Keys a null\n");
        return;
    }

    int i = 0;
    while(keys[i] != NULL) {
        free(keys[i]);
        i++;
    }
    free(keys);
}