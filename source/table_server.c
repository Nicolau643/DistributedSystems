//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network_server.h"

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        printf("numero de argumentos incorreto\n");
        printf("Exemplo de uso: table-server 12345 5 localhost:2181");
        exit(-1);
    }

    char *token;
    char *arg = strdup(argv[3]);

    token = strtok(arg, ":");
    char *ip = token;
    printf("%s",ip);
    token = strtok(NULL, ":");
    char *porto = token;
    printf("%s",porto);
    table_skel_init(atoi(argv[2]), ip, porto,argv[1]);

    int socket = network_server_init((short)atoi(argv[1]));
    if (socket == -1) {
        return -1;
    }

    printf("A iniciar servidor no socket %d\n",socket);

    int result = network_main_loop(socket);
    
    if (result == -1)
    {
        printf("erro no servidor\n");
    }
    
    table_skel_destroy();

    return 0;
}
