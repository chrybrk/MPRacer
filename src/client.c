#include "include/client.h"
#include "include/global.h"
#include "include/physics.h"

void create_network(short port, char *ip)
{
	Server = init_server(port, ip);
	Client = init_client();

	client_bind(&Client);

	net_send(
			&Client, 
			(void*)&(packet_T){ ID, JOIN, 0, { 0, 0 } }, 
			sizeof(packet_T), 
			Server.addr
	);

	packet_T *packet = malloc(sizeof(packet_T));
	net_recv(&Client, packet, sizeof(packet_T));

	pthread_create(&ClientLoop, NULL, client_loop, &Client);
	// pthread_create(&ClientRecv, NULL, client_recv, &Client);
	Created = packet->kind == SUCCESS ? true : false;

	if (Created)
		ID = packet->id;

	return;
}

void *client_loop(void *arg)
{
	network_T *network = (network_T*)arg;

	while (true)
	{
		packet_T *buffer = malloc(sizeof(packet_T));
		struct sockaddr_in addr = net_recv(network, buffer, sizeof(packet_T));

		switch (buffer->kind)
		{
			case JOIN:
			{
				if (buffer->action == NEW_PLAYER_JOINED && !HasStarted)
					MaxPlayer++;

				break;
			}

			case UPDATE:
			{
				switch (buffer->action)
				{
					case GAME_HAS_STARTED:
					{
						Vector2 start_pos = { 100, 100 };
						float rotation = 90;
						for (int i = 0; i < MaxPlayer; ++i)
						{
							players[i] = InitPlayer(
									(Vector2){ 0, 0 },
									(Vector2){ 100.0f, 50.0f },
									(CarProperty){
										600.0f,
										300.0f,
										400.0f,
										300.0f,
										0.99f
									}
							);
						}

						HasStarted = true;
						break;
					}
					
					case SET_POSITION:
					{
						recvpackets[recvpackets_index] = buffer;
						recvpackets_index++;

						/*
						if (buffer->id != ID)
						{
							players[buffer->id].position.x = buffer->position[0];
							players[buffer->id].position.y = buffer->position[1];
							players[buffer->id].rotation = buffer->rotation;
						}
						*/
						break;
					}
				}

				break;
			}

			case SET_KEY:
			{
				switch (buffer->action)
				{
					// case KEY_W: players[buffer->id].y += 10; break;
					// case KEY_S: players[buffer->id].y -= 10; break;
				}

				break;
			}
		}
	}

	// usleep(60);
}

void *client_recv(void *arg)
{
	network_T *network = (network_T*)arg;
	struct timeval start, stop;
	double time_interval;

	while (true)
	{
    gettimeofday(&start, NULL);
		printf("CLIENT :: total_packets: %d\n", recvpackets_index);
		
		for (int i = 0; i < recvpackets_index; ++i)
		{
			packet_T *buffer = recvpackets[i];
			if (buffer)
			{
				if (buffer->id != ID)
				{
					players[buffer->id].position.x = buffer->position[0];
					players[buffer->id].position.y = buffer->position[1];
					players[buffer->id].angle = buffer->rotation;
				}
			}
		}

		recvpackets_index = 0;
		gettimeofday(&stop, NULL);
		time_interval = (double) (stop.tv_usec - start.tv_usec);
		if ((double) (stop.tv_usec - start.tv_usec) > 0) {
				time_interval = (double) (stop.tv_usec - start.tv_usec);
		}
		usleep(10000 - time_interval);
	}
}
