/*Grupo 05
	Joana Sousa no47084, Joao Leal no46394,
	Liliana Delgadinho no43334*/

#ifndef _PRIMARY_BACKUP_H
#define _PRIMARY_BACKUP_H

#include "message.h"
#include "network_client-private.h"
#include "table_skel.h"

/* Função usada para um servidor avisar o servidor “server” de que
* já acordou. Retorna 0 em caso de sucesso, -1 em caso de insucesso
*/
int hello(struct server_t *server);

/* Pede atualizacao de estado ao server.
* Retorna 0 em caso de sucesso e -1 em caso de insucesso.
*/
int update_state(struct server_t *server);

#endif