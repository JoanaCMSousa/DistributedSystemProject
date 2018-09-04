#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include "message.h"
#include "message-private.h"
#include "network_client.h"

/*
	Projeto5
	Elaborado por:
		Nuno Carreiro 38828
		Ruben Pavao 36011
	Grupo 9
*/


struct rtable_t {
	struct server_t *server; //sockfd sockaddr_in
	
	struct message_t *send;
	struct message_t *receive;
};

#endif
