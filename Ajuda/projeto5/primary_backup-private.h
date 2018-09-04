#ifndef _PRIMARY_BACKUP_PRIVATE_H
#define _PRIMARY_BACKUP_PRIVATE_H


#include "primary_backup.h"
#include "message.h"
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
	char* address;
	int port;
	struct sockaddr_in server;
	struct message_t* bu_msg;
};


#endif
