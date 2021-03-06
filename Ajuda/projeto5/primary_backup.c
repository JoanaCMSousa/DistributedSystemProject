#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


#include "message.h"
#include "message-private.h"
#include "primary_backup.h"
#include "primary_backup-private.h"

#include "inet.h" 
#include <errno.h>

/*
	Projeto5
	Elaborado por:
		Nuno Carreiro 38828
		Ruben Pavao 36011
	Grupo 9
*/


/* Funcao usada para um servidor avisar o servidor server de que já acordou.
* retorna 0 em caso de sucesso, -1 em caso de insucesso
*/
int hello(struct server_t *server){

	// Cria socket TCP 
	if ((server->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		perror("Erro ao criar socket TCP"); 
		return -1; 
	}

	// Preenche estrutura server para estabelecer conexão 
	server->server.sin_family = AF_INET; 

	server->server.sin_port = htons(server->port); 
	
	if (inet_pton(AF_INET, server->address, &(server->server.sin_addr)) < 1) { 
		printf("Erro ao converter IP\n"); 
		close(server->sockfd); 
		return -1; 
	} 

	// Estabelece conexão com o servidor definido em server 
	if (connect(server->sockfd,(struct sockaddr *)&(server->server), sizeof(server->server)) < 0) { 
		perror("Erro ao conectar-se ao servidor"); 
		close(server->sockfd); 
		return -1; 
 	}
	
	
	//Envia um inteiro ao servidor para identificar que eh um servidor
 	int imserver=htonl(0);
 	int nbytes;
	if((nbytes = write(server->sockfd,&imserver,sizeof(imserver))) != sizeof(imserver)){ 
		perror("Erro ao enviar verificacao de servidor...\n"); 
		return -1;
 	} 
	
	
	return 0;
	
	//return send_pm(server->sockfd);
}


/* Pede atualizacao de estado ao server. 
* Retorna 0 em caso de sucesso e -1 em caso de insucesso.
*/
int update_state(struct server_t *server){

	int nbytes;
	int confirm = server->bu_msg->opcode;
	
	//Envia mensagem ao secundario
	char**newbuf=(char **) malloc(MAX_MSG*sizeof(char));
		
	int msg_bufsize = message_to_buffer(server->bu_msg,newbuf);
		
	//envia o tamanho do buffer ao secundario	
	int temp = htonl(msg_bufsize);
			
	if((nbytes = write(server->sockfd,&temp,sizeof(temp))) != sizeof(temp)){ 
		perror("Erro ao enviar tamanho do buffer ao servidor Secundario");
		free(newbuf);
		return -1;
 	} 
		
	//envia o buffer ao secundario
	if(writeall(server->sockfd,*newbuf,msg_bufsize) < 0){
 		perror("Erro ao enviar buffer\n");
 		//Se nao conseguir comunicar com o servidor tenta novamente
 		free(newbuf);
		return -1;
 	}
		
	printf("Mensagem enviada a Secundario....\n");
	
	int newsize;
	
	//le o tamanho do buffer enviado pelo secundario
	if((nbytes = read(server->sockfd,&newsize,sizeof(int))) < 0){
		perror("Erro ao receber tamanho do buffer do servidor\n"); 
		free(newbuf);
 		return -1; 
 		
 	}
 	
	//bufsize foi actualizado pelo read anterior
 	newsize=ntohl(newsize);
 		
 	printf("Tamanho do buffer recebido pelo secundario:%d\n",newsize);
 
 	//Cria buffer com o tamanho recebido 
 	char *buffermsg = (char*) malloc(newsize*sizeof(char));
 					
 	//Funcao auxiliar que le todos os bytes do socket
 	if(readall(server->sockfd,buffermsg,newsize)!= newsize){
 		perror("Erro ao ler buffer do secundario\n");
 		free(newbuf);
 		return -1;
 	
 	}
	
	free_message(server->bu_msg);
	
	server->bu_msg = (struct message_t*)malloc(sizeof(struct message_t));
	//converte o buffer numa mensagem
	server->bu_msg = buffer_to_message(buffermsg,msg_bufsize);
	
	//Se o opcode da mensagem recebida pelo secundario 
	//estiver correcta (opcode+1)
	//entao cpcomp = 0
	//cc cpcomp = -1
	
	printf("Mensagem recebida de Secundario com opcode a:%d\n",server->bu_msg->opcode);
	
	return (server->bu_msg->opcode == (confirm+1))? 0:-1;
}

