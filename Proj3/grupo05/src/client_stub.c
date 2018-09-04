/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "client_stub-private.h"

/* Função para estabelecer uma associação entre o cliente e uma tabela
 * remota num servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtable_t *rtable_bind(const char *address_port){
	struct rtable_t *result;
	result = (struct rtable_t *) malloc(sizeof(struct rtable_t));
	result->remote_table = network_connect(address_port);
	return result;
}

/* Termina a associação entre o cliente e a tabela remota, e liberta
 * toda a memória local. 
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtable_unbind(struct rtable_t *rtable){
	int result;
	result = network_close(rtable->remote_table);
	free(rtable);
	return result;
}

/* Função para adicionar um par chave valor na tabela remota.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtable_put(struct rtable_t *rtable, char *key, struct data_t *value){

	struct message_t *msg_out, *msg_resposta;
	msg_out = (struct message_t*) malloc(sizeof(struct message_t));

	if(msg_out == NULL){return -1;}

	struct entry_t *entry;
	msg_out->opcode = OC_PUT;
	msg_out->c_type = CT_ENTRY;
	entry = entry_create(key, value);
	msg_out->content.entry = entry;

	msg_resposta = network_send_receive(rtable->remote_table, msg_out);

	if(msg_resposta == NULL){free_message(msg_out); return -1;}

	print_messages(msg_out,msg_resposta);
	free_message(msg_out);
	free_message(msg_resposta);
	return 0;
}

/* Função para substituir na tabela remota, o valor associado à chave key.
 * Devolve 0 (OK) ou -1 em caso de erros.
 */
int rtable_update(struct rtable_t *rtable, char *key, struct data_t *value){
	
	struct message_t *msg_out, *msg_resposta;
	msg_out = (struct message_t*) malloc(sizeof(struct message_t));

	if(msg_out == NULL){return -1;}	

	struct entry_t *entry;
	msg_out->opcode = OC_UPDATE;
	msg_out->c_type = CT_ENTRY;
	entry = entry_create(key, value);
	msg_out->content.entry = entry;

	msg_resposta = network_send_receive(rtable->remote_table, msg_out);

	if(msg_resposta == NULL){free_message(msg_out); return -1;}

	print_messages(msg_out,msg_resposta);
	free_message(msg_out);
	free_message(msg_resposta);
	return 0;
}

/* Função para obter da tabela remota o valor associado à chave key.
 * Devolve NULL em caso de erro.
 */
struct data_t *rtable_get(struct rtable_t *table, char *key){

	struct message_t *msg_out, *msg_resposta;
	struct data_t *data;
	msg_out = (struct message_t*) malloc(sizeof(struct message_t));

	if(msg_out == NULL){return NULL;}	

	msg_out->opcode = OC_GET;
	msg_out->c_type = CT_KEY;
	msg_out->content.key = strdup(key);
	
	msg_resposta = network_send_receive(table->remote_table, msg_out);
	
	if(msg_resposta == NULL){free_message(msg_out); return NULL;}
	data = data_dup(msg_resposta->content.data);
	
	print_messages(msg_out,msg_resposta);
	free_message(msg_out);
	free_message(msg_resposta);
	return data;
}

/* Função para remover um par chave valor da tabela remota, especificado 
 * pela chave key.
 * Devolve: 0 (OK) ou -1 em caso de erros.
 */
int rtable_del(struct rtable_t *table, char *key){

	struct message_t *msg_out, *msg_resposta;
	msg_out = (struct message_t*) malloc(sizeof(struct message_t));

	if(msg_out == NULL){return -1;}		

	msg_out->opcode = OC_DEL;
	msg_out->c_type = CT_KEY;
	msg_out->content.key = strdup(key);
	
	msg_resposta = network_send_receive(table->remote_table, msg_out);

	if(msg_resposta == NULL){free_message(msg_out); return -1;}
	
	print_messages(msg_out,msg_resposta);
	free_message(msg_out);
	free_message(msg_resposta);
	return 0;
}

/* Devolve número de pares chave/valor na tabela remota.
 */
int rtable_size(struct rtable_t *rtable){
	
	struct message_t *msg_out, *msg_resposta;
	msg_out = (struct message_t*) malloc(sizeof(struct message_t));

	if(msg_out == NULL){return -1;}		

	msg_out->opcode = OC_SIZE;
	msg_out->c_type = CT_RESULT;
	msg_out->content.result = 0;
	
	msg_resposta = network_send_receive(rtable->remote_table, msg_out);

	if(msg_resposta == NULL){free_message(msg_out); return -1;}
	int count = msg_resposta->content.result;

	print_messages(msg_out,msg_resposta);
	free_message(msg_out);
	free_message(msg_resposta);
	return count;
}

/* Devolve um array de char * com a cópia de todas as keys da
 * tabela remota, e um último elemento a NULL.
 */
char **rtable_get_keys(struct rtable_t *rtable){
	
	struct message_t *msg_out, *msg_resposta;
	msg_out = (struct message_t*) malloc(sizeof(struct message_t));
	char** keys;

	if(msg_out == NULL){return NULL;}	

	msg_out->opcode = OC_GET;
	msg_out->c_type = CT_KEY;
	msg_out->content.key = "*";

	msg_resposta = network_send_receive(rtable->remote_table, msg_out);

	if(msg_resposta == NULL){free_message(msg_out); return NULL;}

	keys = msg_resposta->content.keys;

	print_messages(msg_out,msg_resposta);
	free(msg_out);
	free(msg_resposta);
	return keys;
}

/* Liberta a memória alocada por table_get_keys().
 */
void rtable_free_keys(char **keys){
	table_free_keys(keys);
}

void print_messages(struct message_t *msg_out, struct message_t *msg_resposta){
	printf("Mensagem enviada para o servidor\n");
	print_msg(msg_out);
	printf("Mensagem recebida do servidor\n");
	print_msg(msg_resposta);
}
