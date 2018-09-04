/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

/*
	Programa cliente para manipular tabela de hash remota.
	Os comandos introduzido no programa não deverão exceder
	80 carateres.

	Uso: table-client <ip servidor>:<porta servidor>
	Exemplo de uso: ./table_client 10.101.148.144:54321
*/

#include "network_client-private.h"
#include "client_stub-private.h"

int testInput(int argc){
	if(argc != 3){
		printf("Uso: table-client <ip servidor>:<porta servidor> <ip secundario>:<porto secundario>\n");
		printf("Exemplo de uso: ./table_client 10.101.148.144:54321 20.202.259.255:12345\n");
		return -1;
	}
	return 0;
}

int options(char *optionNew, struct rtable_t *server){
	
	if(optionNew == NULL || server == NULL){return -1;}

	char *key, *aux;
	struct data_t *data;
	int result;
	char **keys;
	char *option = strdup(optionNew);

	if(strncmp(option, "put ", 4) == 0 && strlen(option) >= 7){
		key = strtok(option + 4, " ");
		aux = strtok(NULL," ");
		if(key != NULL && aux != NULL){
			data = data_create2(strlen(aux), aux);
			if(data == NULL){free(option);return -1;}
			result = rtable_put(server,key,data);
			data_destroy(data);
			if(result < 0){free(option);return -1;}
		}
	}

	else if(strncmp(option, "get *", 5) == 0){
		keys = rtable_get_keys(server);
		if(keys == NULL){free(option);return -1;}
		rtable_free_keys(keys);
	}

	else if(strncmp(option, "get ", 4) == 0 && strlen(option) >= 5){
		key = strtok(option + 4, " ");
		if(key != NULL){
			data = rtable_get(server,key);
			if(data == NULL){free(option);return -1;}
			data_destroy(data);
		}
	}

	else if(strncmp(option, "del ", 4) == 0 && strlen(option) >= 5){
		key = strtok(option + 4, " ");
		if(key != NULL){
			result = rtable_del(server,key);
			if(result < 0){free(option);return -1;}
		}
	}

	else if(strncmp(option, "update ", 7) == 0 && strlen(option) >= 10){
		key = strtok(option + 7, " ");
		aux = strtok(NULL, " ");
		if(key != NULL && aux != NULL){
			data = data_create2(strlen(aux), aux);
			if(data == NULL){free(option);return -1;}
			result = rtable_update(server,key,data);
			data_destroy(data);
			if(result < 0){free(option);return -1;}
		}
	}

	else if(strncmp(option, "size", 4) == 0){
		result = rtable_size(server);
		if(result < 0){free(option);return -1;}
	}

	free(option);
	return 0;
}

int main(int argc, char **argv){
	struct rtable_t *server;
	struct rtable_t *backup_server;
	int secundario;//argv[1] -> primario, argv[2] -> secundario

	/* Testar os argumentos de entrada */
	if(testInput(argc) < 0){return -1;}

	/* Usar network_connect para estabelcer ligação ao servidor */
	server = rtable_bind(argv[1]);//porto_servidor
	if(server == NULL){return -1;}
	secundario = 2;

	/* Fazer ciclo até que o utilizador resolva fazer "quit" */
	char *option;
	option = (char *) malloc(sizeof(char) * MAX_MSG);
	bzero(option, MAX_MSG);

	signal(SIGPIPE, SIG_IGN);

 	while (strcmp(option, "quit") != 0){

		printf(">>> "); // Mostrar a prompt para inserção de comando

		/* Receber o comando introduzido pelo utilizador
		   Sugestão: usar fgets de stdio.h
		   Quando pressionamos enter para finalizar a entrada no
		   comando fgets, o carater \n é incluido antes do \0.
		   Convém retirar o \n substituindo-o por \0.
		*/

		if(fgets(option, MAX_MSG-1, stdin) == NULL){return -1;}
		option[strlen(option) - 1] = '\0';

		/* Verificar se o comando foi "quit". Em caso afirmativo
		   não há mais nada a fazer a não ser terminar decentemente.
		 */

		if(strcmp(option, "quit") == 0){
			free(option);
			free_address_port();
			return rtable_unbind(server);
		}

		/* Caso contrário:

			Verificar qual o comando;

			Preparar msg_out;

			Usar network_send_receive para enviar msg_out para
			o server e receber msg_resposta.
		*/
		
		int valid_command;
		valid_command = options(option, server); 

		if(valid_command == -1){//se o primario nao responde
			free(server);
			server = rtable_bind(argv[secundario]);//bind secundario
			if(server == NULL){
				valid_command = -1;
			}
			else{
				valid_command = options(option, server);//envia a mensagem ao secundario
			}

			if(valid_command == -1){//se o secundario nao responde
				sleep(RETRY_TIME);//espera RETRY_TIME
				if(secundario == 2)
					secundario = 1;
				else
					secundario = 2;
				
				free(server);
				server = rtable_bind(argv[secundario]);//bind primario
				if(server == NULL){
					valid_command = -1;
				}
				else{
					valid_command = options(option, server);//volta a tentar o primario
				}
				
				if(valid_command == -1){//se o primario continua sem responder
					if(secundario == 2)
						secundario = 1;
					else
						secundario = 2;
					
					free(server);
					server = rtable_bind(argv[secundario]);//bind secundario
					if(server == NULL){
						valid_command = -1;
					}
					else{
						valid_command = options(option, server);//volta a tentar o secundario
					}

					if(valid_command == -1){//se o secundario continua sem responder
						printf("Nao teve sucesso. Ambos os servidores estao em baixo.\n");//desiste da operaçao
					}
				}
				else{//se o primario responde
					if(secundario == 2)
						secundario = 1;
					else
						secundario = 2;
				}
			}
			else{//se o secundario responde
				if(secundario == 2)
					secundario = 1;
				else
					secundario = 2;
			}
		}
	}

	free(option);
	free_address_port();
	return rtable_unbind(server);
}