//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inet.h"

#include "serialization.h"
#include "read_write_all.h"
#include "network_server.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "table_skel.h"
#include "entry.h"
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include "table_skel-private.h"
#include "network_server_private.h"

#define NFDESC 6 // Número de sockets (um para listening)
#define TIMEOUT 30000 // em milisegundos

int sockfd,client_socket;
struct sockaddr_in server,client;
socklen_t size_client;
struct pollfd connections[NFDESC]; // Estrutura para file descriptors das sockets das ligacoes
char str[MAX_MSG + 1];
int nbytes, count, nfds, kfds, i;
int is_server = 0;
int network_server_init(short port){

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }
    
    server.sin_family = AF_INET;
    server.sin_port = htons(port); 
    server.sin_addr.s_addr = htonl(INADDR_ANY); 

    
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 0) < 0){
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    }


    return sockfd;

}




int network_main_loop(int listening_socket){

     for (i = 0; i < NFDESC; i++){
        connections[i].fd = -1; // poll ignora estruturas com fd < 0
    }

    connections[0].fd = sockfd;  // Vamos detetar eventos na welcoming socket
    connections[0].events = POLLIN;  // Vamos esperar liga��es nesta socket
    


    nfds = 1; // n�mero de file descriptors

    // Retorna assim que exista um evento ou que TIMEOUT expire. * FUN��O POLL *.
    while ((kfds = poll(connections, nfds, TIMEOUT)) >= 0){ // kfds == 0 significa timeout sem eventos

        if (kfds > 0){ // kfds � o n�mero de descritores com evento ou erro

            if ((connections[0].revents & POLLIN) && (nfds < NFDESC)){  // Pedido na listening socket ? tratamento de erro 
                if ((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0){ // Liga��o feita ?
                    printf("----------------------------\n");
					printf("Recebi um pedido do Cliente: %d!\n", nfds);
					printf("----------------------------\n");
                    connections[nfds].events = POLLIN; // Vamos esperar dados nesta socket
                    nfds++;
                }
            }

            for (i = 1; i < nfds; i++){  // Todas as liga��es

                if (connections[i].revents & POLLIN) { // Dados para ler ?
                    
                    struct message_t *m = network_receive(connections[i].fd);
                   
                    
                    if(m == NULL){
                        printf("Mesagem a null recebida pelo servidor\n");
                        close(connections[i].fd);
                        nfds--;
					    connections[i].fd = -1;
                        continue;
                    }

                    if (nextNode != NULL){
                      
                        if(network_send_server(nextNode->socket,m) == -1){
                            printf("Erro ao enviar mensagem para o Servidor seguinte!\n");
                            continue;
                        }

                    }
                        
                    m -> client_server = 1;
                    
                    printf("Mensagem recebida com sucesso\n");
                    
                    if(invoke(m) == -1){
						perror("Erro no invoke");
						close(connections[i].fd);
						connections[i].fd = -1;
						continue;
                    }

                    printf("Invoke feito com sucesso\n");
                    
                   

                    if (m->client_server != 0)
                    {
                        if(network_send(connections[i].fd,m) == -1){
                            printf("Erro ao enviar mensagem para o Cliente!\n");
                            close(connections[i].fd);
                            nfds--;
                            connections[i].fd = -1;
                            continue;
                        }
                    }
                           
                    /*
                    if(network_send(nextNode->socket,mServer) == -1){
                        printf("Erro ao enviar mensagem para o Servidor seguinte!\n");
                        continue;
                    }
                    */
                    printf("Mensagem envidada com sucesso\n\n");   
                }
                
                if (connections[i].revents & POLLHUP) {
                    printf("------O Cliente número %d foi diconnectado!-----", i);
					close(connections[i].fd);
                    nfds--;
					connections[i].fd = -1;
				}
                
            }

        }

    }

// Deixar todos os slots livres quando o servidor terminar de estar a escuta
    for (int m = 0; m < nfds; m++) {
		if (connections[m].fd != -1) {
			close(connections[m].fd);
            nfds--;
			connections[m].fd = -1;
		}
	} 

    network_server_close();
   
    return 0;
}

struct message_t *network_receive(int client_socket){

    char *buf = malloc(sizeof(int));
    int l_aux_net;

    if (buf == NULL)
    {
        return NULL;
    }
    
    int resp = read_all(client_socket,buf,sizeof(int));
    if (resp == -1)
    {
        free(buf);
        return NULL;
    }
    
    //buf[sizeof(int)] = '\0';
    memcpy(&l_aux_net,buf,sizeof(int));
    int msg_size = ntohl(l_aux_net);
    free(buf);

    uint8_t *msg_buf = malloc(msg_size);
     if (msg_buf == NULL)
    {
        return NULL;
    }

    resp = read_all(client_socket,msg_buf,msg_size);
    if (resp == -1)
    {
        free(msg_buf);
        return NULL;
    }

    SDMessage *m = NULL;

    m = sdmessage__unpack(NULL, resp, msg_buf);

    if (m == NULL)
    {
        free(msg_buf);
        return NULL;
    }

    struct message_t *msg = malloc(sizeof(struct message_t));
    
    msg -> opcode = m -> opcode;
    msg -> c_type = m -> c_type;

    printf("OP code da mensagem recebida pelo Cliente = %d\n",m->opcode);
    printf("CT da mensagem recebida pelo Cliente = %d\n",m->c_type);
    
    if (msg -> c_type == CT_ENTRY)
    {
        
        msg -> content.entry = entry_create(strdup(m -> key), data_create2(m->data_size,strdup(m->data)));
        printf("Recebi a entry - key -> %s - data -> %s\n", m->key, m->data);
    } else if(msg -> c_type == CT_KEY)
    {
        // printf("m key %s\n", m -> key);
        msg -> content.key = strdup(m -> key);
        printf("Recebi a key -> %s \n", m->key);
        
    }else if(msg -> c_type == CT_RESULT){

        msg -> content.op_n = m -> op_n;
        printf("Recebi o nº op -> %d \n", m->op_n);
    }
    
    sdmessage__free_unpacked(m, NULL);
    printf("Unpacked da mensagem concluido\n");

    

    return msg;

}

int network_send(int client_socket, struct message_t *msg){

    SDMessage msg_s;
    uint8_t *buf = NULL;
    unsigned len = 0;

    sdmessage__init(&msg_s);

    msg_s.opcode = msg -> opcode;
    msg_s.c_type = msg -> c_type;

    

    if (msg -> c_type == CT_RESULT)
    {   
         if( msg->opcode == OP_PUT || msg->opcode == OP_DEL){
           msg_s.op_n = msg -> content.op_n;
        }else{
            msg_s.result = msg -> content.result;
        }
        
        len = sdmessage__get_packed_size(&msg_s);

         buf = malloc(len);
        if (buf == NULL) 
        {
           return -1;
        }

    }else if (msg -> c_type == CT_VALUE)
    {
      
        msg_s.data_size = msg->content.data->datasize;
        msg_s.data = msg->content.data->data;
        //int buf_size = data_to_buffer(msg->content.data, &data_ser);

        len = sdmessage__get_packed_size(&msg_s);

        buf = malloc(len);
        if (buf == NULL) 
        {
            return -1;
        }

    }else if (msg -> c_type == CT_KEYS)
    {
        int numKs = 0;
        int memCount = 0;
        while (msg->content.keys[numKs] != NULL){
            memCount += strlen(msg->content.keys[numKs]) + 1;
            numKs++;
        }

        msg_s.numkeys = numKs;
        msg_s.n_keys = numKs;
       
        msg_s.keys = malloc((memCount + 1) * sizeof(char *));
        for (int i = 0; i < numKs; i++) {
            msg_s.keys[i] = strdup(msg->content.keys[i]);
        }
        msg_s.keys[numKs] = NULL;
        
        
        len = sdmessage__get_packed_size(&msg_s);

        buf = malloc(len);
        if (buf == NULL) 
        {
            int i = 0;
            while(msg->content.keys[i] != NULL) {
                free(msg->content.keys[i]);
            }
            free(msg->content.keys);
            return -1;
        }
       
    }else if (msg -> c_type == CT_NONE)
    {
        len = sdmessage__get_packed_size(&msg_s);

        buf = malloc(len);
        if (buf == NULL) 
        {
           return -1;
        }
    }

    sdmessage__pack(&msg_s, buf);

    int len_net = htonl(len);

    int size_sent = write_all(client_socket,&len_net,sizeof(int));
    if (size_sent == -1)
    {
        free(buf);
        return -1;
    }
    
    size_sent = write_all(client_socket,buf,len);

    if (size_sent == -1)
    {
        free(buf);
        return -1;
    }
    
    //message_destroy(msg);
    
    return 0;



}
int network_server_close(){

    return close(sockfd);
}


/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtableServer_t * rtable){

    if(rtable == NULL) return -1;

    int res = close(rtable -> socket);
    
    if(res < 0){
        perror("Erro ao fechar o socket");
        return -1;
    }

    free(rtable->address);
    free(rtable->idZoo);
    free(rtable->idNextZoo);
    return res;

}

int network_send_server(int client_socket, struct message_t *msg){

    uint8_t *buf;

    msg->client_server = 0;
   
    // Serializacao
    int message_size = serialization(msg,&buf);
    printf("A sua msg foi serializada para chegar em seguranca ao servidor!\n");

    if(message_size < 0){
        perror("Erro a serializar dados do Cliente");
        return -1;
    }

    //Enviar Tamanho mensagem
    int message_size_net = htonl(message_size);

    int result = write_all(client_socket,&message_size_net, sizeof(int));
    // printf("write_all tamanho result: %d\n", result);
    
        if (result != sizeof(int)){
            perror("Erro ao enviar dados ao servidor");
            //close(rtable-> socket);
            return -1;
        }

    // printf("message size = %d\n",message_size);
    printf("O seu socket = %d\n", client_socket);

    // Envio da mensagem
    result = write_all(client_socket, buf, message_size);
    //printf("write_all mensagem %d\n", result);

    if (result != message_size){
        perror("Erro ao enviar dados ao servidor");
        //close(rtable-> socket);
        return -1;
    }

    free(buf);

    return 0;

}

int serialization(struct message_t *msg,uint8_t **buf){

    unsigned len;
    SDMessage sdmsg;
    
    sdmessage__init(&sdmsg);

    sdmsg.opcode = msg -> opcode;
    sdmsg.c_type = msg -> c_type;
   
    if (msg->opcode == OP_DEL || msg->opcode == OP_GET ){
        
        sdmsg.key = msg -> content.key ;
    
    }else if (msg->opcode == OP_PUT){
        
        sdmsg.key = msg->content.entry->key;
        sdmsg.data = msg->content.entry->value->data;
        sdmsg.data_size = msg->content.entry->value->datasize;
    }else if(msg->opcode == OP_VERIFY){
        sdmsg.op_n = msg->content.op_n;
    }

    len = sdmessage__get_packed_size(&sdmsg);
    
    *buf = malloc(len);
    if (buf == NULL) {
        return -1;
    }

    sdmessage__pack(&sdmsg, *buf);
    
    return len;

}