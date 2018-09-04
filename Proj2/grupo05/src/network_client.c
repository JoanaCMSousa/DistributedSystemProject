/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#include "network_client-private.h"

#include <stdlib.h>


int write_all(int sock, char *buf, int len){
	int buffer_size = len;

	while(len>0){
		int res = write(sock, buf, len);
		if(res<0){return res;}
			
		buf += res;
		len -= res;
	}

	return buffer_size;

}


int read_all(int sock, char *buf, int len){
	int buffer_size = len;

	while(len>0){
		int res = read(sock, buf, len);
		if(res <= 0){return res;}

		buf += res;
		len -= res;

	}

	return buffer_size;
}


struct server_t *network_connect(const char *address_port){
	struct server_t *server = malloc(sizeof(struct server_t));

	/* Verificar parâmetro da função e alocação de memória */
	if(address_port == NULL || server == NULL){return NULL;}

	/* Estabelecer ligação ao servidor:

		Preencher estrutura struct sockaddr_in com dados do
		endereço do servidor.

		Criar a socket.

		Estabelecer ligação.
	*/
	server->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(server->sockfd == -1){return NULL;}

	char *address = strtok((char *)address_port, ":");
	char *port = strtok(NULL, ":");

	server->server.sin_family = AF_INET;
	server->server.sin_port = htons(atoi(port));
	if(inet_pton(AF_INET, address, &(server->server.sin_addr)) < 1){
		close(server->sockfd);//necessario colocar?
		free(server);//necessario?
		return NULL;
	}
	
	int connct;
	connct = connect(server->sockfd, (struct sockaddr *)&server->server, sizeof(server->server));

	/* Se a ligação não foi estabelecida, retornar NULL */

	if(connct < 0){return NULL;}

	return server;
}

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg){
	char *message_out;
	int message_size, msg_size, result;  /*message_size --> host; msg_size --> network*/
	struct message_t *msg_resposta;

	/* Verificar parâmetros de entrada */
	if(server == NULL || msg == NULL){return NULL;}

	/* Verificar se a serialização teve sucesso */

	int data, nkeys, ksize, size;

	switch(msg->c_type){
	     case CT_RESULT:
	      size = 8;
	      break;
	    case CT_VALUE: 
	      data = msg->content.data->datasize;
	      size = 8 + data;
	      break;
	    case CT_KEY:
	      ksize= strlen(msg->content.key);
	      size = 6 + ksize;
	      break;
	    case CT_KEYS:
	      nkeys = 0;
	      ksize = 0;
	      while(msg->content.keys[nkeys]){
	        ksize += strlen(msg->content.keys[nkeys]);
	        nkeys++;
	      }
	      size = 8 + (nkeys*2) + ksize;
	      break;
	    case CT_ENTRY:
	      ksize = strlen(msg->content.entry->key);
	      data = msg->content.entry->value->datasize;
	      size = 10 + ksize + data;
	      break;
 	}

	

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg, &message_out);
	if(message_size != size){free(message_out); return NULL;}

	/* Enviar ao servidor o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);
 	result = write_all(server->sockfd, (char *) &msg_size, _INT);

	/* Verificar se o envio teve sucesso */
	if(result < 0){free(message_out); return NULL;}

	/* Enviar a mensagem que foi previamente serializada */
	result = write_all(server->sockfd, message_out, message_size);

	/* Verificar se o envio teve sucesso */
	if(result < 0){free(message_out); return NULL;}

	/* De seguida vamos receber a resposta do servidor:

		Com a função read_all, receber num inteiro o tamanho da 
		mensagem de resposta.

		Alocar memória para receber o número de bytes da
		mensagem de resposta.

		Com a função read_all, receber a mensagem de resposta.
		
	*/
	
	result = read_all(server->sockfd, (char *)&msg_size, _INT);
	
	if(result < 0){free(message_out); return NULL;}
	message_size = ntohl(msg_size);

	char *message_in;
	message_in = (char *) malloc(message_size);

	if(message_in == NULL){free(message_out); return NULL;}

	result = read_all(server->sockfd, message_in, message_size);


	if(result < 0){free(message_out); free(message_in); return NULL;}

	/* Desserializar a mensagem de resposta */
	msg_resposta = buffer_to_message(message_in, message_size);

	if(msg_resposta == NULL){free(message_out); free(message_in); return NULL;}

	/* Verificar se a desserialização teve sucesso */
	if(msg_resposta->opcode != (OC_SIZE + 1) && msg_resposta->opcode != (OC_DEL + 1) && 
		msg_resposta->opcode != (OC_UPDATE + 1) && msg_resposta->opcode != (OC_GET + 1)
		 && msg_resposta->opcode != (OC_PUT + 1) && msg_resposta->opcode != OC_RT_ERROR){
    		free(message_out); free(message_in); return NULL;
 	}
 	switch(msg->c_type){
 		case CT_ENTRY:
 			if((msg_resposta->opcode == (OC_PUT + 1)) && 
 				(msg_resposta->c_type != CT_RESULT)){free(message_out); free(message_in); return NULL;}
 			if((msg_resposta->opcode == (OC_UPDATE + 1)) && 
 				(msg_resposta->c_type != CT_RESULT)){free(message_out); free(message_in); return NULL;}
 			break;
	 	case CT_KEY:
	 		if(msg_resposta->opcode == (OC_GET + 1)){
	 			if(strcmp(msg->content.key, "*") == 0){
	 				if(msg_resposta->c_type != CT_KEYS){free(message_out); free(message_in); return NULL;}
	 			}
	 			else if(msg_resposta->c_type != CT_VALUE){free(message_out); free(message_in); return NULL;}
	 		}
	 		if((msg_resposta->opcode == (OC_DEL + 1)) && (msg_resposta->c_type != CT_RESULT)){
	 			free(message_out); free(message_in); return NULL;
	 		}
 			break;
 		default:
 			if((msg_resposta->opcode == (OC_SIZE + 1)) && (msg_resposta->c_type != CT_RESULT)){
	 			free(message_out); free(message_in); return NULL;
			}
 	}
 	
	/* Libertar memória */
 	free(message_out);
 	free(message_in);
 	
	return msg_resposta;
}

int network_close(struct server_t *server){
	/* Verificar parâmetros de entrada */
	
	if(server == NULL){return -1;}

	/* Terminar ligação ao servidor */

	close(server->sockfd);

	/* Libertar memória */
	
	free(server);

	return 0;
}

