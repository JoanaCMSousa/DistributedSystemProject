#ifndef _NETWORK_CLIENT_PRIVATE_H
#define _NETWORK_CLIENT_PRIVATE_H

#include "inet.h"

/*
	Projeto5
	Elaborado por:
		Nuno Carreiro 38828
		Ruben Pavao 36011
	Grupo 9
*/


struct server_t {
	int sockfd; //Socket
	struct sockaddr_in server;

};

struct message_t *reconnect(struct message_t *message);

#endif
