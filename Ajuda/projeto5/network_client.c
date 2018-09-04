#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "message.h"
#include "network_client.h"
#include "network_client-private.h"

#include "inet.h" 
#include <errno.h>

/*
	Projeto5
	Elaborado por:
		Nuno Carreiro 38828
		Ruben Pavao 36011
	Grupo 9
*/


char* addressport;
struct server_t *s;
int rec;

/* Esta funcao deve
* - estabelecer a ligacao com o servidor;
* - address_port e’ uma string no formato <hostname>:<port>
* (exemplo: 10.10.10.10:10000)
* - retornar toda a informacao necessaria (e.g., descritor do socket)
* na estrutura server_t
*/
struct server_t *network_connect(const char *address_port){
	
	//Caso seja preciso reconectar
	addressport = strdup(address_port);
	rec=0;
	
	//Aloca memoria para a estrutura s(server_t)
	s = (struct server_t*) malloc(sizeof(struct server_t));
	
	// Cria socket TCP 
	if ((s->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		perror("Erro ao criar socket TCP"); 
		free(addressport);
		network_close(s);
		return NULL; 
 	}
 	
 	// Preenche estrutura server para estabelecer conexão 
	s->server.sin_family = AF_INET; 
	
	//Separa a string address_port para as respectivas variaveis
	char *address = strtok((char*)address_port,":");
	char *port = strtok(NULL,":");
	
	s->server.sin_port = htons(atoi(port)); 
	
	if (inet_pton(AF_INET, address, &(s->server.sin_addr)) < 1) { 
		printf("Erro ao converter IP\n"); 
		network_close(s);
		return NULL; 
 	} 
 	
 	// Estabelece conexão com o servidor definido em server 
	if (connect(s->sockfd,(struct sockaddr *)&(s->server), sizeof(s->server)) < 0) { 
		perror("Erro ao conectar-se ao servidor"); 
		network_close(s);
		return NULL; 
 	}
 	
 	//Envia um inteiro ao servidor para identificar que eh um cliente
 	int imclient=htonl(1);
 	int nbytes;
	if((nbytes = write(s->sockfd,&imclient,sizeof(imclient))) != sizeof(imclient)){ 
		perror("Erro ao enviar verificacao de cliente...\n"); 
		return NULL;
 	} 
	
	//Retorna a struct do server_t
	return s;
}


/* Esta funcao deve
* - Obter o descritor da ligacao (socket) da estrutura server_t;
* - enviar a mensagem msg ao servidor;
* - receber uma resposta do servidor;
* - retornar a mensagem obtida como resposta ou NULL em caso de erro.
*/
struct message_t *network_send_receive(struct server_t *server,
struct message_t *msg){
	
	int nbytes;
	
	//Converte mensagem para um buffer de bytes
	char **msg_buf=(char **)malloc(MAX_MSG*sizeof(char));
	
	int msg_bufsize = message_to_buffer(msg,msg_buf);
	
	
	signal(SIGPIPE,SIG_IGN);
	
	//Converte tamanho do buffer para formato de rede
	// e envia para servidor
	
	int temp = htonl(msg_bufsize);
	//Envia tamanho do buffer um int
	if((nbytes = write(server->sockfd,&temp,sizeof(temp))) != sizeof(temp)){ 
		perror("Erro ao enviar tamanho do buffer ao servidor\n"); 
		free(msg_buf);
		//Se nao conseguir comunicar com servidor tenta novamente
		//Isto so acontece duas vezes, se nao conseguir devolve NULL
		return reconnect(msg);
 	} 
 	
 	printf("A enviar tamanho do buffer ao servidor ...\n");
 	
 	
 	// Vai enviar o buffer com um writeall
 	printf("A enviar buffer ao servidor ...\n");
 	
 	if(writeall(server->sockfd,*msg_buf,msg_bufsize) < 0){
 		perror("Erro ao enviar buffer...\n");
 		//Se nao conseguir comunicar com o servidor tenta novamente
 		free(msg_buf);
		return reconnect(msg);
 	}
	
	printf("A receber resposta do servidor....\n");
	
	//Recebe inteiro com o novo buffer vindo do servidor
 	
 	int nbufsiz;
 	
 	//Recebe o tamanho do buffer do servidor
	if((nbytes = read(server->sockfd,&nbufsiz,sizeof(int))) < 0){
		perror("Erro ao receber tamanho do buffer do servidor\n"); 
			
		free(msg_buf);
		
 		return reconnect(msg); 
 		
 	}
 	
 	
 	//Recebe o buffer do servidor com a nova mensagem
 	
 	nbufsiz=ntohl(nbufsiz);
 	
 	//Cria buffer com o tamanho recebido do servidor
 	char *bufferserv=(char*) malloc(nbufsiz*sizeof(char));
 	
 	//Le o buffer enviado pelo servidor
 	if(readall(server->sockfd,bufferserv,nbufsiz)!=nbufsiz){
 		perror("Erro ao ler o buffer do servidor\n");
 		
 		free(msg_buf);
 		free(bufferserv);
 		return reconnect(msg);
 	
 	}
 	
 	//Devolve a mensagem do servidor
 	return buffer_to_message(bufferserv,nbufsiz);
 
}


//Funcao que tenta reconectar e enviar a mensagem ao servidor
struct message_t *reconnect(struct message_t *message){
	printf("A tentar reconectar ao servidor....\n");
	sleep(3);
	//Incrementa o rec para saber quantas vezes ja se reconectou
	
	if( rec < 1 ){
		network_close(s);
		struct server_t *server= network_connect(addressport);
		if( (server == NULL) ) {
			printf("Nao foi possivel reconectar ao servidor \n");
			free(server);
			free_message(message);
			rec++;
			return NULL;
		}
		else{
			printf("A reenviar mensagem...\n");
			rec++;
			return network_send_receive(server,message);
		}
	}
	else{
		printf("Numero de reconeccoes maxima atingido...\n");
		free_message(message);
		network_close(s);
		return NULL;
	}

}


/* A funcao network_close() deve fechar a ligacao estabelecida por
* network_connect(). Se network_connect() alocou memoria, a funcao
* deve libertar essa memoria.
*/
int network_close(struct server_t *server){
	
	int result;
	
	printf("A fechar conexao com servidor...\n");
	result = close(server->sockfd);
	free(server);

	return result;

}
