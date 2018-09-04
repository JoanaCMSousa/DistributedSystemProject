/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "table_skel.h"
#include "table-private.h"
#include "message-private.h"

struct table_t *tabela;

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists){
	if ((tabela = table_create(n_lists)) == NULL){
		return -1;
	}
	return 0;
}

/* Libertar toda a memória e recursos alocados pela função anterior.
 */
int table_skel_destroy(){
	if(tabela == NULL){return -1;}
	table_destroy(tabela);
	return 0;
}

/* Executa uma operação (indicada pelo opcode na msg_in) e retorna o
 * resultado numa mensagem de resposta ou NULL em caso de erro.
 */
struct message_t *invoke(struct message_t *msg_in){
	struct message_t *msg_resposta;

	// Verifica se a msg_in eh valida
	if(msg_in == NULL){return NULL;}

	/* Verificar opcode e c_type na mensagem de pedido */
	if(msg_in->opcode != OC_SIZE && msg_in->opcode != OC_DEL && msg_in->opcode != OC_UPDATE 
		&& msg_in->opcode != OC_GET && msg_in->opcode != OC_PUT && msg_in->opcode != OC_HELLO
			&& msg_in->opcode != OC_SERVER){return NULL;}
	
	if(msg_in->c_type != CT_RESULT && msg_in->c_type != CT_VALUE && msg_in->c_type != CT_KEY 
		&& msg_in->c_type != CT_KEYS && msg_in->c_type != CT_ENTRY){return NULL;}

	/* Aplicar operação na tabela */
	int msg_size, opcode, c_type, result;
	struct data_t *value;
	char **keys;

	switch(msg_in->c_type){
		case CT_KEY: 
			switch(msg_in->opcode){
				case OC_DEL: 
					result = table_del(tabela, msg_in->content.key);
					if(result == -1){opcode = OC_RT_ERROR;}
					else{opcode = OC_DEL + 1;}
					c_type = CT_RESULT;
					break;
				case OC_GET:
					if(strcmp(msg_in->content.key,"*") == 0){
						keys = table_get_keys(tabela);
						if(keys	== NULL){
							opcode = OC_RT_ERROR;
							c_type = CT_RESULT;
							result = -1;
						}
						else{
							opcode = OC_GET + 1; 
							c_type = CT_KEYS;
						}
					}
					if(strcmp(msg_in->content.key,"*") != 0){
						value = table_get(tabela, msg_in->content.key);
						if(value == NULL){
							value = (struct data_t *) malloc(sizeof(struct data_t));
							if(value == NULL){return NULL;}
							value->datasize = 0;
							value->data = NULL;
						}
						opcode = OC_GET + 1;
						c_type = CT_VALUE;
					}
					break;
			}
		break;

		case CT_ENTRY:
			switch(msg_in->opcode){
				case OC_UPDATE: 
					result = table_update(tabela,msg_in->content.entry->key,
						msg_in->content.entry->value);
					if(result == -1){opcode = OC_RT_ERROR;}
					else{opcode = OC_UPDATE + 1;}
					c_type = CT_RESULT;
					break;
				case OC_PUT:
					result = table_put(tabela,msg_in->content.entry->key,
						msg_in->content.entry->value);
					if(result == -1){opcode = OC_RT_ERROR;}
					else{opcode = OC_PUT + 1;}
					c_type = CT_RESULT;
					break;
			}
		break;

		default:
			if(msg_in->opcode == OC_SIZE){
				result = table_size(tabela);
				if(result == -1){opcode = OC_RT_ERROR;}
				else{opcode = OC_SIZE + 1;}
				c_type = CT_RESULT;
			}
			else if(msg_in->opcode == OC_SERVER){
				opcode = OC_SERVER + 1;
				c_type = CT_RESULT;
				result = 0;
			}
			else if(msg_in->opcode == OC_HELLO){
				opcode = OC_HELLO + 1;
				c_type = CT_RESULT;
				result = 0;
			}
		break;
	}

	/* Preparar mensagem de resposta */
	msg_resposta = (struct message_t *) malloc(sizeof(struct message_t));
	if(msg_resposta == NULL){return NULL;}

	msg_resposta->opcode = opcode;
	msg_resposta->c_type = c_type;
	switch(c_type){
		case CT_RESULT:
			msg_resposta->content.result = result;
			break;

		case CT_VALUE: 
			msg_resposta->content.data = value;
			break;

		case CT_KEYS:
			msg_resposta->content.keys = keys;
			break;
	}

	return msg_resposta;
}
