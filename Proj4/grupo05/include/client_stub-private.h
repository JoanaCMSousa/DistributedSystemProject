/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include "network_client-private.h"

/* Remote table. A definir pelo grupo em client_stub-private.h 
 */
struct rtable_t{
	struct server_t *remote_table;

};

void print_messages(struct message_t *msg_out, struct message_t *msg_resposta);

#endif
