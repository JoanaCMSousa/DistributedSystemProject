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
#include <pthread.h>

#include "inet.h"
#include "table-private.h"
#include "message-private.h"
#include "network_client-private.h"
#include "table_skel.h"
#include "client_stub-private.h"
#include "primary_backup-private.h"

#define NFDESC 50 // Número de sockets (uma para listening e outra para o stdin)
#define TIMEOUT 50 // em milisegundos
#define TRUE 1
#define FALSE 0
#define FAIL -1


struct rtable_t *other_server;
struct message_t *command;//variavel que indica qual o comando feito pelo cliente
int backup_on;//variavel a indicar se o servidor secundario estah ativo, 1 TRUE, 0 FALSE
int primario; //indica se o servidor atual eh primario ou secundario, 1 TRUE, 0 FALSE
int new_command;//indica se existe um novo comando para o servidor secundario realiazar ou nao, 1 TRUE, 0 FALSE
char valid_command;//indica se o servidor secundario ja fez o que tinha a fazer, portanto ja pode responder ao cliente
char ** thread_args; //argumentos que a thread vai necessitar
pthread_t o_server;  //thread que trata da iteracao entre os servidores
pthread_mutex_t command_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	command_available = PTHREAD_COND_INITIALIZER;
pthread_mutex_t valid_command_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	valid_command_available = PTHREAD_COND_INITIALIZER;

void* func_secundario(){

	printf("Iniciando a thread\n");
	
	struct message_t *msg_server;  //messagem que contem o porto do servidor primario
	struct message_t *msg_ok;
	
	other_server = rtable_bind(thread_args[1]);
	if(other_server == NULL){
		backup_on = 0;
	}  
	else{
		backup_on = 1; //se houve sucesso na ligacao, entao ambos estao ativos
		printf("Primario marcou 'UP' no servidor secundario\n");
	}

	msg_server = (struct message_t *) malloc(sizeof(struct message_t));
	if(msg_server == NULL){return NULL;}  

	msg_server->opcode = OC_SERVER;
	msg_server->c_type = CT_RESULT;
	msg_server->content.result = atoi(thread_args[0]);

	if(backup_on == 1)
		msg_ok = network_send_receive(other_server->remote_table,msg_server); //resposta do secundario

	int result;

	while(1){

		pthread_mutex_lock(&command_mutex);
		

		while(new_command == 0)
			pthread_cond_wait(&command_available, &command_mutex);
		
		switch(command->opcode){
		 	case OC_PUT:
				result = rtable_put(other_server,command->content.entry->key,command->content.entry->value);
				pthread_mutex_lock(&valid_command_mutex);
				if(result == 0)
					valid_command = TRUE;
				else if(result < 0)
					valid_command = FAIL;

				pthread_cond_signal(&valid_command_available);
				pthread_mutex_unlock(&valid_command_mutex); 
				
				break;
			case OC_UPDATE:
				result = rtable_update(other_server,command->content.entry->key,command->content.entry->value);
				pthread_mutex_lock(&valid_command_mutex);
				if(result == 0)
					valid_command = TRUE;
				else if(result < 0)
					valid_command = FAIL;

				pthread_cond_signal(&valid_command_available);
				pthread_mutex_unlock(&valid_command_mutex); 
				break;
		 	case OC_DEL:
				result = rtable_del(other_server,command->content.key);
				pthread_mutex_lock(&valid_command_mutex);
				if(result == 0)
					valid_command = TRUE;
				else if(result < 0)
					valid_command = FAIL;

				pthread_cond_signal(&valid_command_available);
				pthread_mutex_unlock(&valid_command_mutex); 		
	 			break;
		}
		
		new_command = 0;

		pthread_mutex_unlock(&command_mutex);
		if (valid_command == FAIL){
			rtable_unbind(other_server);
			pthread_exit(NULL);
		}
	}

}

void printStatus(){
	char**keys;
	struct message_t *info;
	struct message_t *pedido = (struct message_t*) malloc(sizeof(struct message_t));

	if(pedido != NULL){

		printf("*********************************************\n");

		if(primario == 1)
			printf("Tipo do Servidor: Primario\n\n");
		else
			printf("Tipo do Servidor: Secundario\n\n");

		pedido->opcode = OC_GET;
		pedido->c_type = CT_KEY;
		pedido->content.key = "*";

		info = invoke(pedido);

		keys = info->content.keys;

		free(info);

		int i;
		int len;
		char* data;
		for(i = 0; keys[i]; i++){
			pedido->content.key = keys[i];
			info = invoke(pedido);
			data = (char*)info->content.data->data;
			len = info->content.data->datasize;
			data[len] = '\0';
			printf("Key %d: %s | Value: %s\n",i,keys[i],data);
			free_message(info);
		}

		printf("*********************************************\n");
	}
	rtable_free_keys(keys);
	free(pedido);
}


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

	/* Com a função read_all, receber a mensagem de resposta. */
	result = read_all(sockfd, message_pedido, message_size);

	/* Verificar se a receção teve sucesso */
	if(result < 0){free(message_pedido);free_message(msg_pedido);return -1;}
	if(result == 0){free(message_pedido);free_message(msg_pedido);return 1;}

	/* Desserializar a mensagem do pedido */
	msg_pedido = buffer_to_message(message_pedido, message_size);
	printf("Mensagem recebida do cliente\n");
	print_msg(msg_pedido);
	
	/* Verificar se a desserialização teve sucesso */
	if(msg_pedido->opcode != OC_SIZE && msg_pedido->opcode != OC_DEL && 
		msg_pedido->opcode != OC_UPDATE && msg_pedido->opcode != OC_GET
		 && msg_pedido->opcode != OC_PUT && msg_pedido->opcode != OC_HELLO 
		  && msg_pedido->opcode != OC_SERVER){
			free(message_pedido);
			free_message(msg_pedido);
			return -1;
 	}
 	switch(msg_pedido->c_type){//nesta parte preenche a variavel 'command'
 		case CT_RESULT:
 			if(msg_pedido->opcode != OC_HELLO && msg_pedido->opcode != OC_SERVER 
				&& msg_pedido->opcode != OC_SIZE){free(message_pedido);free_message(msg_pedido);return -1;}
			if(msg_pedido->opcode == OC_SIZE)
				command->opcode = OC_SIZE;
 		break;
 		case CT_ENTRY:
 			if(msg_pedido->opcode != OC_PUT && msg_pedido->opcode != OC_UPDATE){
 				free(message_pedido);
 				free_message(msg_pedido);
 				return -1;
 			}
			command->c_type = CT_ENTRY;
			if(msg_pedido->opcode == OC_PUT){
				command->opcode = OC_PUT;
				command->content.entry = entry_dup(msg_pedido->content.entry);
				}
			if(msg_pedido->opcode == OC_UPDATE){
				command->opcode = OC_UPDATE;
				command->content.entry = entry_dup(msg_pedido->content.entry);
				}
 			break;
	 	case CT_KEY:
	 		if(msg_pedido->opcode != OC_GET && msg_pedido->opcode != OC_DEL){
	 			free(message_pedido);
	 			free_message(msg_pedido);
	 			return -1;
	 		}
			command->c_type = CT_KEY;
			if(msg_pedido->opcode == OC_DEL){
				command->opcode = OC_DEL;
				command->content.key = strdup(msg_pedido->content.key);
				}
			if(msg_pedido->opcode == OC_GET)
				command->opcode = OC_GET;
 			break;
			
 	}
	
	if(primario == 1 && backup_on == 1)
		if(command->opcode != OC_SIZE && command->opcode != OC_GET){
			pthread_mutex_lock(&command_mutex);
			new_command = 1; //novo comando a fazer
			pthread_cond_signal(&command_available); 
			pthread_mutex_unlock(&command_mutex);
		}

	if(msg_pedido->opcode == OC_SERVER){
		struct sockaddr_in sockaddr;
		socklen_t len = sizeof(struct sockaddr_in);
		int teste;
		if((teste = getpeername(sockfd,(struct sockaddr *)&sockaddr,&len)) != 0){return -1;}
		FILE *fp = fopen("a.txt","w");  //servidor secundario escreve neste ficheiro o seu porto e os dados do servidor primario
		char string[1024];
		bzero((void *)string, 1024);
		inet_ntop(AF_INET,(struct in_addr *)(&(sockaddr.sin_addr)),string,1024);
		fprintf(fp,"%d\n",atoi(thread_args[0]));
		fprintf(fp,"%s:",string);
		fprintf(fp,"%d",msg_pedido->content.result);
		fclose(fp);
	} 
	

	msg_resposta = invoke(msg_pedido);
	if(msg_resposta == NULL){
		free(message_pedido);
		free_message(msg_pedido);
		return -1;
	}

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg_resposta, &message_resposta);
	
	printf("Mensagem enviada para o cliente\n");
	print_msg(msg_resposta);

	/* Verificar se a serialização teve sucesso */
	int data, nkeys, ksize;

	switch(msg_resposta->c_type){
		case CT_RESULT:
		if(message_size != 8){
			free(message_pedido);
			free_message(msg_pedido);
			free_message(msg_resposta);
			free(message_resposta);
			return -1;
		}
		break;
		case CT_VALUE: 
		data = msg_resposta->content.data->datasize;
		if(message_size != (8 + data)){
			free(message_pedido);
			free_message(msg_pedido);
			free_message(msg_resposta);
			free(message_resposta);
			return -1;
		}
		break;
		case CT_KEY:
		ksize= strlen(msg_resposta->content.key);
		if(message_size != (6 + ksize)){
			free(message_pedido);
			free_message(msg_pedido);
			free_message(msg_resposta);
			free(message_resposta);
			return -1;
		}
		break;
		case CT_KEYS:
		nkeys = 0;
		ksize = 0;
		while(msg_resposta->content.keys[nkeys]){
			ksize += strlen(msg_resposta->content.keys[nkeys]);
			nkeys++;
		} 
		if(message_size != (8 + (nkeys*2) + ksize)){
			free(message_pedido);
			free_message(msg_pedido);
			free_message(msg_resposta);
			free(message_resposta);
			return -1;
		}
		break;
		case CT_ENTRY:
		ksize = strlen(msg_resposta->content.entry->key);
		data = msg_resposta->content.entry->value->datasize;
		if(message_size != (10 + ksize + data)){
			free(message_pedido);
			free_message(msg_pedido);
			free_message(msg_resposta);
			free(message_resposta);
			return -1;
		}
		break;
 	}
	

	if(primario == 1 && backup_on == 1)
		if(command->opcode != OC_SIZE && command->opcode != OC_GET){
			pthread_mutex_lock(&valid_command_mutex);
			while(valid_command == FALSE)
				pthread_cond_wait(&valid_command_available, &valid_command_mutex);  //quando o secundario terminar de
													//atualizar a sua tabela
			if(valid_command == FAIL){
				backup_on = 0;  //backup_on = FALSE
				printf("Primario marcou 'DOWN' no servidor secundario\n");
			}
			valid_command = FALSE;
			pthread_mutex_unlock(&valid_command_mutex);
		}

	if(msg_pedido->opcode == OC_HELLO){
		backup_on = 1;
		if (pthread_create(&o_server,NULL,func_secundario,NULL) != 0){
			perror("\nThread não criada.\n");
			exit(EXIT_FAILURE);
		}
	}
	

	/* Enviar ao cliente o tamanho da mensagem que será enviada
	 logo de seguida
	*/
	msg_size = htonl(message_size);
	result = write_all(sockfd, (char *) &msg_size, _INT);

	/* Verificar se o envio teve sucesso */
	if(result < 0){
		free(message_pedido);
		free_message(msg_pedido);
		free_message(msg_resposta);
		free(message_resposta);
		return -1;
	}

	/* Enviar a mensagem que foi previamente serializada */
	result = write_all(sockfd, message_resposta, message_size);

	/* Verificar se o envio teve sucesso */
	if(result < 0){
		free(message_pedido);
		free_message(msg_pedido);
		free_message(msg_resposta);
		free(message_resposta);
		return -1;
	}

	/* Libertar memória */
	
	free(message_pedido);
	free_message(msg_pedido);
	free_message(msg_resposta);
	free(message_resposta);

	return 0;
}



int main(int argc, char **argv){
	int result, nfds, kfds, i, j;
	struct sockaddr_in client;
	socklen_t size_client;
	char address_port[1024];

	struct pollfd connections[NFDESC]; // Estrutura para file descriptors das sockets das ligações
	int sockfd; // file descriptor para a welcoming socket

	if (argc > 4){
		printf("Uso: ./server <porta TCP_primario> <dimensão da tabela> <ip secundario>:<porta TCP_secundario> ou\n");
		printf("./server <porta TCP> <dimensão da tabela>\n");
		printf("Exemplo de uso: ./table-server 54321 10 10.101.148.144:12345 ou\n");
		printf("Exemplo de uso: ./table-server 12345 10\n");
		return -1;
	}

	if(argc == 3)
		primario = 0;  //se recebeu 3 argumentos, eh secundario
	else
		primario = 1;  //caso contrario eh primario

	command = (struct message_t *) malloc(sizeof(struct message_t));
	if(command == NULL){return -1;}

	valid_command = FALSE;
	int can_create_thread = 1;  //indica se pode criar a thread no main ou nao, 1 TRUE, 0 FALSE

	if ((sockfd = make_server_socket(atoi(argv[1]))) < 0){return -1;}

	table_skel_init(atoi(argv[2]));
	
	signal(SIGPIPE, SIG_IGN);

	size_client = sizeof(client);

	if(primario == 1){
		struct sockaddr_in sockaddr;
		socklen_t len = sizeof(struct sockaddr_in);
		FILE *fp = fopen("b.txt","w");  //o servidor primario escreve neste ficheiro o seu porto e os dados do servidor secundario
		fprintf(fp,"%d\n",atoi(argv[1]));
		fprintf(fp,"%s",argv[3]);
		fclose(fp);

	}
	
	if(primario == 0){
		char string[1024];  //guarda os dados que o servidor necessita de ler
		bzero((void *)string, 1024);  //coloca tudo a zero '\0'
		FILE *fp = fopen("a.txt","r");
		int porto;  //o porto que estah colocado no ficheiro indica qual servidor deve ler aquele ficheiro
		if(fp != NULL){  //se existe ficheiro e conseguiu abirir
			fscanf(fp,"%d",&porto);
			if(porto != atoi(argv[1])){  //se o porto que estah escrito eh diferente ao do servidor, abrir o outro ficheiro
				fclose(fp);
				FILE *fp = fopen("b.txt","r");
				if(fp != NULL){
					fscanf(fp,"%*d");
					fscanf(fp,"%s",string);
					fclose(fp);
				}
			}
			else{  //caso contrario, le os dados que necessita
				fscanf(fp,"%s",string);
				fclose(fp);
			}
		}
		else{  //caso contario, tenta abrir o outro ficheiro
			FILE *fp = fopen("b.txt","r");
			if(fp != NULL){  //se existe o ficheiro e conseguiu abrir
				fscanf(fp,"%*d");
				fscanf(fp,"%s",string);
				fclose(fp);
			}
		}

		if( strlen(string) != 0){  //se a string nao estah 'vazia'
			other_server = rtable_bind(string);
			if(other_server == NULL){return -1;}
			hello(other_server->remote_table);
			update_state(other_server->remote_table);
			rtable_unbind(other_server);
		}  //caso estiver 'vazia', significa que arrancou pela primeira vez

	}

	for (i = 0; i < NFDESC; i++)
		connections[i].fd = -1;		// poll ignora estruturas com fd < 0

	connections[0].fd = sockfd;	// Vamos detetar eventos na welcoming socket
	connections[0].events = POLLIN;	// Vamos esperar ligações nesta socket

	// socket listening stdin
	connections[1].fd = fileno(stdin);
	connections[1].events = POLLIN;

	nfds = 2; // número de file descriptors
	
	thread_args = (char **) malloc(sizeof(char *) * 2);
	if(thread_args == NULL){return -1;}

	if(can_create_thread == 1){
		thread_args[0] = argv[1];
		thread_args[1] = argv[3];
	}

	while ((kfds = poll(connections, nfds, 10)) >= 0) // kfds == 0 significa timeout sem eventos
		
		if (kfds > 0){ // kfds é o número de descritores com evento ou erro

			for (j = 2; j < nfds; j++) // Todas as ligações exceto a 0 e a 1 que estao reservadas
			
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

			if ((connections[0].revents & POLLIN) && (nfds < NFDESC))	// Pedido na listening socket ?
				if ((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0){ // Ligação feita ?
					printf(" * Client is connected!\n");
					
					if(primario == 1 && can_create_thread == 1){

						if (pthread_create(&o_server,NULL,func_secundario,NULL) != 0){
							perror("\nThread não criada.\n");
							exit(EXIT_FAILURE);
						} 
						can_create_thread = 0;
					}
					

					connections[nfds].events = POLLIN; // Vamos esperar dados nesta socket
					nfds++;

					if(primario == 0 && nfds > 3){
						primario = 1;
						backup_on = 0;
						FILE *fp = fopen("a.txt","r");
						int porto;
						bzero((void*)address_port,1024);
						if(fp != NULL){
							fscanf(fp,"%d",&porto);
							if(porto != atoi(argv[1])){
								fclose(fp);
								FILE *fp = fopen("b.txt","r");
								fscanf(fp,"%*d");
							}
						}
						else{
							FILE *fp = fopen("b.txt","r");
							fscanf(fp,"%*d");
						}
						fscanf(fp,"%s",address_port);
						fclose(fp);
						thread_args[1] = address_port;
						can_create_thread = 0;
					}
				}
				
				

			if(connections[1].revents & POLLIN){ // Pedido na stdin
				char *option;
				option = (char *) malloc(sizeof(char) * MAX_MSG);
				bzero(option, MAX_MSG);

				if(fgets(option, MAX_MSG-1, stdin) == NULL){return -1;}
				option[strlen(option) - 1] = '\0';
				
				if(strcmp(option, "quit") == 0){
					free(option);
					free_message(command);
					table_skel_destroy();
					free(thread_args);
					return 0;
				}

				if(strcmp(option, "print") == 0){
					printStatus();
				}

				free(option);
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
