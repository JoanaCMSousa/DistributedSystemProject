#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "message-private.h"
#include "data.h"
#include "entry.h"

/*
 Nuno Narciso Carreiro nº38828
 Ruben Narciso Pavao nº 36011
 Grupo nº 9
*/

//Funcao fornecida para trocar os bits de um long(timestamp)
long long swap_bytes_64(long long number){
        long long new_number;

        new_number =   ((number & 0x00000000000000FF) << 56 |
                        (number & 0x000000000000FF00) << 40 |
                        (number & 0x0000000000FF0000) << 24 |
                        (number & 0x00000000FF000000) << 8  |
                        (number & 0x000000FF00000000) >> 8  |
                        (number & 0x0000FF0000000000) >> 24 |
                        (number & 0x00FF000000000000) >> 40 |
                        (number & 0xFF00000000000000) >> 56);

        return new_number;
}




/* Transforma uma message_t num char*, retornando o tamanho do
 * buffer alocado para a mensagem como um array de bytes, ou -1
 * em caso de erro.
 */
 int message_to_buffer(struct message_t *msg, char **msg_buf){
 
 	//Variaveis que recebem valores conforme o tipo de operacao
 	//Tamanho do buffer
 	int bufferlen=0;
 	//Aponta para uma posicao do buffer
 	int offset=0;
 	//Recebe o tamanho da chave
 	int keylen =0;
 	//recebe a quantidade de chaves e o seu tamanho
 	int nkeys=0,ksize=0;
 	
 	//Conforme o codigo do tipo de operacao entra num dos casos
 	switch (msg->c_type){
 	
 		case CT_ENTRY:
 			//Formato da mensagem:
 			//OPCODE+CTYPE+TIMESTAMP+KEYSIZE+KEY+DATASIZE+DATA
 		
 			//Calcular 'a priori o tamanho do buffer
 			//Com o auxilio das variaveis keylen + temp
 			keylen = strlen(msg->content.entry->key);
 		
 			//Variavel que vai recebendo valores de funcoes/tamanho do datasize
 			int temp = (msg->content.entry->value->datasize);
 		
 			bufferlen = 2 +2 + 8 + 4 + keylen  + 4 + temp;
 		
 			//Alocar espaco para o buffer depois de calculado o tamanho
 			*(msg_buf) = (char*) malloc ( bufferlen * sizeof(char));
 			//Caso nao consiga alocar espaco retorna NULL
 			if(*(msg_buf) == NULL)
 				return -1;
 			
 			//OPCODE
			temp = htons(msg->opcode);

			memcpy( *(msg_buf)+offset , &(temp), 2);
		
			//CTYPE
			temp = htons(msg->c_type);
			
			offset+=2; // coloca o offset na nova posicao do buffer
			
			memcpy( (*(msg_buf))+offset , &(temp), 2);
			
			//Timestamp
			//criar funcao
			
			offset+=2; //offset estah a 2+2
			
			long long tempts=swap_bytes_64(msg->content.entry->timestamp);
		
			memcpy(*msg_buf+offset,&(tempts), sizeof(long long));
			
			offset+=sizeof(long long);//8;
		
			//Keysize
			temp = htonl(keylen);
		
			memcpy(*msg_buf+offset,&(temp), sizeof(int));
		
			offset+=sizeof(int); //4;
		
			//Key
			memcpy(*msg_buf+offset,msg->content.entry->key, keylen);
			
			offset+=keylen;
		
			//Datasize
			temp=htonl(msg->content.entry->value->datasize);
			
			memcpy(*msg_buf+offset,&(temp),sizeof(int));
		
			offset+=sizeof(int);//4;
		
			//Data
			memcpy(*msg_buf+offset,msg->content.entry->value->data,msg->content.entry->value->datasize);
		
		break;
		
		case CT_KEY:
			//Formato da mensagem:
			//OPCODE+C_TYPE+KEYSIZE+KEY
			
			//Tamanho do buffer dado o tipo de formato
			keylen = strlen(msg->content.key);

			bufferlen = 2+2+4+keylen;
		
			*(msg_buf) = (char*) malloc( bufferlen * sizeof(char) );
			
			if(*(msg_buf) == NULL)
				return -1;
				
			//OPCode
			temp = htons(msg->opcode);
			
			memcpy( *(msg_buf)+offset , &(temp), 2);
			
			offset+=2;
			
			//C_type
			temp = htons(msg->c_type);

			memcpy( (*(msg_buf))+offset , &(temp), 2);
			
			offset+=2;
			
			//Keysize
			temp = htonl(keylen);
			
			memcpy( (*(msg_buf))+offset , &(temp), 4); //offset-posicao 2+2
			
			offset+=4;
			//Key
			
			memcpy( (*(msg_buf))+offset ,msg->content.key ,keylen); //offset-posicao 2+2+4
		break;
		
		case CT_KEYS:
			//Formato do buffer:
			//OPCODE+CTTYPE+NKEYS(numero de chaves)+(KEYSIZE+KEY)(por numero de chaves)
			//Conta o numero de chaves
			while(msg->content.keys[nkeys]){
				ksize+=strlen(msg->content.keys[nkeys]);
				nkeys++;
			}
			
			//ksize soma de todos os tamanhos das chaves
			//short short int(nkeys) nkeys*(int(tamanho chave) ksize)
			bufferlen = 2+2+4+(4*nkeys)+ksize;
			
			*msg_buf = (char*) malloc (bufferlen * sizeof(char));
			
			if( *(msg_buf) == NULL)
				return -1;

			//OPcode
			temp = htons(msg->opcode);
			
			memcpy( *(msg_buf)+offset , &(temp), 2);
			
			offset+=2;
			
			//CTtype
			temp = htons(msg->c_type);

			memcpy( (*(msg_buf))+offset , &(temp), 2);
			
			offset+=2;
			
			//NKEYS
			temp= htonl(nkeys);
			
			memcpy( (*(msg_buf))+offset , &(temp) , 4);
			
			offset+=4;
			
			//Keysize e key
			int i;
			int kset=0;
			//Para cada chave mete no buffer o keysize e depois a key
			for(i = 0; i < nkeys ; i++){
				//keysize
				temp = strlen(msg->content.keys[i]);

				temp = htonl(temp);
				
				memcpy( (*(msg_buf))+offset+kset , &(temp),4);
				
				//key
				kset+=4;
				
				memcpy( (*(msg_buf))+offset+kset,msg->content.keys[i] ,strlen(msg->content.keys[i]));
				
				kset+= strlen(msg->content.keys[i]);
				
			}
			
		break;
		
		case CT_RESULT:
		
			//Formato da mensagem:
			//OPCODE+CTTYPE+RESULT
			bufferlen=2+2+4;
			
			*(msg_buf) = (char*) malloc( bufferlen * sizeof(char) );
			
			if(*(msg_buf) == NULL)
				return -1;
				
			//OPCODE
			temp = htons(msg->opcode);
			
			memcpy( *(msg_buf)+offset , &(temp), 2);
			
			offset+=2;
			
			//CTTYPE
			temp = htons(msg->c_type);

			memcpy( *(msg_buf)+offset , &(temp), 2);
			
			offset+=2;
			
			temp = htonl(msg->content.result);

			memcpy( *(msg_buf)+offset , &(temp), 4);
		break;
		
		case CT_VALUE:
			
			//Formato da mensagem:
			//OPCODE+CTTYPE+DATASIZE(int)+DATA
			bufferlen=2+2+4+(msg->content.value->datasize);
			
			*(msg_buf) = (char*) malloc (bufferlen * sizeof(char));

			if( *(msg_buf) == NULL)
				return -1;
			
			//OPCODE
			temp = htons(msg->opcode);

			memcpy( *(msg_buf)+offset , &(temp), 2);

			//CTYPE
			temp = htons(msg->c_type);

			offset+=2;

			memcpy( (*(msg_buf))+offset , &(temp), 2);
			
			offset+=2;

			//Datasize
			temp = htonl(msg->content.value->datasize);

			memcpy(*(msg_buf)+offset,&(temp),sizeof(int));

			offset+=sizeof(int);//4
			
			//Data
			memcpy(*msg_buf+offset,msg->content.value->data,msg->content.value->datasize);
		break;
		
		default:
		return -1;
 	
 	}
 
 return bufferlen;
 }
 
 /* Transforma uma mensagem em buffer para uma struct message_t*
*/
struct message_t *buffer_to_message(char *msg_buf, int msg_size){

	int offset = 0;
	
	int nkeys;
	
	long long timestamp =0;
	
	struct message_t *message;
	
	struct entry_t *entry;
	
	struct data_t *value;

	message = (struct message_t*) malloc (sizeof(struct message_t));
	
	if(message == NULL)
		return NULL;
		
	//OPCODE
	message->opcode = * (( short* )(msg_buf+offset));
	
	message->opcode= ntohs(message->opcode);
	
	offset+=2;
	
	//CTTYPE
	message->c_type = * (( short* )(msg_buf+offset));
	
	message->c_type = ntohs(message->c_type);
	
	offset+=2;
	
	switch(message->c_type){
	
		case CT_ENTRY:
			
			//Timestamp
			timestamp =swap_bytes_64(*(( long long* )(msg_buf+offset)));
			
			//Keysize
			offset+=sizeof(long long);//2+2+8
			
			int ksize = * (( int* )(msg_buf+offset));
			
			ksize = ntohl(ksize);
			
			//key
			offset+=sizeof(int);//offset 2+2+8+4
			
			char* key = strndup(msg_buf+offset,ksize);
			
			if(key == NULL)
				return NULL;
			
			offset+=ksize;//offset 2+2+8+4+ksize
			
			//Pega no datasize e no data e usa o data_create2
			
			int datasize= *(( int* )(msg_buf+offset));
			
			datasize = ntohl(datasize);
			
			offset+=sizeof(int); // offset 2+2+4+8+4+ksize+4
			
			void* catch = malloc(datasize*sizeof(void*));
			
			memcpy(catch,msg_buf+offset,datasize);
			
			struct data_t* value = data_create2(datasize,catch);
			
			//Cria a entry
			
			message->content.entry= entry_create(key,value);
			
			message->content.entry->timestamp=timestamp;
			
			break;
			
		case CT_KEY:
			
			//Keysize
			
			ksize = *(( int* )(msg_buf+offset));
			
			ksize = ntohl(ksize);
			
			//key
			offset+=sizeof(int);//offset 2+2+4
			
			message->content.key = strndup(msg_buf+offset,ksize);
			
			if(message->content.key == NULL)
				return NULL;
			
			break;
			
		case CT_KEYS:
			
			//nkeys
			
			nkeys = *(( int* )(msg_buf+offset));
			
			nkeys = ntohl(nkeys);
			//Aloca memoria para todas as chaves
			message->content.keys = malloc( (nkeys+1) * sizeof(char*));
			
			//Para cada chave:
			//keysize e key
			
			offset+=sizeof(int); // offset: 2 + 2 + 4
			
			int i,temp=0;
			ksize=0;
			for(i = 0; i < nkeys; i++){
			
				//Keysize
				
				ksize= * (( int* )(msg_buf+offset+temp));
				
				ksize = ntohl(ksize);
				
				temp+=sizeof(int);//avanca 4
				
				//Key
				message->content.keys[i] = strndup(msg_buf+offset+temp,ksize);
				//SE nao conseguiu alocar memoria para a key
				if(message->content.keys[i] == NULL)
					return NULL;
			
				temp+=ksize;//avanca o offset para o proximo keysize
				
			}
			message->content.keys[i]=NULL;
			break;
			
		case CT_VALUE:
			
			//Pega no datasize e no data e usa o data_create2
			
			datasize= *(( int* )(msg_buf+offset));
			
			datasize = ntohl(datasize);
			
			offset+=sizeof(int); // offset 2+2+4
			
			catch = malloc(datasize*sizeof(void*));
			
			memcpy(catch,msg_buf+offset,datasize);
			
			message->content.value = data_create2(datasize,catch);
			
			break;
			
		case CT_RESULT:
		
			message->content.result = * (( int* )(msg_buf+offset));
			
			message->content.result = ntohl(message->content.result);
			
			break;
			
			default : 
			free(message);
			return NULL;
				
	}
	
	return message;

}

/* Liberta a memoria alocada na função buffer_to_message
*/
void free_message(struct message_t *message){
	int i;
	switch (message->c_type){
		
		case CT_ENTRY:
			entry_destroy(message->content.entry);
			free(message);

		break;
		
		case CT_KEY:
			free(message->content.key);
			
			free(message);
		
		break;
		
		case CT_KEYS:
			

			for(i=0;message->content.keys[i];i++){

				free(message->content.keys[i]);

			}
			free(message->content.keys);
			
			free(message);
		
		break;
		
		case CT_RESULT:
			free(message);
		
		break;
		
		case CT_VALUE:
			data_destroy(message->content.value);
			
			free(message);
		
		break;
		
		default : break;
	}

}

//Funcao que le todos os bytes de uma socket
int readall(int sockfd, char* buffer, int len){

	int nbytes=0;
	int ptr = 0;
 	int nleft = len;
 		
 	while(nleft>0){
		//recebe buffer
		nbytes=read(sockfd,buffer+ptr,nleft);
		
		if(nbytes<=0){
			perror("Erro ao ler buffer...\n"); 
			close(sockfd); 
			return -1; 
		}
		//actualiza o ponteiro do buffermsg
		nleft -= nbytes;
		ptr += nbytes;
	}
	
	return nbytes;
}


//Funcao que escreve todos os bytes de uma socket
int writeall(int sockfd, char* buffer, int len){

	int nbytes=0;
	int nleft = len;
	int ptr = 0;
	
	while(nleft>0){
		//envia buffer
		nbytes=write(sockfd,buffer,nleft);
		
		if(nbytes<=0){
			perror("Erro ao escrever buffer...\n"); 
			close(sockfd); 
			return -1;
		}
			
		//actualiza o ponteiro do newbuf
		nleft -= nbytes;
		*buffer += nbytes;
	}

	return nbytes;
}


