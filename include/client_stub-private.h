#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "data.h"
#include "client_stub.h"

/* Remote table. A definir pelo grupo em client_stub-private.h
 */
struct rtable_t{
    char *address_head;
	int porto_head;
	char *address_tail;
	int porto_tail;
	int socket_head;
	int socket_tail;	 
};

char **keys_dup(char **keys);

#endif