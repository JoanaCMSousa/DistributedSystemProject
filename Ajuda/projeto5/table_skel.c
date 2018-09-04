#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "table_skel.h"
#include "message.h"
#include "message-private.h"
#include "table.h"
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


struct table_t *table;

/* Inicia o skeleton da tabela.
* O main() do servidor deve chamar esta funcao antes de usar a
* funcao invoke(). O parametro n_lists define o numero de listas a
* serem usadas pela tabela mantida no servidor.
* Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int table_skel_init(int n_lists){

	table = table_create(n_lists);

	if(table!=NULL)
		return 0;
	else
		return -1;
}


/* Libertar toda a memoria e recursos alocados pela funcao anterior.
*/
int table_skel_destroy(){

 	table_destroy(table);
 	
 	return 0;
	
}



/* Executa uma operacao (indicada pelo opcode na msg) e retorna o
* resultado na propria struct msg.
* Retorna 0 (OK) ou -1 (erro, por exemplo, tabela nao incializada)
*/
int invoke(struct message_t *msg){
	
	if(table==NULL){
		free_message(msg);
		
		return -1;
	}
	char *key;
	struct data_t *data;
	
	switch(msg->opcode){
			
		case OC_PUT:
				
			//Retira a key da mensagem
			key=strdup(msg->content.entry-> key);
				
			//Retira a data da mensagem
			data=data_dup(msg->content.entry->value);
			
			//Insere a data na table do servidor
			
			table_put(table,key,data);
			//Colocando o resultado na nova mensagem
			msg->content.result=table_put(table,key,data);
			//Preenche o resto da nova mensagem a enviar
			msg->opcode=OC_PUT+1;
				
			msg->c_type=CT_RESULT;
			
			break;
			
		case OC_COND_PUT:
			
			//Retira a key da mensagem
			key=strdup(msg->content.entry-> key);
				
			//Retira a data da mensagem
			data=data_dup(msg->content.entry->value);
				
			//Insere a data na table do servidor
			table_conditional_put(table,key,data);
			//Colocando o resultado na nova mensagem
			msg->content.result = table_conditional_put(table,key,data);
			//Preenche o resto da nova mensagem a enviar
			msg->opcode=OC_COND_PUT+1;
				
			msg->c_type=CT_RESULT;
		
			break;
			
		case OC_GET:
				
			//Retira a key da mensagem
			key=strdup(msg->content.key);
				
			//Procura pelo value com a key dada na mensagem
			msg->content.value= table_get(table,key);
			
			if(msg->content.value!=NULL){
				//Preenche o resto da nova mensagem a enviar
				msg->opcode=OC_GET+1;
				
				msg->c_type=CT_VALUE;
			}
			else{
			
				msg->opcode=OP_RT_ERROR;
				
				msg->c_type=OP_RT_ERROR;
			}
				
			break;
				
		case OC_DEL:
				
			//Retira a key da mensagem
			key=strdup(msg->content.key);
				
			msg->content.result=table_del(table,key);
				
			//Preenche o resto da nova mensagem a enviar
			msg->opcode=OC_DEL+1;
				
			msg->c_type=CT_RESULT;
				
			break;
				
		case OC_SIZE:
			
			msg->content.result=table_size(table);
				
			//Preenche o resto da nova mensagem a enviar
			msg->opcode=OC_SIZE+1;
				
			msg->c_type=CT_RESULT;
				
			break;
			
		case OC_GET_KEYS:
				
			msg->content.keys=table_get_keys(table);
				
			//Preenche o resto da nova mensagem a enviar
			msg->opcode=OC_GET_KEYS+1;
				
			msg->c_type=CT_KEYS;
				
			break;
				
		default: 
			msg->opcode=OP_RT_ERROR;
				
			msg->c_type=CT_RESULT;
				
			//MENSAGEM DE ERRO	
			msg->content.result=-1;
				
			break;
		//FIM DO SWITCH		
		}

	//Tudo okay!
	return 0;
}




