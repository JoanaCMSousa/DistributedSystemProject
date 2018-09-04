#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "message.h"
#include "network_client.h"
#include "network_client-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
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
 if ( (argc != 2) && (argc != 3) ){ 
printf("Uso: ./table_client <ip_servidor>:<porto_servidor> [<ip_backup>:<porto_backup>]\n"); 
        printf("Exemplo(sem replicacao): ./table_client 127.0.0.1:12345\n");
        printf("Exemplo(com replicacao): ./client 127.0.0.1:12345 127.0.0.1:12346\n");
return -1; 
 } 
return 0; 
}


int quit=0;
struct rtable_t *rtable;

void interrupt_handler(int sig){

	rtable_unbind(rtable);
	exit(sig);

}


int main(int argc, char **argv) {

	char buffer[MAX_MSG];
	
	//Variavel que salta entre o primeiro e segundo parametro da linha de comandos
	//Conseguindo-se conectar ao servidor que estiver up
	int backup=2;
	
	//Confirma se o programa foi chamado correctamente
	if(testInput(argc) <0) return -1;
	
	//Mete o endereco:porto do servidor
	// no cliente_stub
	rtable = rtable_bind(argv[1]);
	
	//flag de operacao realizada
	//Se flag diferente de -1 entao operacao correu com sucesso
	int flag=1;
	
	struct data_t *getd;
	char **getkeys;
	char *key;
	char *data;
	struct data_t *d;
	
	//Variaveis backup
	char *kbu;
	struct data_t *dbu;
	
	signal(SIGPIPE,SIG_IGN);
	signal(SIGINT,interrupt_handler);
	
	
	while(quit==0){
	
		fgets(buffer,MAX_MSG,stdin);
		int c;
	
		//Pega na string que contem a operacao que o utilizador deseja realizar
		char *operation;
		//Se no buffer houver espacos 
		if(strstr(buffer," ")!=NULL){
			for(c= 0;buffer[c]!=' ';c++);
			operation = (char *) malloc( c*sizeof(char));
			memcpy(operation,buffer+0,c);
		}
		
		//Se nao houver espacos ex:Size getkeys Quit
		else{
			operation = (char *) malloc( strlen(buffer)-1*sizeof(char));
			
			memcpy(operation,buffer+0,strlen(buffer)-1);
		}
	
	
		//Offset que aponta para a parte da data do buffer
		int offset=strlen(operation)+1;//retira espaco

		//Caso o comando for o put
		if(strcmp(operation,"put")==0){
			
			//Offset que vai apontar para a key do buffer
			 c=0;
			
			for(c = strlen(buffer);buffer[c]!=' ';c--);
			
			c+=1;
			
			int keysize= strlen(buffer)-c-1; //buffer-apontador-newline
			
			key = (char *) malloc((keysize)*sizeof(char));
			
			memcpy(key,buffer+c,keysize);
			
			//Parte da data
			int datasize = strlen(buffer)-keysize-strlen(operation)-3;//-3 por questoes de espacos da string completa
			
			data=(char *)malloc( datasize*sizeof(char));
			
			memcpy(data,buffer+offset,datasize);
			
			//Cria a data
			
			d = data_create2(datasize,data);
			
			
			//Backup
			kbu=strdup(key);
			dbu=data_dup(d);
			
			//
			
			//chama o stub para preencher uma mensagem
			//****************************************
			flag = rtable_put(rtable,key,d);
			
		}
	
		//Caso cput
		if(strcmp(operation,"cput")==0){
		
		//Offset que vai apontar para a key do buffer
			 c=0;
			
			for(c = strlen(buffer);buffer[c]!=' ';c--);
			
			c+=1;
			
			int keysize= strlen(buffer)-c-1; //buffer-apontador-newline
			
			key = (char *) malloc((keysize)*sizeof(char));
			
			memcpy(key,buffer+c,keysize);
			
			//Parte da data
			int datasize = strlen(buffer)-keysize-strlen(operation)-3;//-3 por questoes de espacos da string completa
			
			data=(char *)malloc( datasize*sizeof(char));
			
			memcpy(data,buffer+offset,datasize);
			
			//Cria a data e a entry
			
			d = data_create2(datasize,data);
			
			//BACKUP
			kbu=strdup(key);
			dbu=data_dup(d);
			
			
			//chama o stub para preencher uma mensagem
			//****************************************
			flag = rtable_conditional_put(rtable,key,d);
		
		}
	
	
		//Caso o comando for o get
		if(strcmp(operation,"get")==0){
			
			//Offset que vai apontar para a key do buffer
			 c=0;
			
			for(c = strlen(buffer);buffer[c]!=' ';c--);
			
			c+=1;
			
			int keysize= strlen(buffer)-c-1; //buffer-apontador-newline
			
			key = (char *) malloc((keysize)*sizeof(char));
			
			memcpy(key,buffer+c,keysize);
			
			
			//BAckup
			kbu=strdup(key);
				
			//chama o stub para preencher uma mensagem
			//****************************************
			getd = rtable_get(rtable,key);
			
			if(getd!=NULL)
				flag=0;
			else
				flag=-1;
			
		}
	
	
		//Caso o comando for o del
		if(strcmp(operation,"del")==0){
			
			//Offset que vai apontar para a key do buffer
			 c=0;
			
			for(c = strlen(buffer);buffer[c]!=' ';c--);
			
			c+=1;
			
			int keysize= strlen(buffer)-c-1; //buffer-apontador-newline
			
			key = (char *) malloc((keysize)*sizeof(char));
			
			memcpy(key,buffer+c,keysize);
			
			//Backup
			kbu=strdup(key);
			
			
			//Preenche a mensagem
			//chama o stub para preencher uma mensagem
			//****************************************
			flag = rtable_del(rtable,key);
			
		}
	
		//Caso o comando for o Size
		if(strcmp(operation,"Size")==0){
						
			//Preenche a mensagem
			//chama o stub para preencher uma mensagem
			//****************************************
			flag = rtable_size(rtable);

			
		}
	
		//Caso o comando for o getkeys
		if(strcmp(operation,"getkeys")==0){
						
			//Preenche a mensagem
			//chama o stub para preencher uma mensagem
			//****************************************
			//GUARDAR NUM ARRAY DE STRINGS
			//++++++++++++++++++++++++++++++++++++++++
			getkeys = rtable_get_keys(rtable);
			
			if(getkeys!=NULL)
				flag=0;
			else
				flag=-1;

		}
	
		//Caso o comando for o Quit
		if(strstr(operation,"Quit")!=NULL){
	
			quit=1;
		
			flag = rtable_unbind(rtable);
		
		}
		
	
		if(flag==-1){
			printf("Operacao:%s causou erro...\n",operation);
			
			//Veh se o erro eh por causa do servidor que fechou
			int error = 0;
			socklen_t len = sizeof (error);
			int retval = getsockopt (rtable->server->sockfd, SOL_SOCKET, SO_ERROR, &error, &len );
			//END
			
			//Se foi uma das operacoes validas que deu erro EEEE detecta que a socket estah indisponivel
			//Tenta conectar ao servidor backup
			if(((strcmp(operation,"put")==0)||(strcmp(operation,"cput")==0)||(strcmp(operation,"Size")==0)||(strcmp(operation,"del")==0)||(strcmp(operation,"get")==0)||(strcmp(operation,"getkeys")==0)) && retval!=0 && argc==4){
				rtable_unbind(rtable);
				printf("A conectar a servidor%d..\n",backup);
				sleep(3);
				//Fazer bind com o servidor backup
				rtable = rtable_bind(argv[backup]);
				
				//A cada chamada alterna a ligacao dos servidores
				if(backup==2)
					backup=1;
				else 
					backup=2;
				
				printf("A reenviar operacao...\n");
				//Volta a fazer a operacao com o backup
				if(strcmp(operation,"put")==0){
				
					flag = rtable_put(rtable,kbu,dbu);
				}
				
				if(strcmp(operation,"cput")==0){
					flag = rtable_conditional_put(rtable,kbu,dbu);
				}
				
				
				if(strcmp(operation,"get")==0){
					//chama o stub para preencher uma mensagem
					//****************************************
					getd = rtable_get(rtable,kbu);
			
				if(getd!=NULL)
					flag=0;
				else
					flag=-1;
				}
				
				if(strcmp(operation,"del")==0){
					flag = rtable_del(rtable,kbu);
				}
				
				if(strcmp(operation,"Size")==0){
					flag = rtable_size(rtable);
				}
				
				
				if(strcmp(operation,"getkeys")==0){
					getkeys = rtable_get_keys(rtable);
			
					if(getkeys!=NULL)
						flag=0;
					else
						flag=-1;
				}
			//Fim do if de operacoes validas e de tentar ligar ao backup	
			}
			if(retval==0)
				printf("Servidor esta' ligado mas impossivel de concluir operacao confirme os dados...\n");
			
			
		}
	
		//Verifica os codigos do OPCODE 
		//e confirma se as operacoes
		//foram realizadas com sucesso
		if(quit==0 && flag!=-1){
			
			//verifica se o OPCODE da nova mensagem eh opcode+1
			if( rtable->receive->opcode == rtable->send->opcode+1)
				printf("Operacao concluida com sucesso!\n");
		
			//Como o resultado do size foi guardado na flag / imprime a flag		
			if( rtable->receive->opcode == OC_SIZE+1 )
				printf("Tamanho da table:%d\n",flag);
			
			//Imprime o value mas value eh um void*
			if( rtable->receive->opcode == OC_GET+1 )
				printf("Data da key:%s\n",(char*)getd);
			
			//Imprime as chaves
			if( rtable->receive->opcode == OC_GET_KEYS+1 ){
		
				int i;
			
				for(i=0;getkeys[i];i++)
					printf("Key%d: %s\n",i,getkeys[i]);
			}
			//"reset" na flag
			flag=1;
		
		}
	

	//FIM DO WHILE
	}
	
	return 0;
}

