#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "table.h"
#include "client_stub-private.h"
#include "zookeeper/zookeeper.h"
#include "table_skel-private.h"

extern struct rtableServer_t *nextNode;

struct task_t {
    int op_n; //o número da operação
    int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
    char* key; //a chave a remover ou adicionar
    char* data; // os dados a adicionar em caso de put, ou NULL em caso de delete
    struct task_t *next; // proxima task
};


/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists, char *ip, char *porto,char *porto_server);

/* Liberta toda a memória e recursos alocados pela função table_skel_init.
 */
void table_skel_destroy();

/* Executa uma operação na tabela (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, tabela nao incializada)
*/
int invoke(struct message_t *msg);

/* Verifica se a operação identificada por op_n foi executada. */
int verify(int op_n);

/* Função do thread secundário que vai processar pedidos de escrita.
*/
void *process_task (void *params);

// Função que adiciona uma nova task a fila
void queue_add_task(struct task_t *task);

// Função que retorna a task que estiver na fila
struct task_t *queue_get_task();

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

/**
* Data Watcher function for /MyData node
*/
void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx);

// Funçao que retorna o nó filho com o id mais alto
void get_bigger_id(struct String_vector * children_list,char* myNode,char **b);

// Funçao que conecta o Servidor ao Servidor seguinte
void connect_to_nextServer(struct rtableServer_t *rtable);

#endif
