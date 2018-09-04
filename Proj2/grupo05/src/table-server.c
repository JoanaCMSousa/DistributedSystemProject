/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

/*
	Programa que implementa um servidor de uma tabela hash com chainning.
	Uso: table-server <porta TCP> <dimensão da tabela>
	Exemplo de uso: ./table_server 54321 10
*/
#include <error.h>
#include <signal.h>

#include "inet.h"
#include "table-private.h"
#include "message-private.h"
#include "network_client-private.h"


/* Função para preparar uma socket de receção de pedidos de ligação.
*/
int make_server_socket(short port){
	int socket_fd;
	struct sockaddr_in server;

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
	perror("Erro ao criar socket");
	return -1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);	
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("Erro ao fazer bind");
		close(socket_fd);
		return -1;
	}

	if (listen(socket_fd, 0) < 0){
		perror("Erro ao executar listen");
		close(socket_fd);
		return -1;
	}
	return socket_fd;
}


/* Função que recebe uma tabela e uma mensagem de pedido e:
	- aplica a operação na mensagem de pedido na tabela;
	- devolve uma mensagem de resposta com o resultado.
*/
struct message_t *process_message(struct message_t *msg_pedido, struct table_t *tabela){
	struct message_t *msg_resposta;
	
	/* Verificar parâmetros de entrada */
	if(msg_pedido == NULL || tabela == NULL){return NULL;}

	/* Verificar opcode e c_type na mensagem de pedido */
	if(msg_pedido->opcode != OC_SIZE && msg_pedido->opcode != OC_DEL && msg_pedido->opcode != OC_UPDATE 
		&& msg_pedido->opcode != OC_GET && msg_pedido->opcode != OC_PUT){return NULL;}
	
	if(msg_pedido->c_type != CT_RESULT && msg_pedido->c_type != CT_VALUE && msg_pedido->c_type != CT_KEY 
		&& msg_pedido->c_type != CT_KEYS && msg_pedido->c_type != CT_ENTRY){return NULL;}

	/* Aplicar operação na tabela */
	int msg_size, opcode, c_type, result;
	struct data_t *value;
	char **keys;

	switch(msg_pedido->c_type){
		case CT_KEY: 
			switch(msg_pedido->opcode){
				case OC_DEL: 
					result = table_del(tabela, msg_pedido->content.key);
					if(result == -1){opcode = OC_RT_ERROR;}
					else{opcode = OC_DEL + 1;}
					c_type = CT_RESULT;
					break;
				case OC_GET:
					if(strcmp(msg_pedido->content.key,"*") == 0){
						keys = table_get_keys(tabela);
						if(keys  == NULL){
							opcode = OC_RT_ERROR;
							c_type = CT_RESULT;
							result = -1;
						}
						else{
							opcode = OC_GET + 1; 
							c_type = CT_KEYS;
						}
					}
					if(strcmp(msg_pedido->content.key,"*") != 0){
						value = table_get(tabela, msg_pedido->content.key);
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
			switch(msg_pedido->opcode){
				case OC_UPDATE: 
					result = table_update(tabela,msg_pedido->content.entry->key,
						msg_pedido->content.entry->value);
					if(result == -1){opcode = OC_RT_ERROR;}
					else{opcode = OC_UPDATE + 1;}
					c_type = CT_RESULT;
					break;
				case OC_PUT:
					result = table_put(tabela,msg_pedido->content.entry->key,
						msg_pedido->content.entry->value);
					if(result == -1){opcode = OC_RT_ERROR;}
					else{opcode = OC_PUT + 1;}
					c_type = CT_RESULT;
					break;
			}
		break;

		default:
			if(msg_pedido->opcode == OC_SIZE){
				result = table_size(tabela);
				if(result == -1){opcode = OC_RT_ERROR;}
				else{opcode = OC_SIZE + 1;}
				c_type = CT_RESULT;
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


/* Função "inversa" da função network_send_receive usada no table-client.
	Neste caso a função implementa um ciclo receive/send:

	Recebe um pedido;
	Aplica o pedido na tabela;
	Envia a resposta.
*/
int network_receive_send(int sockfd, struct table_t *table){
	char *message_resposta, *message_pedido;
	int msg_length;
	int message_size, msg_size, result; /*message_size --> host; msg_size --> network*/
	struct message_t *msg_pedido, *msg_resposta;
	struct list_t *results;

	/* Verificar parâmetros de entrada */
	if(sockfd <= 0 || table == NULL){return -1;}

	/* Com a função read_all, receber num inteiro o tamanho da 
	 mensagem de pedido que será recebida de seguida.*/
	result = read_all(sockfd, (char *) &msg_size, _INT);
	message_size = ntohl(msg_size);
	
	/* Verificar se a receção teve sucesso */
	if(result < 0){return -1;}
	if(result == 0){return 1;}

	/* Alocar memória para receber o número de bytes da
	 mensagem de pedido. */
	message_pedido = (char *) malloc(message_size * sizeof(char));

	if(message_pedido == NULL){return -1;}

	msg_pedido = (struct message_t *) malloc(sizeof(struct message_t));

	if(msg_pedido == NULL){free(message_pedido); return -1;}

	/* Com a função read_all, receber a mensagem de resposta. */
	result = read_all(sockfd, message_pedido, message_size);

	/* Verificar se a receção teve sucesso */
	if(result < 0){return -1;}
	if(result == 0){return 1;}

	/* Desserializar a mensagem do pedido */
	msg_pedido = buffer_to_message(message_pedido, message_size);
	printf("Mensagem recebida do cliente\n");
	print_msg(msg_pedido);

	/* Verificar se a desserialização teve sucesso */
	if(msg_pedido->opcode != OC_SIZE && msg_pedido->opcode != OC_DEL && 
		msg_pedido->opcode != OC_UPDATE && msg_pedido->opcode != OC_GET
		 && msg_pedido->opcode != OC_PUT){
    	return -1;
 	}
 	switch(msg_pedido->c_type){
 		case CT_ENTRY:
 			if(msg_pedido->opcode != OC_PUT && msg_pedido->opcode != OC_UPDATE){return -1;}
 			break;
	 	case CT_KEY:
	 		if(msg_pedido->opcode != OC_GET && msg_pedido->opcode != OC_DEL){return -1;}
 			break;
 		default:
 			if(msg_pedido->opcode != OC_SIZE){return -1;}
 	}

	/* Processar a mensagem */
	msg_resposta = process_message(msg_pedido, table);

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg_resposta, &message_resposta);
	
	printf("Mensagem enviada para o cliente\n");
	print_msg(msg_resposta);

	/* Verificar se a serialização teve sucesso */
	int data, nkeys, ksize;

	switch(msg_resposta->c_type){
		case CT_RESULT:
		  if(message_size != 8){return -1;}
	      break;
	    case CT_VALUE: 
	      data = msg_resposta->content.data->datasize;
	      if(message_size != (8 + data)){return -1;}
	      break;
	    case CT_KEY:
	      ksize= strlen(msg_resposta->content.key);
	      if(message_size != (6 + ksize)){return -1;}
	      break;
	    case CT_KEYS:
	      nkeys = 0;
	      ksize = 0;
	      while(msg_resposta->content.keys[nkeys]){
	        ksize += strlen(msg_resposta->content.keys[nkeys]);
	        nkeys++;
	      } 
	      if(message_size != (8 + (nkeys*2) + ksize)){return -1;}
	      break;
	    case CT_ENTRY:
	      ksize = strlen(msg_resposta->content.entry->key);
	      data = msg_resposta->content.entry->value->datasize;
	      if(message_size != (10 + ksize + data)){return -1;}
	      break;
 	}
	/* Enviar ao cliente o tamanho da mensagem que será enviada
	 logo de seguida
	*/
	msg_size = htonl(message_size);
	result = write_all(sockfd, (char *) &msg_size, _INT);

	/* Verificar se o envio teve sucesso */
	if(result < 0){return -1;}

	/* Enviar a mensagem que foi previamente serializada */
	result = write_all(sockfd, message_resposta, message_size);

	/* Verificar se o envio teve sucesso */
	if(result < 0){return -1;}

	/* Libertar memória */
	free(message_resposta);
	free(message_pedido);
	free_message(msg_pedido);

	return 0;
}



int main(int argc, char **argv){
	int listening_socket, connsock, result;
	struct sockaddr_in client;
	socklen_t size_client;
	struct table_t *table;

	if (argc != 3){
		printf("Uso: ./server <porta TCP> <dimensão da tabela>\n");
		printf("Exemplo de uso: ./table-server 54321 10\n");
	return -1;
	}

	if ((listening_socket = make_server_socket(atoi(argv[1]))) < 0){return -1;}

	if ((table = table_create(atoi(argv[2]))) == NULL){
		result = close(listening_socket);
		return -1;
	}

	signal(SIGPIPE, SIG_IGN);

	size_client = sizeof(client);
	while ((connsock = accept(listening_socket, (struct sockaddr *) &client, &size_client)) != -1) {
		printf(" * Client is connected!\n");
		
		int x = 0;

		while (x == 0){

			/* Fazer ciclo de pedido e resposta */
			result = network_receive_send(connsock, table);

			/* Ciclo feito com sucesso ? Houve erro?
			 Cliente desligou? */
			if(result < 0){
				printf("Ocorreu erro!\n");
				close(connsock);
				close(listening_socket);
				return -1;
			}
			if(result == 1){
				printf(" * Client is disconnected!\n");
				x = -1;
			}
			/*if(result == 0){
				x = -1;
			}*/ 

		}
		close(connsock);
	}

	close(listening_socket);
	return 0;
}
