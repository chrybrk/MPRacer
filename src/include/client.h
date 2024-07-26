#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "network.h"

void create_network(short port, char *ip);
void *client_loop(void *arg);
void *client_recv(void *arg);

#endif
