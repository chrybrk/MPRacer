#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "raylib.h"
#include "network.h"
#include "utils.h"

extern int ID;

extern bool Created;
extern bool NotCreatedButJoinEventCalled;
extern bool HasStarted;

extern unsigned int MaxPlayer;

extern InputBox IPaddrIB;

extern network_T Server;
extern network_T Client;

extern pthread_t ServerLoop;
extern pthread_t ClientLoop;

extern Rectangle players[4];

#endif
