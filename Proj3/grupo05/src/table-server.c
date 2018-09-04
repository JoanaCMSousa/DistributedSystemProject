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
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "inet.h"
#include "table-private.h"
#include "message-private.h"
#include "network_client-private.h"
#include "table_skel.h"

#define NFDESC 4 // Número de sockets (uma para listening)
#define TIMEOUT 50 // em milisegundos

/* Função para preparar uma socket de receção de pedidos de ligação.
*/
int make_server_socket(short port){
	int socket_fd, sim;
	struct sockaddr_in server;

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("Erro ao criar socket");
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);	
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	sim = 1;

	if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,(int*)&sim, sizeof(sim)) < 0) {
		perror("SO_REUSEADDR setsockopt error");
	}

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

/* Função "inversa" da função network_send_receive usada no table-client.
	Neste caso a função implementa um ciclo receive/send:

	Recebe um pedido;
	Aplica o pedido na tabela;
	Envia a resposta.
*/
int network_receive_send(int sockfd){
	char *message_resposta, *message_pedido;
	int msg_length;
	int message_size, msg_size, result; /*message_size --> host; msg_size --> network*/
	struct message_t *msg_pedido, *msg_resposta;
	struct list_t *results;

	/* Verificar parâmetros de entrada */
	if(sockfd <= 0){return -1;}

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
	msg_resposta = invoke(msg_pedido);

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
	int result, nfds, kfds, i, j;
	struct sockaddr_in client;
	socklen_t size_client;

	struct pollfd connections[NFDESC]; // Estrutura para file descriptors das sockets das ligações
	int sockfd; // file descriptor para a welcoming socket

	if (argc != 3){
		printf("Uso: ./server <porta TCP> <dimensão da tabela>\n");
		printf("Exemplo de uso: ./table-server 54321 10\n");
		return -1;
	}

	if ((sockfd = make_server_socket(atoi(argv[1]))) < 0){return -1;}

	table_skel_init(atoi(argv[2]));
	
	signal(SIGPIPE, SIG_IGN);

	size_client = sizeof(client);

	for (i = 0; i < NFDESC; i++)
		connections[i].fd = -1;		// poll ignora estruturas com fd < 0

	connections[0].fd = sockfd;	// Vamos detetar eventos na welcoming socket
	connections[0].events = POLLIN;	// Vamos esperar ligações nesta socket

	nfds = 1; // número de file descriptors

	while ((kfds = poll(connections, nfds, 10)) >= 0) // kfds == 0 significa timeout sem eventos
		
		if (kfds > 0){ // kfds é o número de descritores com evento ou erro

			if ((connections[0].revents & POLLIN) && (nfds < NFDESC))	// Pedido na listening socket ?
				if ((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0){ // Ligação feita ?
					printf(" * Client is connected!\n");
					connections[nfds].events = POLLIN; // Vamos esperar dados nesta socket
					nfds++;
				}
			for (j = 1; j < nfds; j++) // Todas as ligações
			
				if (connections[j].revents & POLLIN) { // Dados para ler ?
					
					printf(" * Client sent a request!\n");
					
					/* Fazer ciclo de pedido e resposta */
					result = network_receive_send(connections[j].fd);

					/* Ciclo feito com sucesso ? Houve erro?
					 Cliente desligou? */
					if(result < 0){
						printf("Ocorreu erro!\n");
						close(connections[j].fd);
						connections[j].fd = -1;
						close(sockfd);
						return -1;
					}
					if(result == 1){
						printf(" * Client is disconnected!\n");
						close(connections[j].fd);
						connections[j].fd = -1;
					}
				}

		}

	//free da table
	table_skel_destroy();

	//free connections
	for(i = 1; i < nfds; i++){
		close(connections[i].fd);
		connections[i].fd = -1;
	}

	return 0;
}
