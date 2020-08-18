#ifndef _READ_WRITE_ALL_H
#define _READ_WRITE_ALL_H

#include "inet.h"

int write_all(int sockfd, void *buf, int len);

int read_all(int sockfd, void *buf, int len);

#endif