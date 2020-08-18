//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub.h"
#include "table_client-private.h"

struct rtable_t *table;

int main(int argc, char **argv) {

   
    char *input;
    char *key;
    char *data;

    // possibilidade: substituir strstr por um regex para confirmar formato correto
    if (argc != 2) {
        printf ("Uso: <ip zookeeper>:<porta zookeeper>\n");
        return 0;
    }

    table = rtable_connect(argv[1]);

    if (table == NULL) {
        printf("Erro ao conectar ao zookeeper\n");
        return 0;
    }

    printf("Olá, Bem-vindo!\n");

    while (1) {

        //SEM CERTEZA
        input = malloc(INPUT_SIZE);
        key = malloc(KEY_SIZE);
        data = malloc(DATA_SIZE);

        printf(">>> ");
        if (fgets(input, INPUT_SIZE, stdin) == NULL)
        {
            printf("ocorreu um erro\n");
            free(input);
            free(key);
            free(data);
            exit(-1);
        }
        
        //char **inputs = sanitizeInput(input);

        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }

        char* com = strtok(input, " ");

        if (strcmp(com, "quit") == 0) {
            return rtable_disconnect(table);

        } else if (strcmp(com, "put") == 0) {
          
            key = strtok(NULL, " ");
            data = strtok(NULL, " ");
            int op;
            if (key == NULL || data == NULL)
            {
                printf("Erro ao fazer a instrucao (put)\n");
                printf("put (key) (data)\n");
                continue;
            }
            
            if ((op = rtable_put(table, entry_create(key, data_create2(strlen(data), data)))) != -1) {
                printf("Operação put com sucesso! Operacao numero: %d\n", op + 1);
            } else {
                printf("Erro ao inserir na tabela\n");
            }

        } else if (strcmp(com, "get") == 0) {
            key = strtok(NULL, " ");
            struct data_t *data;

            if ((data = rtable_get(table, key)) != NULL) {
                
                //printf("data s %d\n", data->datasize);

                if (strcmp(data->data," ") == 0) {
                    printf("Valor não encontrado para a key: %s\n", key);
                } else {
                    printf("Data: %s\n", (char*) data->data);
                    // printf("Operação get com sucesso\n");
                }
                data_destroy(data);
            } else {
                printf("Erro ao obter data da key: %s\n", key);
            }

        } else if (strcmp(com, "del") == 0) {
            key = strtok(NULL, " ");
            int op;
            if ((op = rtable_del(table, key)) != -1) {
                printf("Operação delete com sucesso! Operacao numero: %d\n",op + 1);
            } else {
                printf("Erro ao apagar entrada da key: %s\n", key);
            }

        } else if (strcmp(com, "size") == 0) {
            int size;
            printf("Pedido size enviado\n");
            if ((size = rtable_size(table)) != -1) {
                printf("Tamanho da tabela: %d\n", size);
            } else {
                printf("Erro ao obter tamanho da tabela\n");
            }

        } else if (strcmp(com, "getkeys") == 0) {
            char **keys;

            if ((keys = rtable_get_keys(table)) != NULL) {
                int i = 0;
                printf("Impressao das Keys:\n");
                if (keys[0] == NULL) {
                    printf("Não existem dados na tabela\n");
                }
                while (keys[i] != NULL) {
                    printf("Key %d: %s\n", i, keys[i]);
                    i++;
                }
                //rtable_free_keys(keys);
            } else {
                printf("Erro ao obter keys\n");
            }

        } else if (strcmp(com, "verify") == 0){
            char* op_n = strtok(NULL, " ");
            int res_op;
            if((res_op = rtable_verify(table, atoi(op_n))) != -1){
                
                if (res_op == 0){
                    printf("A operação já foi executada\n");
                }else{
                    printf("A operação ainda nao foi executada\n");
                } 
            }else{
                 printf("Erro ao obter verificar\n");
            }

        } else {
            printf("Comando não suportado: %s\n", strtok(NULL, " "));
        }
    }
    return rtable_disconnect(table);
    
}
