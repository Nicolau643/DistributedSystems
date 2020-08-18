//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "sdmessage.pb-c.h"
#include "table_skel.h"
#include "table.h"
#include "message-private.h"
#include "inet.h"
#include "network_server.h"
#include "entry.h"
#include "data.h"
#include "client_stub-private.h"
#include "zookeeper/zookeeper.h"

typedef struct String_vector zoo_string; 

struct table_t *table;
struct rtableServer_t *rtable;
struct rtableServer_t *nextNode;

int zoo_data_len;

int last_assigned; // sempre que adiciono uma terefa na queue
int op_count; // incrementado quando a tarefa a realizada
static int is_connected;
char *aux;
 
static zhandle_t *zh;

#define QUEUE_SIZE_MAX 2000

/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024 * 1024

struct task_t *queue_head = NULL;

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	queue_not_empty = PTHREAD_COND_INITIALIZER;

static char *root_path = "/chain";
int new_path_len = 1024;
char* new_path;


int table_skel_init(int n_lists, char *ip, char *porto,char *porto_server){
    
    const char* host_port[100];
    sprintf(host_port,"%s:%s", ip,porto);
    pthread_t thread_main;
    // Reserva espaço de memoria para rtable
    rtable = (struct rtableServer_t *) malloc (sizeof(struct rtableServer_t));

    zoo_string *children_list =	NULL;

    table = table_create(n_lists);

     if (table == NULL)
    {
        return -1;
    }

    last_assigned = 0;
    op_count = 0;

    rtable->porto = atoi(porto);
    rtable->address = ip;
    
    /* Connect to ZooKeeper server */
	zh = zookeeper_init(host_port, connection_watcher,2000, 0, NULL, 0); 
	if (zh == NULL)	{
		fprintf(stderr, "Erro ao contectar-se ao servidor Zookeeper!\n");
	    exit(EXIT_FAILURE);
	}

    printf("Conseguiu conectar se ao Zookeeper\n");

    sleep(3);

    if(is_connected){

        // Verificar se o no /chain existe
        if (ZNONODE == zoo_exists(zh, root_path, 0, NULL)) {
            // Caso nao exista temos de cria-lo
            if (ZOK == zoo_create(zh, root_path, NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
                    fprintf(stderr, "%s created!\n", root_path);
            }else{
                fprintf(stderr, "Erro ao criar o znode atraves do path %s!\n", root_path);
                exit(EXIT_FAILURE);
            }       
        }

        char node_path[120] = "";
        strcat(node_path,root_path); 
        strcat(node_path,"/node");
        new_path = malloc (new_path_len);

        char portServer[100];
        sprintf(portServer,"127.0.0.1:%s",porto_server);
        printf("portserver %s\n",portServer);


        // Criar o nó filho associado ao no /chain
        if (ZOK == zoo_create(zh, node_path, portServer, strlen(portServer), & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, new_path_len)) {
                    fprintf(stderr, "%s created!\n", new_path);
        }else{
            fprintf(stderr, "Error creating znode from path %s!\n", node_path);
            exit(EXIT_FAILURE);
        }
        
        // Guardar o id atribuido ao Znode pelo Zookeeper
        rtable->idZoo = new_path;

        // Obter e fazer watch aos filhos de /chain
        children_list =	(zoo_string *) malloc(sizeof(zoo_string));


        if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, NULL, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", node_path);
			}

        

        // Servidor com o id mais alto aseguir ao nosso
        char *biggerIdNode = malloc(100);
        aux = malloc(1024);
        get_bigger_id(children_list,rtable->idZoo,&biggerIdNode);
        printf("biggerIdNode - %s\n", biggerIdNode);
        printf("myNode - %s\n", rtable->idZoo);
        printf("###########################################\n");
        if (strcmp(biggerIdNode,rtable->idZoo) == 0){
            rtable->idNextZoo = (struct rtableServer_t *) malloc (sizeof(struct rtableServer_t));
            rtable->idNextZoo->idZoo = NULL;
            nextNode = NULL;
        }else{
            nextNode = (struct rtableServer_t *) malloc (sizeof(struct rtableServer_t));
            rtable->idNextZoo = nextNode;
            nextNode->idZoo = biggerIdNode;

            // Obter os meta-dados do nextNode
            
	        char zdata_buf[1024];
            int zdata_len = sizeof(zdata_buf);

            //sleep(2);

            if (ZOK != zoo_get(zh, nextNode->idZoo, 0, zdata_buf, &zdata_len, NULL)){
				fprintf(stderr, "Error getting at %s!\n", nextNode->idZoo);
			}

            char *token;
            token = strtok(zdata_buf, ":");
            nextNode->address = token;
            token = strtok(NULL, ":");
            nextNode->porto = atoi(token);

            connect_to_nextServer(nextNode);

        }

    } 

    /* criação de nova thread */
	if (pthread_create(&thread_main, NULL, &process_task, NULL) != 0){
		perror("\nThread não criada.\n");
		exit(EXIT_FAILURE);
	}

    return 0;
}

void table_skel_destroy(){

    if (table != NULL)
    {
       table_destroy(table);
    }
    
}

int invoke(struct message_t *msg){

    if (msg == NULL || table == NULL)
    {
        return -1;
    }

    int opcode = msg -> opcode;

    if (opcode == OP_BAD)
    {
        /* code */

    }else if (opcode == OP_SIZE)
    {
        int res = table_size(table);
        if (res == -1)
        {
            msg->opcode = OP_ERROR;
            return 0;
        }

        msg -> opcode = msg->opcode + 1;
        msg -> c_type = CT_RESULT;
        
        msg -> content.result = res;
        
        return 0;

    }else if (opcode == OP_DEL)
    {
        /*if(table_del(table,msg->content.key)==-1){
            msg->opcode = OP_ERROR;
            msg->c_type = CT_NONE;
            return 0;
        }

        msg -> opcode = msg->opcode + 1;
        msg ->c_type = CT_NONE;

        return 0;*/


        struct task_t *newTask = malloc(sizeof(struct task_t));

        if (newTask == NULL)
        {
            msg->opcode = OP_ERROR;
            msg->c_type = CT_NONE;
            return 0;
        }
        

        newTask->op_n = last_assigned;
        newTask->op = 0;
        newTask->key = msg->content.key;

        queue_add_task(newTask);

        msg -> opcode = msg->opcode + 1;
        msg ->c_type = CT_RESULT;
        msg -> content.op_n = last_assigned;

        last_assigned++;
        
        return 0;

    }else if (opcode == OP_GET)
    {
        struct data_t *d = table_get(table,msg->content.key);
        

         msg -> opcode = msg->opcode + 1;
         msg -> c_type = CT_VALUE;

         msg -> content.data = d;

         

         return 0;
        
    }else if (opcode == OP_PUT){
        
       /*if (table_put(table,msg->content.entry->key,msg->content.entry->value) == -1)
       {
            msg->opcode = OP_ERROR;
            msg -> c_type = CT_NONE;
            return 0;
       }

        msg -> opcode = msg->opcode + 1;
        msg -> c_type = CT_NONE;

        return 0;*/

        struct task_t *newTask = malloc(sizeof(struct task_t));

        if (newTask == NULL)
        {
            msg->opcode = OP_ERROR;
            msg->c_type = CT_NONE;
            return 0;
        }
        

        newTask->op_n = last_assigned;
        newTask->op = 1;
        newTask->key = strdup(msg->content.entry->key);
        newTask->data = strdup(msg->content.entry->value->data);

        queue_add_task(newTask);

        msg -> opcode = msg->opcode + 1;
        msg ->c_type = CT_RESULT;
        msg -> content.op_n = last_assigned;

        last_assigned++;
        
        return 0;
       

    }else if (opcode == OP_GETKEYS)
    {
        char ** ks = table_get_keys(table);
        

        if (ks == NULL)
        {
            msg->opcode = OP_ERROR;
            return 0;
        }

         msg -> opcode = msg->opcode + 1;
         msg -> c_type = CT_KEYS;

         msg -> content.keys = ks;
         

         return 0;
        

    }else if (opcode == OP_VERIFY)
    {
        msg -> opcode = msg->opcode + 1;
        msg -> c_type = CT_RESULT;

        int v = verify(msg->content.op_n);

        if(v == -1){
            msg -> opcode = OP_ERROR;
            msg -> c_type = CT_NONE;
        }else
        {
            msg->content.result = verify(msg->content.op_n);
        }
        return 0;
    }
    
    return -1;
}


/* Verifica se a operação identificada por op_n foi executada. */
int verify(int op_n){

    int verif = 1;
    //printf("op_n -> %d\n", op_n);
    //printf("op_count -> %d\n", op_count);

    // Para combater o primeiro caso de o cliente fizer verify 0 e ainda nao tiver feito nenhuma op.
    if (op_count == 0)
    {
        return -1;
    }

    if (op_n <= op_count){
        verif = 0;
    }else if(op_n > last_assigned){
        return -1;
    }
    
    return verif;
}

/* Função do thread secundário que vai processar pedidos de escrita.
*/
void *process_task (void *params){

    while (params == NULL){
        
        struct task_t *tarefa = queue_get_task();

        pthread_mutex_lock(&table_lock);

        if (tarefa->op == 1){
            if(table_put(table,tarefa->key,data_create2(strlen(tarefa->data),tarefa->data)) == 0)
                op_count = op_count + 1;
        }else if (tarefa->op == 0){
            if(table_del(table, tarefa->key)==0)
                ++op_count;  
        }
        
        pthread_mutex_unlock(&table_lock);        

    }

    int *res = 0;
    return res;
}

void queue_add_task(struct task_t *task){

    pthread_mutex_lock(&queue_lock);

    if(queue_head==NULL) { /* Adiciona na cabeça da fila */
        queue_head = task; 
        task->next=NULL;
    } else { /* Adiciona no fim da fila */
        struct task_t *tptr = queue_head;
        while(tptr->next != NULL) tptr = tptr->next;
        tptr->next=task;
        task->next=NULL;
    }

    pthread_cond_signal(&queue_not_empty); /* Avisa um bloqueado nessa condição */
    pthread_mutex_unlock(&queue_lock);
    
}

struct task_t *queue_get_task() {

    pthread_mutex_lock(&queue_lock);

    while(queue_head==NULL)
        pthread_cond_wait(&queue_not_empty, &queue_lock); /* Espera haver algo */
    struct task_t *task = queue_head; 
    queue_head = task->next;
    pthread_mutex_unlock(&queue_lock);

    return task;
}

/**
* Watcher function for connection state change events
*/
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1; 
		} else {
			is_connected = 0; 
		}
	} 
}

/**
* Data Watcher function for /MyData node
*/
void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
	 	   /* Get the updated children and reset the watch */ 

 			if (ZOK != zoo_wget_children(zh, root_path, child_watcher, NULL, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", root_path);
 			}
            // Servidor com o id mais alto aseguir ao nosso
            char *biggerIdNode = malloc(100);
            get_bigger_id(children_list,rtable->idZoo,&biggerIdNode);
           
            if (strcmp(biggerIdNode,rtable->idZoo) == 0){
                
                rtable->idNextZoo = NULL;
                nextNode = NULL;
            }else{
                //nextNode = (struct rtableServer_t *) malloc (sizeof(struct rtableServer_t));
                //rtable->idNextZoo = nextNode;
                //aux = malloc(1024);
                
                if(rtable->idNextZoo->idZoo == NULL){
                    
                    aux = "a";
                }else{
                    aux = rtable->idNextZoo->idZoo;
                }
                
                if (strcmp(aux,biggerIdNode) != 0)
                {
                    nextNode = (struct rtableServer_t *) malloc (sizeof(struct rtableServer_t));
                    rtable->idNextZoo = nextNode;
                    nextNode->idZoo = biggerIdNode;

                    /* char *node = strdup(nextNode->idZoo);
                    char *token3;
                    token3 = strtok(node, "/");
                    token3 = strtok(NULL, "/"); */

                    // Obter os meta-dados do nextNode
                    
                    char zdata_buf[1024];
                    int zdata_len =sizeof(zdata_buf);

                    //sleep(4);

                   

                    if (ZOK != zoo_get(zh, nextNode->idZoo, 0, zdata_buf, &zdata_len, NULL)){
                        fprintf(stderr, "Error getting at %s!\n", nextNode->idZoo);
                    }
                    

               

                    char *token;
                    token = strtok(zdata_buf, ":");
                    nextNode->address = token;
                    token = strtok(NULL, ":");
                    nextNode->porto = atoi(token);
                    //printf("npassou pelo next_server\n");
                   
                
                    connect_to_nextServer(nextNode);
                   
                     //free(aux);
                    }
                   
            }

			
		} 
	 }
	 free(children_list);
}

// Funçao que retorna o nó filho com o id mais alto
void get_bigger_id(zoo_string* children_list, char* myNode, char **b){

    char *node = strdup(myNode);
    char *token;
    token = strtok(node, "/");
    token = strtok(NULL, "/");

    printf("######### Impressao dos Nós Filhos - (%d) ########\n", children_list->count);

    for (int i = 0; i < children_list->count; i++)  {

        printf("Filho %d - %s\n",i, children_list->data[i]);

        if (strcmp(token,children_list->data[i]) == 0){
            if (i == (children_list->count -1) ){
                printf("###########################################\n");
                *b = strdup(myNode);
                return;
            }else{
                printf("Filho %d - %s\n",i+1, children_list->data[i+1]);
                printf("###########################################\n");
                sprintf(*b,"/chain/%s",children_list->data[i+1]);
                return;
            }
                
        }
	}
    printf("###########################################\n");
    return;

}

// Funçao que conecta o Servidor ao Servidor seguinte
void connect_to_nextServer(struct rtableServer_t *rtable){

    struct sockaddr_in server;

    //if(rtable == NULL) return -1;

    // Cria o socket TCP
    if ((rtable -> socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        return;
    }


    char *ip = strdup(rtable -> address);

    // Preenche a estrutura server para estabelecer conexao
    server.sin_family = AF_INET;
    server.sin_port = htons(rtable -> porto);
    server.sin_addr.s_addr = *ip; //Endereco IP, formato rede

    if(inet_pton(AF_INET, ip, &server.sin_addr) < 1){
        printf("Erro ao converter IP\n" );
        network_close(rtable);
        return ;
    }

    // Estabelece conexao com o servidor definido em server
    if (connect(rtable ->socket,(struct sockaddr *)&server, sizeof(server)) < 0) {
	    perror("Erro ao conectar-se ao servidor");
	    close(rtable-> socket);
	    return ;
	}

    //return 0;

}