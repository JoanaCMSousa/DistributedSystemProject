#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "client_stub-private.h"
#include "client_stub.h"
#include "message.h"
#include "message-private.h"
#include "network_client.h"
#include "entry.h"
#include "data.h"
#include "inet.h"

/*
	Projeto5
	Elaborado por:
		Nuno Carreiro 38828
		Ruben Pavao 36011
	Grupo 9
*/


/* Funcao para estabelecer uma associacao com uma tabela num servidor.
* address_port e’ uma string no formato <hostname>:<port>.
* retorna NULL em caso de erro 
*/
struct rtable_t *rtable_bind(const char *address_port){
	
	struct rtable_t *result = (struct rtable_t*) malloc(sizeof(struct rtable_t));
	
	result->send=(struct message_t*)malloc(sizeof(struct message_t));
	
	result->receive=(struct message_t*)malloc(sizeof(struct message_t));
	
	
	//conecta-se ao servidor com o network-client
	result->server=network_connect(address_port);
	
	return result;
	
}

/* Fecha a ligacao com o servidor, desaloca toda a memoria local. 
* Retorna 0 se tudo correr bem e -1 em caso de erro.
*/
int rtable_unbind(struct rtable_t *rtable){

	if(rtable!=NULL){
	
		network_close(rtable->server);
		
		if(rtable->send!=NULL)
			free_message(rtable->send);
			
		if(rtable->receive!=NULL)
			free_message(rtable->receive);
		free(rtable);
		
		return 0;
	}
	
	return -1;

}



/* Funcao para adicionar um elemento na tabela.
* Se a key ja existe, vai substituir essa entrada pelos novos dados.
* Devolve 0 (ok) ou -1 (problemas).
*/
int rtable_put(struct rtable_t *rtable, char *key, struct data_t *data){
	
	struct entry_t *entry = entry_create(key,data);
	
	rtable->send-> opcode = OC_PUT;
			
	rtable->send-> c_type= CT_ENTRY;
			
	rtable->send-> content.entry = entry;
	
	//Envia e recebe mensagem
	rtable->receive = network_send_receive(rtable->server, rtable->send);
	if(rtable->receive!=NULL)
		return 0;
	else 
		return -1;
}


/* Funcao para adicionar um elemento na tabela.
* Se a key ja existe, retorna erro.
* Devolve 0 (ok) ou -1 (problemas).
*/
int rtable_conditional_put(struct rtable_t *rtable, char *key, struct data_t 
*data){
	struct entry_t *entry = entry_create(key,data);
	
	rtable->send-> opcode = OC_COND_PUT;
			
	rtable->send-> c_type = CT_ENTRY;
			
	rtable->send-> content.entry = entry;

	//Envia e recebe mensagem
	rtable->receive = network_send_receive(rtable->server, rtable->send);
	
	if(rtable->receive!=NULL)
		return 0;
	else 
		return -1;

}

/* Funcao para obter um elemento da tabela.
* Em caso de erro, devolve NULL.
*/
struct data_t *rtable_get(struct rtable_t *rtable, char *key){

	rtable->send-> opcode = OC_GET;
			
	rtable->send-> c_type = CT_KEY;
			
	rtable->send-> content.key = key;

	//Envia e recebe mensagem
	rtable->receive = network_send_receive(rtable->server, rtable->send);
	
	if(rtable->receive!=NULL)
		return rtable->receive->content.value->data;
	else 
		return NULL;
	

}


/* Funcao para remover um elemento da tabela. Vai desalocar 
* toda a memoria alocada na respectiva operacao rtable_put().
* Devolve: 0 (ok), -1 (key not found ou outro problema).
*/
int rtable_del(struct rtable_t *rtable, char *key){

	rtable->send-> opcode = OC_DEL;
	
	rtable->send-> c_type = CT_KEY;
			
	rtable->send-> content.key = key;

	//Envia e recebe mensagem
	rtable->receive = network_send_receive(rtable->server, rtable->send);
	
	if(rtable->receive!=NULL)
		return 0;
	else 
		return -1;
}

/* Devolve numero de elementos da tabela.
*/
int rtable_size(struct rtable_t *rtable){

	rtable->send-> opcode = OC_SIZE;
			
	rtable->send-> c_type = CT_RESULT;
			
	rtable->send-> content.result = 0;
	
	
	//Envia e recebe mensagem
	rtable->receive = network_send_receive(rtable->server, rtable->send);
	
	if(rtable->receive!=NULL)
		return rtable->receive->content.result;
	else 
		return -1;
}


/* Devolve um array de char* com a copia de todas as keys da tabela,
* e um ultimo elemento a NULL.
*/
char **rtable_get_keys(struct rtable_t *rtable){


	rtable->send-> opcode = OC_GET_KEYS;
			
	rtable->send-> c_type= CT_RESULT;
			
	rtable->send -> content.result=0;
	
	//Envia e recebe mensagem
	rtable->receive = network_send_receive(rtable->server, rtable->send);
	
	free_message(rtable->send);
	
	if(rtable->receive!=NULL)
		return rtable->receive->content.keys;
	else 
		return NULL;

}

/* Desaloca a memoria alocada por rtable_get_keys().
*/
void rtable_free_keys(char **keys){

	int i;

	for(i=0;keys[i];i++){

		free(keys[i]);

	}
	free(keys);
}





