/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#ifndef _NETWORK_CLIENT_PRIVATE_H
#define _NETWORK_CLIENT_PRIVATE_H

#define TRUE 1
#define FALSE 0

#include <strings.h>
#include <signal.h>
#include "inet.h"
#include "network_client.h"

struct server_t{
	int sockfd;
	struct sockaddr_in server;
	/* Atributos importantes para interagir com o servidor, */
	/* tanto antes da ligação estabelecida, como depois.    */
};

/* Função que garante o envio de len bytes armazenados em buf,
   através da socket sock.
*/
int write_all(int sock, char *buf, int len);

/* Função que garante a receção de len bytes através da socket sock,
   armazenando-os em buf.
*/
int read_all(int sock, char *buf, int len);

#endif
