#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define __ANDROID__ 0

#include "raylib.h"
#include "network.h"
#include "utils.h"

typedef struct {
	float maxSpeed;
	float acceleration;
	float braking;
	float turnSpeed;
	float friction;
	Vector2 origin;
	float rotation;
	Vector2 position;
	Vector2 velocity;
	Vector2 rectSize;
} Car;

extern int ID;

extern bool Created;
extern bool NotCreatedButJoinEventCalled;
extern bool HasStarted;

extern unsigned int MaxPlayer;

extern InputBox IPaddrIB;

extern network_T Server;
extern network_T Client;

extern pthread_t ServerLoop, ServerSend, ClientLoop, ClientRecv;

extern bool IsCustomKeyboardActive;
extern int CustomLastKeyInput;
extern char CustomLastKeyCharInput;
extern bool hasClickedOnKey;

extern Car players[4];

extern packet_T *recvpackets[64 * 1024 * 1024];
extern int recvpackets_index; 

#endif
