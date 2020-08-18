#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include "message-private.h"


int network_send_server(int client_socket, struct message_t *msg);

int serialization(struct message_t *msg,uint8_t **buf);


#endif