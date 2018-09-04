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

int testInput(int argc){
	if(argc != 2){
		printf("Uso: table-client <ip servidor>:<porta servidor>\n");
		printf("Exemplo de uso: ./table_client 10.101.148.144:54321\n");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv){
	struct server_t *server;
	char input[81];
	struct message_t *msg_out, *msg_resposta;

	/* Testar os argumentos de entrada */
	if(testInput(argc) < 0){return -1;}

	/* Usar network_connect para estabelcer ligação ao servidor */
	server = network_connect(argv[1]);//porto_servidor

	/* Fazer ciclo até que o utilizador resolva fazer "quit" */
	char *option;
	option = (char *) malloc(sizeof(char) * MAX_MSG);
	bzero(option, MAX_MSG);

	signal(SIGPIPE, SIG_IGN);

 	while (strcmp(option, "quit") != 0){

		printf(">>> "); // Mostrar a prompt para inserção de comando

		msg_out = (struct message_t *) malloc(sizeof(struct message_t));

		if(msg_out == NULL){return -1;}

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
			free(msg_out);
			return network_close(server);
		}		

		/* Caso contrário:

			Verificar qual o comando;

			Preparar msg_out;

			Usar network_send_receive para enviar msg_out para
			o server e receber msg_resposta.
		*/
		
		char valid_input = FALSE;
		char *key, *aux;
		struct data_t *data;

		if(strncmp(option, "put ", 4) == 0 && strlen(option) >= 7){
			struct entry_t *entry;
			msg_out->opcode = OC_PUT;
			msg_out->c_type = CT_ENTRY;
			key = strtok(option + 4, " ");
			aux = strtok(NULL," ");
			if(key != NULL && aux != NULL){
				data = data_create2(strlen(aux), aux);
				entry = entry_create(key, data);
				msg_out->content.entry = entry;
				valid_input = TRUE;
				data_destroy(data);
			}
		}

		else if(strncmp(option, "get ", 4) == 0 && strlen(option) >= 5){
			msg_out->opcode = OC_GET;
			msg_out->c_type = CT_KEY;
			key = strtok(option + 4, " ");
			if(key != NULL){
				msg_out->content.key = strdup(key);
				valid_input = TRUE;
			}
		}

		else if(strncmp(option, "del ", 4) == 0 && strlen(option) >= 5){
			msg_out->opcode = OC_DEL;
			msg_out->c_type = CT_KEY;
			key = strtok(option + 4, " ");
			if(key != NULL){
				msg_out->content.key = strdup(key);
				valid_input = TRUE;
			}
		}

		else if(strncmp(option, "update ", 7) == 0 && strlen(option) >= 10){
			struct entry_t *entry;
			msg_out->opcode = OC_UPDATE;
			msg_out->c_type = CT_ENTRY;
			key = strtok(option + 7, " ");
			aux = strtok(NULL, " ");
			if(key != NULL && aux != NULL){
				data = data_create2(strlen(aux), aux);
				entry = entry_create(key, data);
				msg_out->content.entry = entry;
				valid_input = TRUE;
				data_destroy(data);
			}

		}

		else if(strncmp(option, "size", 4) == 0){
			msg_out->opcode = OC_SIZE;
			msg_out->c_type = CT_RESULT;
			msg_out->content.result = 0;
			valid_input = TRUE;
		}

		if(valid_input){		
			printf("Mensagem enviada para o servidor\n");
			print_msg(msg_out);
			msg_resposta = network_send_receive(server, msg_out);
			printf("Mensagem recebida do servidor\n");
			print_msg(msg_resposta);
			free_message(msg_out);
			free_message(msg_resposta);
		} else
			free(msg_out);
	}

	free(option);
  	return network_close(server);
}
