//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "data.h"
#include "entry.h"
#include "message-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "table.h"
#include "table-private.h"
#include "table_client.h"
#include "zookeeper/zookeeper.h"

typedef struct String_vector zoo_string; 

static zhandle_t *zh;



int zdata_len;

int is_connected;

zoo_string *children_list;

void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {
    if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1; 
		} else {
			is_connected = 0; 
		}
	} 
}


void child_watcher(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx){

    if (state == ZOO_CONNECTED_STATE && type == ZOO_CHILD_EVENT)	 {
            
           
            free(children_list);
            children_list = malloc(sizeof(zoo_string));

            // meter o watch
            if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, NULL, children_list)) {
                        fprintf(stderr, "Error setting watch wget!\n");
                }

            char aux[1024];
            sprintf(aux,"/chain/%s",children_list->data[0]);

            //printf("aux %s\n",aux);
            
            char server_head[1024];
            char server_tail[1024];
            
            zdata_len = sizeof(server_head);

           
            if (ZOK != zoo_get(zh,aux , 0, server_head, &zdata_len, NULL)){
                            fprintf(stderr, "Error getting at zoo_get1\n");
            }

            sprintf(aux,"/chain/%s",children_list->data[children_list->count - 1]);
            zdata_len = sizeof(server_tail);

            zdata_len = sizeof(server_tail);


            if (ZOK != zoo_get(zh,aux,0, server_tail, &zdata_len, NULL)){
                            fprintf(stderr, "Error getting at zoo_get2\n");
            }

            
            // Reserva espaço de memoria para rtable
            struct rtable_t *rtable = (struct rtable_t *) malloc (sizeof(struct rtable_t));

            //head

            //Divide o address por ":" para termos acesso ao hostname e porto
            char *token = strtok(strdup(server_head),":");
            rtable -> address_head = strdup(token);

            // Vai buscar o proximo token pela qual tinha sido feita a divisão
            token = strtok(NULL, ":");
            
            // Passa a string do toke(porto) para um inteiro
            rtable -> porto_head = atoi(strdup(token));

            //tail

            //Divide o address por ":" para termos acesso ao hostname e porto
            char *token2 = strtok(strdup(server_tail),":");
            rtable -> address_tail = strdup(token2);

            // Vai buscar o proximo token pela qual tinha sido feita a divisão
            token2 = strtok(NULL, ":");
            
            // Passa a string do toke(porto) para um inteiro
            rtable -> porto_tail = atoi(strdup(token2));

            rtable_disconnect(table);

            if(network_connect(rtable) != 0) return;

            
            table = rtable; //ver extern table_client

   } 

}


/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtable_t *rtable_connect(const char *address_port){


    // Verifica se o address port passado nao e NULL
    if(address_port == NULL) return NULL;

    

    zh = zookeeper_init(address_port, my_watcher_func,	2000, 0, NULL, 0); 
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	}

    sleep(3);

    if(!is_connected){
        return NULL;
    }

    children_list = (zoo_string *) malloc(sizeof(zoo_string));

    // meter o watch
     if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, NULL, children_list)) {
				fprintf(stderr, "Error setting watch wget!\n");
		}

    //get children e mais alto e mais baixo    

    char aux[1024];
    sprintf(aux,"/chain/%s",children_list->data[0]);

   
    
    char server_head[1024];
    char server_tail[1024];

    zdata_len = sizeof(server_head);


    if (ZOK != zoo_get(zh,aux , 0, server_head, &zdata_len, NULL)){
                    fprintf(stderr, "Error getting at zoo_get1\n");
    }

    sprintf(aux,"/chain/%s",children_list->data[children_list->count - 1]);
    zdata_len = sizeof(server_tail);

    int pos = children_list->count - 1;

   

    zdata_len = sizeof(server_tail);


    if (ZOK != zoo_get(zh,aux,0, server_tail, &zdata_len, NULL)){
                    fprintf(stderr, "Error getting at zoo_get2\n");
    }


    // Reserva espaço de memoria para rtable
    struct rtable_t *rtable = (struct rtable_t *) malloc (sizeof(struct rtable_t));

    //head

    //Divide o address por ":" para termos acesso ao hostname e porto
    char *token = strtok(strdup(server_head),":");
    rtable -> address_head = strdup(token);

    // Vai buscar o proximo token pela qual tinha sido feita a divisão
    token = strtok(NULL, ":");
    
    // Passa a string do toke(porto) para um inteiro
    rtable -> porto_head = atoi(strdup(token));

    //tail

     //Divide o address por ":" para termos acesso ao hostname e porto
    char *token2 = strtok(strdup(server_tail),":");
    rtable -> address_tail = strdup(token2);

    // Vai buscar o proximo token pela qual tinha sido feita a divisão
    token2 = strtok(NULL, ":");
    
    // Passa a string do toke(porto) para um inteiro
    rtable -> porto_tail = atoi(strdup(token2));


    if(network_connect(rtable) != 0) return NULL;

    return rtable;

}



/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtable_disconnect(struct rtable_t *rtable){

    if (rtable == NULL) return -1;
    int res = network_close(rtable);
    if (res != 0) return -1;

    return 0;

}

/* Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtable_put(struct rtable_t *rtable, struct entry_t *entry){

    if (rtable == NULL || entry == NULL)
	return -1;

    struct message_t *msg_out = (struct message_t *) malloc(sizeof(struct message_t));
    msg_out -> opcode = OP_PUT;
    msg_out -> c_type = CT_ENTRY;
    msg_out -> content.entry = entry;
    
    
    struct message_t *msg_resposta = network_send_receive(rtable, msg_out);

    // verificar a existencia de erros na instrucao put
    if(msg_resposta == NULL || msg_resposta -> opcode == OP_PUT || msg_resposta->opcode == OP_ERROR){
       
        message_destroy(msg_resposta);
        return -1;

    }
    int res = msg_resposta->content.result;

     message_destroy(msg_resposta);

    return res;
}

/* Função para obter um elemento da tabela.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtable_get(struct rtable_t *rtable, char *key){

    if (rtable == NULL || key == NULL)
    return NULL;

    struct message_t *msg_out = (struct message_t *) malloc(sizeof(struct message_t));

    msg_out -> opcode = OP_GET;
    msg_out -> c_type = CT_KEY;
    msg_out -> content.key = key;

    struct message_t * msg_resposta = network_send_receive(rtable, msg_out);

	if(msg_resposta == NULL || msg_resposta -> opcode == OP_GET || msg_resposta->opcode == OP_ERROR){
        message_destroy(msg_resposta);
        return NULL;
    }
	
    struct data_t *d = data_dup(msg_resposta -> content.data);

    message_destroy(msg_resposta);

    return d;

}

/* Funcao para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respectiva operacao rtable_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtable_del(struct rtable_t *rtable, char *key){

    if (rtable == NULL || key == NULL)
    return -1;

    struct message_t *msg_out = (struct message_t *) malloc(sizeof(struct message_t));
    
    msg_out -> opcode = OP_DEL;
    msg_out -> c_type = CT_KEY;
    msg_out -> content.key = key;

    struct message_t *msg_resposta = network_send_receive(rtable, msg_out);

    if(msg_resposta == NULL || msg_resposta -> opcode == OP_DEL || msg_resposta -> opcode == OP_ERROR){
        message_destroy(msg_resposta);
        return -1;
  	}

    int res = msg_resposta->content.result;

     message_destroy(msg_resposta);

    return res;

}

// Devolve o número de elementos da tabela.
int rtable_size(struct rtable_t *rtable){

    if (rtable == NULL)
    return -1;

    struct message_t *msg_out = (struct message_t *) malloc (sizeof(struct message_t));

    msg_out -> opcode = OP_SIZE;
    msg_out -> c_type = CT_NONE;

    struct message_t *msg_resposta = network_send_receive(rtable, msg_out);

    if(msg_resposta == NULL || msg_resposta -> opcode == OP_SIZE || msg_resposta->opcode == OP_ERROR){
        message_destroy(msg_resposta);
        return -1;
    }

    int size = msg_resposta->content.result;

    message_destroy(msg_resposta);

    return size;
}

/* Devolve um array de char* com a copia de todas as keys da tabela,
 * colocando um ultimo elemento a NULL.
 */
char **rtable_get_keys(struct rtable_t *rtable){

    if (rtable == NULL)
    return NULL;

    struct message_t *msg_out = (struct message_t *) malloc(sizeof(struct message_t));
    msg_out -> opcode = OP_GETKEYS;
    msg_out -> c_type = CT_NONE;

    struct message_t *msg_resposta = network_send_receive(rtable, msg_out);

    if (msg_resposta == NULL || msg_resposta->opcode == OP_ERROR){
        message_destroy(msg_resposta);
        return NULL;
    } 
    char **listKeys = keys_dup(msg_resposta->content.keys);

     message_destroy(msg_resposta);

    return listKeys;
}

/* Liberta a memoria alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys){
    
    if (keys == NULL)
        return;
        
    int i = 0;

    while (keys[i] != NULL)
    {
        free(keys[i]);
        i++;
    }

    free(keys);
}


/* Verifica se a operação identificada por op_n foi executada. */
int rtable_verify(struct rtable_t *rtable, int op_n){

    if (rtable == NULL)
        return -1;

    struct message_t *msg_out = (struct message_t *) malloc(sizeof(struct message_t));
    msg_out -> opcode = OP_VERIFY;
    msg_out -> c_type = CT_RESULT;
    msg_out -> content.op_n = op_n;

    struct message_t *msg_resposta = network_send_receive(rtable, msg_out);


    if (msg_resposta == NULL || (msg_resposta->opcode == OP_ERROR && msg_resposta -> c_type == CT_NONE) || msg_resposta->opcode == OP_VERIFY ){
       
        message_destroy(msg_resposta);
        return -1;
    } 
    

    int res = msg_resposta -> content.result;
    
    message_destroy(msg_resposta);

    return res;

}

//----------------METODOS AUXILIARES------------------//

char **keys_dup(char **keys){

    //conta keys
    int i=0;
    while (keys[i] != NULL){i++;}
    
    //copia keys
    char **nkeys = malloc((i + 1)*sizeof(char *));
    for (int j = 0; j < i; j++)
    {
        nkeys[j] = strdup(keys[j]);
    }

    nkeys[i] = NULL;

    return nkeys;
}