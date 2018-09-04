#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "message.h"
#define OP_RT_ERROR 99
#define MAX_CLIENT 5

/*
	Projeto5
	Elaborado por:
		Nuno Carreiro 38828
		Ruben Pavao 36011
	Grupo 9
*/

long long swap_bytes_64(long long number);

int readall(int sockfd, char* buffer, int len);

int writeall(int sockfd, char* buffer, int len);

#endif
