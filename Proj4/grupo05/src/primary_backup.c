/*Grupo 05
	Joana Sousa no47084, Joao Leal no46394,
	Liliana Delgadinho no43334*/

#include "primary_backup.h"

/* Função usada para um servidor avisar o servidor “server” de que
* já acordou. Retorna 0 em caso de sucesso, -1 em caso de insucesso
*/
int hello(struct server_t *server){

	if(server == NULL){return -1;}
	struct message_t *msg_pedido;
	struct message_t *msg_resposta;
	msg_pedido = (struct message_t*) malloc(sizeof(struct message_t));

	if(msg_pedido == NULL){return -1;}

	msg_pedido->opcode = OC_HELLO;
	msg_pedido->c_type = CT_RESULT;
	msg_pedido->content.result = 0;

	msg_resposta = network_send_receive(server, msg_pedido);

	if(msg_resposta == NULL){free_message(msg_pedido); return -1;}

	free_message(msg_pedido);
	free_message(msg_resposta);
	return 0;
}

/* Pede atualizacao de estado ao server.
* Retorna 0 em caso de sucesso e -1 em caso de insucesso.
*/
int update_state(struct server_t *server){

	char** keys;
	struct message_t *msg_pedido;
	struct message_t *msg_resposta;
	msg_pedido = (struct message_t*) malloc(sizeof(struct message_t));
	
	if(msg_pedido == NULL){return -1;}


	msg_pedido->opcode = OC_GET;
	msg_pedido->c_type = CT_KEY;
	msg_pedido->content.key = "*";

	msg_resposta = network_send_receive(server, msg_pedido);
	
	keys = msg_resposta->content.keys;

	struct entry_t** entrys;
	entrys = (struct entry_t**) malloc(sizeof(struct entry_t) * (sizeof(keys)+1));
	
	if(entrys == NULL){return -1;}

	int i;
	for(i = 0; keys[i] != NULL; i++){
		msg_pedido->opcode = OC_GET;
		msg_pedido->c_type = CT_KEY;
		msg_pedido->content.key = strdup(keys[i]);
		msg_resposta = network_send_receive(server,msg_pedido);
		entrys[i] = entry_create(strdup(keys[i]),data_dup(msg_resposta->content.data));
	}
	
	for(i = 0; entrys[i] != NULL; i++){
		msg_pedido->opcode = OC_PUT;
		msg_pedido->c_type = CT_ENTRY;
		msg_pedido->content.entry = entry_dup(entrys[i]);
		msg_resposta = invoke(msg_pedido);
	}

	for(i = 0; keys[i] != NULL && entrys[i] != NULL; i++){
		free(keys[i]);
		entry_destroy(entrys[i]);
	}

	free_message(msg_pedido);
	free_message(msg_resposta);
	
	return 0;
}