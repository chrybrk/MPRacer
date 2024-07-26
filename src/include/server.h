#ifndef __SERVER_H__
#define __SERVER_H__

#include "network.h"

void *server_loop(void *arg);
void *server_tick(void *arg);

#endif
