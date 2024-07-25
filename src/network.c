#include "include/network.h"

network_T init_server(uint64_t port, char *ip_addr)
{
	network_T net = { 0 };
	net.addr.sin_family = AF_INET;

	if (ip_addr == NULL)
		net.addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		inet_aton(ip_addr, &net.addr.sin_addr);

	net.addr.sin_port = htons(port);

	return net;
}

network_T init_client()
{
	network_T net = { 0 };
	net.addr.sin_family = AF_INET;
	net.addr.sin_addr.s_addr = INADDR_ANY;
	net.addr.sin_port = 0;

	return net;
}

void server_bind(network_T *network)
{
	if ((network->sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0)
		perror("Server :: failed to create socket.\n\tStatus => ");

	if (bind(network->sockfd, (const struct sockaddr*)&network->addr, sizeof(struct sockaddr)))
		perror("Server :: failed to bind.\n\tStatus => ");
}

void client_bind(network_T *network)
{
	if ((network->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
		perror("Client :: failed to create socket.\n\tStatus => ");

	if (bind(network->sockfd, (const struct sockaddr*)&network->addr, sizeof(struct sockaddr)))
		perror("Client :: failed to bind.\n\tStatus => ");
}

void net_send(network_T *network, void *packet, size_t n, struct sockaddr_in addr)
{
	sendto(network->sockfd, packet, n, 0, (const struct sockaddr*)&addr, sizeof(struct sockaddr));
}

struct sockaddr_in net_recv(network_T *network, void *buffer, size_t n)
{
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(struct sockaddr);

	recvfrom(network->sockfd, buffer, n, 0, (struct sockaddr*)&addr, &addr_len);

	return addr;
}

void close_network(network_T *network)
{
	close(network->sockfd);
}
