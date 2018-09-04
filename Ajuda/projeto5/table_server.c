#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>


#include "primary_backup.h"
#include "primary_backup-private.h"
#include "message.h"
#include "message-private.h"
#include "table_skel.h"
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


//Verifica se o numero de argumentos eh valido
int testInput(int argc) 
{ 
if (argc != 3 && argc !=4){ 
	printf("Uso:./table_server <n_lists> <port> [<port_backup>] \n");
	printf("Exemplo de uso (sem replicacao): ./table_server 5 12345\n");
	printf("Exemplo de uso (com replicacao): ./table_server 5 12346 127.0.0.1:12345\n");
	return -1; 
} 

return 0; 
}


struct server_t *serv1;
int *bu_ok;
int nbytes;
int sockfd;
int backup;
int primario;

//Funcao de controlo de sinal
void interrupt_handler(int sig){
    nbytes= -1;
    table_skel_destroy();
    close(sockfd);
    
    free(bu_ok);
    
    if(serv1!=NULL){
    	
    	if(serv1->bu_msg !=NULL)
    		free_message(serv1->bu_msg);
    		
    	close(serv1->sockfd);
    	
    }
    
    printf("Sinal de interrupcao detectado, a fechar o servidor\n");
    if(backup==1)
    	backup=0;
    exit(sig);
}

//Funçao chamada pelo pthread
void* update (){
	int error = 0;
	socklen_t len = sizeof (error);
	int retval = getsockopt (serv1->sockfd, SOL_SOCKET, SO_ERROR, &error, &len );
	if(retval==0)
		*bu_ok=update_state(serv1);
	else
		*bu_ok=-1;
	pthread_exit((void*)bu_ok);
}

//Funcao que imprime o estado do servidor
//Conteudo da table + tipo de servidor
void* printStatus() {

	char* operation;
	int i;
	char**keys;
	char buffer[1024];
	
	struct message_t *info = (struct message_t*)malloc(sizeof(struct message_t));
	
	while(1){
	
		fgets(buffer,MAX_MSG,stdin);
		
		operation = (char *) malloc( strlen(buffer)-1*sizeof(char));
			
		memcpy(operation,buffer+0,strlen(buffer)-1);
		
		//Se o comando lido no terminal for um print
		if(strstr(operation,"print")!=NULL){
		
			if(primario==1)
				printf("Tipo de Servidor: Primario\n");
			else
				printf("Tipo de Servidor: Secundario\n");
			
			info->opcode = OC_GET_KEYS;
			
			info->c_type= CT_RESULT;
			
			invoke(info);
			
			keys=info->content.keys;
			
			for(i=0;keys[i];i++){
				
				
				info-> opcode = OC_GET;
				
				info->c_type=CT_VALUE;
				
				info->content.key=keys[i];
					
				invoke(info);
				
				printf("Key%d: %s | Value: %s\n",i,keys[i],(char*)info->content.value->data);
			
			}
		//Fim do if==print
		}
		
	free(operation);
	//Fim do While
	}
	

}



int main(int argc, char **argv) {
	
	backup=0;
	
	//VAriavel que define se o servidor eh primario ou nao
	primario=0;
	
	int callnot=0;
	bu_ok = (int*) malloc(sizeof(int));
	
	//Confirma se tem o numero de argumentos certos
	if(testInput(argc) <0) return -1;
	
	//Cria tabela do servidor
	//com o numero de entradas que a table terah
	table_skel_init(atoi(argv[1]));
	
	
	//Thread destinada a dar trabalho ao servidor secundario
	pthread_t psec;
	
	//Thread destinada a receber pedidos do teclado
	pthread_t command;
	
	pthread_create(&command,NULL,&printStatus,NULL);
						
	
	serv1 = (struct server_t*) malloc(sizeof(struct server_t));

	int hi;

	//Forma de o primario saber se o secundario ja estah ligado
	serv1->sockfd=-1;
	if(argc == 4){
		
		//Separa a string argv[3] para as respectivas variaveis
		serv1->address = strtok((char*)argv[3],":");
		serv1->port = atoi(strtok(NULL,":"));
		
		hi=hello(serv1);
		if(hi==-1){
			close(serv1->sockfd);
			serv1->sockfd=-1;
		}else{
		 	int imdunno=htonl(0);
		 	int servtype;
			if((nbytes = write(serv1->sockfd,&imdunno,sizeof(imdunno))) != sizeof(imdunno)){ 
				perror("Erro ao enviar verificacao de servidor...\n"); 
				return -1;
		 	}
		 	
		 	if((nbytes = read(serv1->sockfd,&servtype,sizeof(servtype)) != sizeof(servtype))){
		 		perror("Error a receber verificacao do servidor...\n");
		 		return -1;
		 	}
		 	
		 	if(servtype==0)
		 		primario=1;
		 	else{ /* Receber tabela do primário */
		 		int sizetable;
		 		
		 		read(serv1->sockfd,&sizetable,sizeof(sizetable));
		 		//recebe o numero de buckets
		 		sizetable = ntohl(sizetable);
		 		//se o numero de buckets for diferente
		 		if(sizetable!=atoi(argv[1])){
		 			printf("WARNING: Buckets do servidor secundario diferente do primario a desligar...\n");
		 			int error=htonl(-1);
		 			write(serv1->sockfd,&error,sizeof(error));	
		 			return -1;
		 		
		 		}
		 		//Caso contrario envia um inteiro a confirmar que tudo ok
		 		//Pode enviar mensagens de actualizacao
		 		else{
		 			printf("A receber actualizacoes de primario...\n");
		 			int fine=htonl(0);
		 			write(serv1->sockfd,&fine,sizeof(fine));
		 			
		 			read(serv1->sockfd,&sizetable,sizeof(sizetable));
		 			
		 			//Para cada entry na table do outro servidor
		 			//Vai actualizar a table deste
		 			sizetable=ntohl(sizetable);
		 			int j;
		 			int bufsize2;
		 			struct message_t *update2 = (struct message_t*)malloc(sizeof(struct message_t));
		 			for(j=0;j<sizetable;j++){
		 			
		 				//le tamanho do buffer
		 				read(serv1->sockfd,&bufsize2,sizeof(int));
		 				
			 			//bufsize foi actualizado pelo read anterior
	 					bufsize2=ntohl(bufsize2);
	 		
	 					printf("Tamanho do buffer recebido:%d\n",bufsize2);
	 
	 					//Cria buffer 
	 					char *buffermsg2 = (char*) malloc(bufsize2*sizeof(char));
	 					
	 					//Funcao auxiliar que le todos os bytes do socket
	 					if(readall(serv1->sockfd,buffermsg2,bufsize2)!=bufsize2){
	 						
	 						perror("Erro ao ler buffer de actualizacao"); 
							close(serv1->sockfd); 
							return -1;
	 					
	 					}
		
						//converte o buffer numa mensagem
						update2 = buffer_to_message(buffermsg2,bufsize2);
						
						invoke(update2);
						
						free(buffermsg2);
					//Fim do for
		 			}
		 			free_message(update2);
		 		
		 		}
		 		
		 	
		 	 }
		}
	}
	
	
	struct sockaddr_in server,client;
	socklen_t size_client = sizeof(struct sockaddr_in);
	
	// Cria socket TCP 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) { 
		perror("Erro ao criar socket"); 
		return -1; 
 	}
 	
 	/* Permite reutilização do socket */ 
	int sim = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (int *)&sim, sizeof(sim)) < 0 ) { 
		
		perror("SO_REUSEADDR setsockopt error"); 
		return -1;
 	}
 	
 	 // Preenche estrutura server para bind
	server.sin_family = AF_INET; 
	server.sin_port = htons(atoi(argv[2])); 
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Faz bind
	if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){ 
		perror("Erro ao fazer bind"); 
		close(sockfd); 
		return -1; 
 	}
 	
 	// Faz listen
	if (listen(sockfd, 0) < 0){ 
		perror("Erro ao executar listen"); 
		close(sockfd); 
		return -1; 
 	}
 	
	printf("Servidor 'a espera de dados\n");
	
	
	//Socket de cliente
	int connsockfd;
	
	//Funcao de controlo de sinais
	signal(SIGINT, interrupt_handler);
	signal(SIGPIPE,SIG_IGN);
	
	int bufsize=0;
	struct message_t *message;
	
	//Espera por conexao de um cliente
	//Parte do poll	
	//********************************************************
	
	//MAX_CLIENT DEFINIDO NO MESSAGE-PRIVATE
	//FACILMENTE ALTERA-SE O VALOR PARA A QUANTIDADE DE CLIENTES QUE SE QUER
	struct pollfd desc_set[MAX_CLIENT+1];
	desc_set[0].fd = sockfd;
	desc_set[0].events = POLLIN;
	
	//Se conseguiu fazer um hello com sucesso 
	//Guarda o outro servidor na segunda casa do poll
	if( serv1->sockfd!=-1 ){
		desc_set[1].fd = serv1->sockfd;
		desc_set[1].events = POLLIN;
	}else{
		desc_set[1].fd = 0;
		desc_set[1].events = 0;
	}
	
	int i;
	for(i=2;i<MAX_CLIENT+1;i++){
	
		desc_set[i].fd = 0;
		desc_set[i].events = 0;
	
	}
	
	
	while(poll(desc_set,MAX_CLIENT+1,-1) >= 0) {
		
		//Se a socket do servidor tem dados para ler
		if( desc_set[0].revents & POLLIN ) {
		
			//aceita a conexao
			connsockfd = accept(sockfd,(struct sockaddr *) &client, &size_client);
			
			printf("Conexao recebida...\n");
			
			//Vai verificar se eh um cliente ou servidor
			int verify;
			read(connsockfd,&verify,sizeof(int));
			
			verify=ntohl(verify);
			
			if(verify==1){
				printf("Ligacao tipo cliente!\n");
				//Se este servidor receber uma conexao do tipo cliente
				//Entao eh um servidor primario
				//
				primario=1;
				
				//Procura pelo primeiro poll livre
				for(i=2;(i<MAX_CLIENT+1 && desc_set[i].fd!=0);i++);
				
				//Verifica se tem um servidor secundario para enviar ou nao um backup
				if(serv1->sockfd!=-1)
					backup=1;
				else
					backup=0;
			}
			else{
				i=1;
			
				printf("Ligacao tipo servidor!\n");
				int servtype;
				
				printf("Socket:%d\n",serv1->sockfd);
				if(serv1->sockfd==-1){
					serv1->sockfd = connsockfd;
					serv1->address = inet_ntoa(client.sin_addr);
					serv1->port = ntohs(client.sin_port);
				}

				read(connsockfd,&servtype,sizeof(servtype));
				
				if(servtype==0){
					int iknow;

					//Se este servidor for o primario avisa o secundario do que eh
					if(primario==1){
						iknow=1;
						write(connsockfd,&iknow,sizeof(iknow));
					
						printf("Servidor Secundario is UP\n");
						
						/* Envia tabela para o secundario */
						
						//Manda as buckets
						int buckets = htonl(atoi(argv[1]));
						write(connsockfd,&buckets,sizeof(buckets));
						
						//Espera que o secundario confirma que tem o mesmo numero de buckets
						read(connsockfd,&buckets,sizeof(int));
						
						buckets=ntohl(buckets);
						//Se receber 0 esta tudo ok pode enviar mensagens
						if(buckets==0){
							printf("A mandar actualizacoes a Secundario...\n");
							
							struct message_t *update = (struct message_t*)malloc(sizeof(struct message_t));
							
							update->opcode = OC_SIZE;
			
							update->c_type= CT_RESULT;
			
							invoke(update);
							//envia o numero de entradas na table
							int num_chaves=htonl(update->content.result);
							
							write(connsockfd,&num_chaves,sizeof(num_chaves));
							
							update->opcode = OC_GET_KEYS;
							
							invoke(update);
							
							char** cpkeys = update -> content.keys;
							int o;
							struct entry_t *temp;
							struct data_t *dat;
							for(o=0;cpkeys[o];o++){
								
								update->opcode = OC_GET;
								
								update->content.key=cpkeys[o];
								
								invoke(update);
								
								dat = update->content.value;
								
								temp = entry_create(cpkeys[o],dat);
							
								update -> opcode = OC_PUT;
								
								update -> c_type= CT_ENTRY;
								
								update -> content.entry = temp;
								
								//Envia mensagem ao secundario
								char**newbuf2=(char **) malloc(MAX_MSG*sizeof(char));
		
								int msg_bufsize2 = message_to_buffer(update,newbuf2);
		
								//envia o tamanho do buffer ao secundario	
								int temp2 = htonl(msg_bufsize2);
			
								write(connsockfd,&temp2,sizeof(temp2));
		
								//envia o buffer ao secundario
								if(writeall(connsockfd,*newbuf2,msg_bufsize2) < 0){
							 		perror("Erro ao enviar buffer\n");
							 		free(newbuf2);
									continue;
							 	}
							 	
							 	free(newbuf2);
							 //Fim do envio de uma actualizacao
							}
							free_message(update);
							
						}
						//ELse
						else{
							printf("Servidor Secundario incompativel...\n");
						}
						
						/***********************************/
						
						backup=1;
					
					}
					//Se este servidor for secundario
					else{
						iknow=0;
						write(connsockfd,&iknow,sizeof(iknow));
					
						printf("Sou servidor backup...\n");

						backup=0;

					}
				}
			}
			
			//preenche a struct
			desc_set[i].fd = connsockfd;
		
			desc_set[i].events = POLLIN;
		}
		
		//um dos sockets tem dados pra ler
		for( i = 1; i < MAX_CLIENT;i++) {
			
			if(desc_set[i].revents & POLLIN) {

				//le ou nao o comando escrito pelo cliente ********************
				nbytes = read(desc_set[i].fd,&bufsize,sizeof(int));
				
				//se nao recebeu dados nenhuns fecha a ligacao com o cliente
				if(nbytes==0) {
					close(desc_set[i].fd);
					desc_set[i].fd = 0;
					desc_set[i].events = 0;
					if(i==1){
						serv1->sockfd=-1;
						printf("Conexao terminada com servidor\n");
					}else{
						printf("Conexao terminada com cliente\n");
					}
					
				}
				//Caso contrario
				//Comeca a fazer a comunicacao, elaborada no projeto anterior
				else{
					
					printf("A receber mensagem!\n");
					
					//bufsize foi actualizado pelo read anterior
 					bufsize=ntohl(bufsize);
 		
 					printf("Tamanho do buffer recebido:%d\n",bufsize);
 
 					//Cria buffer com o tamanho recebido do cliente
 					char *buffermsg = (char*) malloc(bufsize*sizeof(char));
 					
 					//Funcao auxiliar que le todos os bytes do socket
 					if(readall(desc_set[i].fd,buffermsg,bufsize)!=bufsize){
 						
 						perror("Erro ao ler buffer ao cliente"); 
						close(desc_set[i].fd); 
						continue;
 					
 					}
		
					//converte o buffer numa mensagem
					message = buffer_to_message(buffermsg,bufsize);
			
						
						//Se o primario estiver conectado ao secundario
						//Cria uma copia da mensagem original enviada pelo cliente
					if(backup && (message->opcode==OC_PUT || message->opcode==OC_COND_PUT || message->opcode==OC_DEL)){
						callnot=1;
						
						
						serv1->bu_msg = (struct message_t*) malloc(bufsize*sizeof(char*));
						serv1->bu_msg->opcode= message->opcode;
							
						serv1->bu_msg->c_type= message->c_type;
							
						int tempk;
							
						switch(serv1->bu_msg->c_type){
							
							case CT_ENTRY:
								serv1->bu_msg->content.entry=entry_dup(message->content.entry);
								break;
							case CT_KEY:
								serv1->bu_msg->content.key=strdup(message->content.key);
								break;
							case CT_KEYS:
								for(tempk=0;message->content.keys[tempk];tempk++)
									serv1->bu_msg->content.keys[tempk]=strdup(message->content.keys[tempk]);
								break;
							case CT_VALUE:
								serv1->bu_msg->content.value=data_dup(message->content.value);
								break;
							case CT_RESULT:
								serv1->bu_msg->content.result=message->content.result;
								break;
								
							default:
								printf("Copia de messagem com erro\n");
								break;
							
							
						}
							
					}
					//***************************************************
					//Chama a funcao do table_skel
					//que realiza uma operacao na table dado uma message
					//do cliente e actualiza a struct message com os novos
					//dados.
					//***************************************************
					
					invoke(message);
						
						
					//Se backup ligado(==1) e operacao requer actualizacao da table
					//callnot==1 -> actualizar o backup
					if(backup && callnot==1){
						
						
						pthread_create(&psec,NULL,&update,NULL);
						
						pthread_join(psec,(void**)&bu_ok);
							
						if(*bu_ok==0){
							printf("Servidor secundario Actualizado\n");
							free_message(serv1->bu_msg);
						}
						else{
							//O servidor secundario estah com problema
							//Termina a conexao com este backup turnoff
							free_message(serv1->bu_msg);	
							close(serv1->sockfd);
							serv1->sockfd=-1;
							printf("Servidor secundario Down\n");
							backup=0;
						}
								
							
					}
						
					//ENVIAR A MENSAGEM PARA O CLIENTE
					//Novo buffer a enviar para o cliente
					char**newbuf=(char **) malloc(MAX_MSG*sizeof(char));
		
					int msg_bufsize = message_to_buffer(message,newbuf);
					int temp;
					
					//envia o tamanho do novo buffer ao cliente
					//Se o msg_bufsize deu !=-1
					//Operacao correu bem
					if(msg_bufsize!=-1){
						temp = htonl(msg_bufsize);
			
						if((nbytes = write(desc_set[i].fd,&temp,sizeof(temp))) != sizeof(temp)){ 
							perror("Erro ao enviar tamanho do buffer ao cliente"); 
							close(desc_set[i].fd); 
							continue;
	 					} 
	 					
	 					
		 				//envia o novo buffer ao cliente
						//funcao auxiliar que escreve todos os bytes para a socket
						if(writeall(desc_set[i].fd,*newbuf,msg_bufsize) != msg_bufsize){
							perror("Erro ao enviar buffer ao cliente"); 
							close(desc_set[i].fd); 
							continue;
						}
	 					
	 				}
	 				else{
	 					temp=htonl(0);
	 					if((nbytes = write(desc_set[i].fd,&temp,sizeof(temp))) != sizeof(temp)){ 
							perror("Erro ao enviar tamanho do buffer ao cliente"); 
							close(desc_set[i].fd); 
							continue;
	 					} 
	 				
	 				}
		
					
					callnot=0;
					free(buffermsg);
					free(newbuf);
					printf("Operacao com cliente terminada\n");
				//FIM DO ELSE
				}
			//FIM DO IF SE CAPTUROU UM EVENTO
			}
			else{
				//Caso haja um evento de erro ou de desconexao
				if( (desc_set[i].revents & POLLERR) || (desc_set[i].revents & POLLHUP) ) {
					//fecha conexao com o cliente
					close(desc_set[i].fd);
					desc_set[i].fd = 0;
					desc_set[i].events = 0;
					perror("Conexao interrompida com cliente\n");
				}
			}
		//FIM DO FOR DEPOIS DE PERCORRER TODAS AS SOCKETS
		}
	
	//FIM DO WHILE DA CONEXAO COM O CLIENTE
	}
	return -1;
}
