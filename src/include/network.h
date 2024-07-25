#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

typedef enum {
	NOOP = 0,
	GET_ID,
	JOIN,
	UPDATE,
	SET_KEY,
	ERROR,
	SUCCESS
} kind_T;

typedef enum {
	// JOIN
	NEW_PLAYER_JOINED = 0,
	
	// UPDATE
	GAME_HAS_STARTED,
	SET_POSITION
} action_T;

typedef struct {
	int id;
	int kind;
	int action;
	int position[2];
} packet_T;

typedef struct {
	int sockfd;
	struct sockaddr_in addr;
} network_T;

void net_send(network_T *network, void *packet, size_t n, struct sockaddr_in addr);
struct sockaddr_in net_recv(network_T *network, void *buffer, size_t n);

network_T init_server(uint64_t port, char *ip_addr);
network_T init_client();

void server_bind(network_T *network);
void client_bind(network_T *network);

void close_network(network_T *network);

#endif
