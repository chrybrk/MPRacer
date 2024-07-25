#include "include/server.h"
#include "include/global.h"

#define MAX_CLIENTS 4

int IDs = -1;
struct sockaddr_in clients[4];
unsigned int client_index = 0;

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
						for (int i = 0; i <= IDs; ++i)
						{
							struct sockaddr_in addr = clients[i];
							net_send(network, buffer, sizeof(packet_T), addr);
						}
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
}
