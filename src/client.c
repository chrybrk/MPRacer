#include "include/client.h"
#include "include/global.h"

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
						for (int i = 0; i < MaxPlayer; ++i)
						{
							players[i] = (Rectangle){ start_pos.x, start_pos.y + (i * 60), 50, 50 };
						}

						HasStarted = true;
						break;
					}
					
					case SET_POSITION:
					{
						if (buffer->id != ID)
						{
							players[buffer->id].x = buffer->position[0];
							players[buffer->id].y = buffer->position[1];
						}

						break;
					}
				}

				break;
			}

			case SET_KEY:
			{
				switch (buffer->action)
				{
					case KEY_W: players[buffer->id].y += 10; break;
					case KEY_S: players[buffer->id].y -= 10; break;
				}

				break;
			}
		}
	}
}
