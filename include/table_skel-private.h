#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

#include "data.h"
#include "table_skel.h"

/* Remote table. A definir pelo grupo em client_stub-private.h
 */
struct rtableServer_t{
    char *address;
	int porto;
	int socket;	 
	char *idZoo;
	struct rtableServer_t *idNextZoo;
};

#endif