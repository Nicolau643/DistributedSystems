#ifndef _NETWORK_CLIENT_PRIVATE_H
#define _NETWORK_CLIENT_PRIVATE_H

#include "client_stub.h"
#include "sdmessage.pb-c.h"


int serialization(struct message_t *msg,uint8_t **buf);

struct message_t *deserialization(unsigned int len, uint8_t *msg_buf);

#endif