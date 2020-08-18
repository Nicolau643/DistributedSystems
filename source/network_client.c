//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network_client.h"
#include "network_client-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "serialization.h"
#include "inet.h"
#include "read_write_all.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include <unistd.h>



int connect_server(int *soc, char *address, int port){


    struct sockaddr_in server;

    // Cria o socket TCP
    if ((*soc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    char *ip = strdup(address);

    // Preenche a estrutura server para estabelecer conexao
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = *ip; //Endereco IP, formato rede

    if(inet_pton(AF_INET, ip, &server.sin_addr) < 1){
        printf("Erro ao converter IP\n" );
        //network_close(rtable);
        return -1;
    }

    // Estabelece conexao com o servidor definido em server
    if (connect(*soc,(struct sockaddr *)&server, sizeof(server)) < 0) {
	    perror("Erro ao conectar-se ao servidor");
	    close(*soc);
	    return -1;
	}

    return 0;

}



/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtable;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtable;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtable_t *rtable){


    if(rtable == NULL) return -1;

   
    if (rtable -> porto_head == rtable -> porto_tail){
        
       
       if (connect_server(&rtable->socket_head,rtable -> address_head,rtable -> porto_head) == -1){
            return -1;
        }
    }else{

       
        if (connect_server(&rtable->socket_head,rtable -> address_head,rtable -> porto_head) == -1){
            return -1;
        }
    
        if (connect_server(&rtable->socket_tail,rtable -> address_tail,rtable -> porto_tail) == -1)
        {
        return -1;
        }
    }

    
    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtable_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct message_t *network_send_receive(struct rtable_t * rtable, struct message_t *msg){

    if(rtable == NULL) return NULL;
    if(msg == NULL) return NULL;

    uint8_t *buf;
    int socket;

    msg -> client_server = 1;
    

    // Serializacao
    int message_size = serialization(msg,&buf);
    printf("A sua msg foi serializada para chegar em seguranca ao servidor!\n");

    if(message_size < 0){
        perror("Erro a serializar dados do Cliente");
        return NULL;
    }

    //Enviar Tamanho mensagem
    int message_size_net = htonl(message_size);


    
    if (msg->opcode == OP_DEL || msg->opcode == OP_PUT){
            
            socket = rtable -> socket_head;
    }else{
           
            socket = rtable -> socket_tail;

    }
    
    if (rtable -> porto_head == rtable -> porto_tail)
    {
        socket = rtable -> socket_head;
    }




    int result = write_all(socket,&message_size_net, sizeof(int));
    // printf("write_all tamanho result: %d\n", result);

        if (result != sizeof(int)){
            perror("Erro ao enviar dados ao servidor");
            //close(rtable-> socket);
            return NULL;
        }

    // printf("message size = %d\n",message_size);
    printf("O seu socket = %d\n", socket);
    // Envio da mensagem
    result = write_all(socket, buf, message_size);
    //printf("write_all mensagem %d\n", result);
    if (result != message_size){
        perror("Erro ao enviar dados ao servidor");
        //close(rtable-> socket);
        return NULL;
    }
    free(buf);
    //message_destroy(msg); //TODO erro free invalid pointers
    //printf("destroy message\n");

    printf("A espera de resposta do servidor ...\n");

    // Simulaçao de Tempo
    //sleep(1);

    // Rececao do tamanho da mensagem e alocacao
    char *size = malloc(sizeof(int));
    int size_net;
    int resp = read_all(socket,size,sizeof(int));

    if (resp != sizeof(int)){
        perror("Erro ao receber dados do servidor");
        free(size);
        //close(rtable-> socket);
        return NULL;
    }
    memcpy(&size_net,size,sizeof(int));
    int size_msg = htonl(size_net);
    free(size);

    uint8_t *msg_buf = malloc(size_msg);

    // Rececao da mensagem
    resp = read_all(socket,msg_buf,size_msg);
    
    if (resp != size_msg){
        perror("Erro ao receber dados do servidor");
        free(msg_buf);
        //close(rtable-> socket);
        return NULL;
    }

    struct message_t *msg_receive = deserialization((unsigned int) size_msg,msg_buf);
    if (msg_receive == NULL)
    {
        free(msg_buf);
        return NULL;
    }

    
    free(msg_buf);
    
    return msg_receive;

}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtable_t * rtable){

    if(rtable == NULL) return -1;

    int res = close(rtable -> socket_head);
    
    if(res < 0){
        perror("Erro ao fechar o socket");
        return -1;
    }

    res = close(rtable -> socket_tail);

    if(res < 0){
        perror("Erro ao fechar o socket");
        return -1;
    }

    free(rtable->address_head);
    free(rtable->address_tail);
    free(rtable);
    return res;

}

//----------------METODOS AUXILIARES------------------//


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

struct message_t* deserialization(unsigned int len, uint8_t *msg_buf){

    SDMessage *recv_msg = NULL;
    
    recv_msg = sdmessage__unpack(NULL, len, msg_buf);
    if (recv_msg == NULL) {
        return NULL;
    } 

    struct message_t *m = malloc(sizeof(struct message_t));

    m->opcode = recv_msg->opcode;
    m->c_type = recv_msg->c_type;

    if (m->opcode == OP_ERROR){
        return m;
    }
    
  

    if (!((recv_msg->opcode == 11 && recv_msg->c_type == 50) || (recv_msg->opcode == 21 && recv_msg->c_type == 50) ||
        (recv_msg->opcode == 31 && recv_msg->c_type == 20) || (recv_msg->opcode == 41 && recv_msg->c_type == 50) ||
        (recv_msg->opcode == 51 && recv_msg->c_type == 40)||(recv_msg->opcode == 61 && recv_msg->c_type == 50))){

        sdmessage__free_unpacked(recv_msg, NULL);
        return NULL;
    }

    if (m->c_type == CT_RESULT)
    {
        if( m->opcode == OP_PUT || m->opcode == OP_DEL){
            m->content.op_n = recv_msg -> op_n;
        }else{
             m->content.result = recv_msg -> result;
        }
       

    }else if ( m->c_type == CT_VALUE )
    {
        //m -> content.data = buffer_to_data(recv_msg->data,recv_msg->data_size);
        // printf("data %s\n", recv_msg -> data);
        m -> content.data = data_create2(recv_msg -> data_size,strdup(recv_msg -> data));

    }else if (m->c_type == CT_KEYS)
    {
        int i;
        int numKeys = recv_msg->numkeys;
        m -> content.keys = malloc((numKeys+1)*sizeof(char *));
        for (i = 0; i < numKeys; i++)
        {
            m -> content.keys[i] = strdup(recv_msg -> keys[i]);
        } 

        m -> content.keys[numKeys] = NULL;

    }
    
    sdmessage__free_unpacked(recv_msg, NULL);
    
    return m;

}