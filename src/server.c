#include "include/server.h"
#include "include/global.h"

#define MAX_CLIENTS 4

int IDs = -1;
struct sockaddr_in clients[4];
unsigned int client_index = 0;

packet_T *packets[64 * 1024 * 1024];
int packetrecv_index = 0;

void *server_loop(void *arg)
{
	network_T *network = (network_T*)arg;

	while (true)
	{
		packet_T *buffer = malloc(sizeof(packet_T));
		struct sockaddr_in client = net_recv(network, buffer, sizeof(packet_T));

		switch (buffer->kind)
		{
			case JOIN:
			{
				IDs++;

				packet_T packet;
				if (client_index < MAX_CLIENTS)
				{
					clients[client_index] = client;
					client_index++;

					packet.id = IDs;
					packet.kind = SUCCESS;
					packet.action = 0;
				}
				else
				{
					packet.kind = ERROR;
					packet.action = 0;
				}

				net_send(network, &packet, sizeof(packet_T), client);

				for (int i = 0; i < IDs; ++i)
				{
					struct sockaddr_in addr = clients[i];

					packet_T packet = { IDs, JOIN, NEW_PLAYER_JOINED, { 0, 0 } };
					net_send(network, &packet, sizeof(packet_T), addr);
				}

				break;
			}

			case UPDATE:
			{
				switch (buffer->action)
				{
					case GAME_HAS_STARTED:
					{
						for (int i = 0; i <= IDs; ++i)
						{
							struct sockaddr_in addr = clients[i];

							packet_T packet = {
								buffer->id,
								UPDATE,
								GAME_HAS_STARTED,
								{ 0, 0 }
							};
							net_send(network, &packet, sizeof(packet_T), addr);
						}

						break;
					}

					case SET_POSITION:
					{
						packets[packetrecv_index] = buffer;
						packetrecv_index++;
						/*
						for (int i = 0; i <= IDs; ++i)
						{
							struct sockaddr_in addr = clients[i];
							net_send(network, buffer, sizeof(packet_T), addr);
						}
						*/
						break;
					}
				}

				break;
			}

			case SET_KEY:
			{
				for (int i = 0; i <= IDs; ++i)
				{
					struct sockaddr_in addr = clients[i];

					packet_T packet = {
						buffer->id,
						SET_KEY,
						buffer->action,
						{ 0, 0 }
					};
					net_send(network, &packet, sizeof(packet_T), addr);
				}
				break;
			}

			case ERROR:
				break;

			case SUCCESS:
				break;
		}
	}

	usleep(60);
}

void *server_tick(void *arg)
{
	network_T *network = (network_T*)arg;
	struct timeval start, stop;
	double time_interval;

	while (true)
	{
    gettimeofday(&start, NULL);
		printf("SERVER :: total_packets: %d\n", packetrecv_index);
		for (int i = 0; i < packetrecv_index; ++i)
		{
			packet_T *buffer = packets[i];
			if (buffer)
			{
				for (int i = 0; i <= IDs; ++i)
				{
					struct sockaddr_in addr = clients[i];
					net_send(network, buffer, sizeof(packet_T), addr);
				}
			}
		}
		packetrecv_index = 0;
		gettimeofday(&stop, NULL);
		time_interval = (double) (stop.tv_usec - start.tv_usec);
		if ((double) (stop.tv_usec - start.tv_usec) > 0) {
				time_interval = (double) (stop.tv_usec - start.tv_usec);
		}

		if (time_interval < 0) time_interval = 4.0f;

		printf("LATENCY: %f\n", time_interval);
		usleep(time_interval);
	}
}
