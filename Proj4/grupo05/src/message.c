/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "message-private.h"

void print_msg(struct message_t *msg) {
	int i;
	
	printf("----- MESSAGE -----\n");
	printf("opcode: %d, c_type: %d\n", msg->opcode, msg->c_type);
	switch(msg->c_type) {
		case CT_ENTRY:{
			printf("key: %s\n", msg->content.entry->key);
			printf("datasize: %d\n", msg->content.entry->value->datasize);
		}break;
		case CT_KEY:{
			printf("key: %s\n", msg->content.key);
		}break;
		case CT_KEYS:{
			for(i = 0; msg->content.keys[i] != NULL; i++) {
				printf("key[%d]: %s\n", i, msg->content.keys[i]);
			}
		}break;
		case CT_VALUE:{
			printf("datasize: %d\n", msg->content.data->datasize);
		}break;
		case CT_RESULT:{
			printf("result: %d\n", msg->content.result);
		};
	}
	printf("-------------------\n");
}

void free_message(struct message_t *msg){

  /* Verificar se msg é NULL */
  if(msg == NULL){return;}

  /* Se msg->c_type for:
      VALOR, libertar msg->content.data
      ENTRY, libertar msg->content.entry
      CHAVES, libertar msg->content.keys
      CHAVE, libertar msg->content.key
  */
  switch(msg->c_type){
    case CT_VALUE: 
      data_destroy(msg->content.data);
      break;
    case CT_KEY: 
      free(msg->content.key);
      break;
    case CT_KEYS:
      list_free_keys(msg->content.keys);
      break;
    case CT_ENTRY:
      entry_destroy(msg->content.entry);
      break;
  }

  /* libertar msg */
  free(msg);
}

int message_to_buffer(struct message_t *msg, char **msg_buf){

  /* Verificar se msg é NULL */

  if(msg == NULL || msg_buf == NULL){return -1;}

  /* Consoante o msg->c_type, determinar o tamanho do vetor de bytes
     que tem de ser alocado antes de serializar msg
  */

  int buffer_size, nkeys; //nkeys é para o caso CT_TYPE = CT_KEYS
  buffer_size = 4; //OPCODE+CT_TYPE

  int ksize, data;
  switch(msg->c_type){ 

    case CT_RESULT:
      //formato:
      //buffer_size = OPCODE + CT_TYPE + RESULT
      buffer_size += 4;
      break;
    case CT_VALUE: 
      //formato:
      //buffer_size = OPCODE + CT_TYPE + DATASIZE + DATA
      data = msg->content.data->datasize;
      buffer_size += 4 + data;
      break;
    case CT_KEY: 
      //formato:
      //size = OPCODE + CT_TYPE + KEYSIZE + KEY 
      ksize= strlen(msg->content.key);
      buffer_size += (2 + ksize);
      break;
    case CT_KEYS:
      //formato:
      //size = OPCODE + CT_TYPE + NKEYS + KEYSIZE + KEY
      nkeys = 0;
      ksize = 0;
      while(msg->content.keys[nkeys]){
        ksize += strlen(msg->content.keys[nkeys]);
        nkeys++;
      } 
      buffer_size += 4 + (nkeys*2) + ksize;
      break;
    case CT_ENTRY:
      //formato:
      //size = OPCODE + CT_TYPE + KEYSIZE + KEY + DATASIZE + DATA
      ksize = strlen(msg->content.entry->key);
      data = msg->content.entry->value->datasize;
      buffer_size += (2 + ksize + 4 + data);
      break;
    default:
    return -1;
  }

  /* Alocar quantidade de memória determinada antes 
     *msg_buf = ....
  */
  char * ptr;
  int short_value, value;
  *(msg_buf) = (char *) malloc(buffer_size * sizeof(char));

  if(*(msg_buf) == NULL){return -1;}

  /* Inicializar ponteiro auxiliar com o endereço da memória alocada */
  ptr = *msg_buf;

  short_value = htons(msg->opcode);
  memcpy(ptr, &short_value, _SHORT);
  ptr += _SHORT;

  short_value = htons(msg->c_type);
  memcpy(ptr, &short_value, _SHORT);
  ptr += _SHORT;

  /* Consoante o conteúdo da mensagem, continuar a serialização da mesma */

  switch(msg->c_type){ 

    case CT_RESULT:
      value = htonl(msg->content.result);
      memcpy(ptr, &value, _INT);
      break;    

    case CT_VALUE: 
      value = htonl(msg->content.data->datasize);
      memcpy(ptr, &value, _INT);
      ptr += _INT;
      memcpy(ptr, (msg->content.data->data), (msg->content.data->datasize));
      break;
    case CT_KEY: 
      value = htons(strlen(msg->content.key));
      memcpy(ptr, &value, _SHORT);
      ptr += _SHORT;
      memcpy(ptr, (msg->content.key), (strlen(msg->content.key)));
      break;
    case CT_KEYS:
      value = htonl(nkeys);
      memcpy(ptr, &value, _INT);
      ptr += _INT;

      int i;

      for(i = 0; i < nkeys; i++){
        value = htons(strlen(msg->content.keys[i]));
        memcpy(ptr, &value, _SHORT);
        ptr += _SHORT;
        memcpy(ptr, (msg->content.keys[i]), strlen(msg->content.keys[i]));
        ptr += strlen(msg->content.keys[i]);
      }
      break;

    case CT_ENTRY:
      value = htons(strlen(msg->content.entry->key));
      memcpy(ptr, &value, _SHORT);
      ptr += _SHORT;
      memcpy(ptr, (msg->content.entry->key), strlen(msg->content.entry->key));
      ptr += strlen(msg->content.entry->key);
      value = htonl(msg->content.entry->value->datasize);
      memcpy(ptr, &value, _INT);
      ptr += _INT;
      memcpy(ptr, (msg->content.entry->value->data), msg->content.entry->value->datasize);
      break;
    default:
    return -1;
  }

  return buffer_size;

}

struct message_t *buffer_to_message(char *msg_buf, int msg_size){

  /* Verificar se msg_buf é NULL */
  if(msg_buf == NULL){return NULL;}

  /* msg_size tem tamanho mínimo ? */
  if(msg_size < 7){return NULL;}//2 OP + 2 CT + 3 do 'get *', pois eh o mais pequeno

  /* Alocar memória para uma struct message_t */
  struct message_t *msg;
  msg = (struct message_t *) malloc(sizeof(struct message_t));
  if(msg == NULL){return NULL;}

  /* A mesma coisa que em cima mas de forma compacta, ao estilo C! */
  msg->opcode = ntohs(*(short *) msg_buf++);
  msg->c_type = ntohs(*(short *) ++msg_buf);
  msg_buf += _SHORT;

  /* O opcode e c_type são válidos? */
  if(msg->opcode != OC_SIZE && msg->opcode != OC_DEL && msg->opcode != OC_UPDATE 
        && msg->opcode != OC_GET && msg->opcode != OC_PUT && msg->opcode != OC_HELLO &&
	msg->opcode != OC_SERVER && msg->opcode != (OC_SIZE+1) && msg->opcode != (OC_DEL+1) &&
	msg->opcode != (OC_UPDATE+1) && msg->opcode != (OC_GET+1) && msg->opcode != (OC_PUT+1) &&
	msg->opcode != (OC_HELLO+1) && msg->opcode != (OC_SERVER+1) && msg->opcode != OC_RT_ERROR){
    free_message(msg);
    return NULL;
  }
  
  if(msg->c_type != CT_RESULT && msg->c_type != CT_VALUE && msg->c_type != CT_KEY 
        && msg->c_type != CT_KEYS && msg->c_type != CT_ENTRY){
    free_message(msg);
    return NULL;
  }

  /* Consoante o c_type, continuar a recuperação da mensagem original */
  int datasize, keysize, nkeys, i;
  void* data;
  char* key;
  char **keys;
  switch(msg->c_type){
    case CT_RESULT:
      msg->content.result = ntohl(*(int *) msg_buf);
      break;

    case CT_VALUE:
      //vai buscar o datasize ao msg_buf
      datasize = ntohl(*(int *) msg_buf);
      
      //incrementa o msg_buf
      msg_buf += _INT;
      
      if(datasize != 0){
         //aloca memoria para a data
      	 data = (void*)malloc(datasize);
      	 if(data==NULL){return NULL;}
	       //vai buscar a data
         memcpy(data, msg_buf, datasize);

         //cria um novo data e coloca-o no content
         msg->content.data = data_create2(datasize,data);

         free(data);
      }
      
      else{ 
    	  msg->content.data = (struct data_t *) malloc(sizeof(struct data_t));
    	  msg->content.data->datasize = 0;
    	  msg->content.data->data = NULL;
      }

      break;

    case CT_KEY:
      //vai buscar o keysize ao msg_buf
      keysize = ntohs(*(short *) msg_buf);
      
      //incrementa o msg_buf
      msg_buf += _SHORT;
      
      //aloca memoria para a key
      key = (char *) malloc(keysize + 1);
      if(key==NULL){return NULL;}

      //vai buscar a key
      memcpy(key, msg_buf, keysize);
      key[keysize] = '\0';
      
      //coloca uma copia da key no content
      msg->content.key = key;
      break;

    case CT_KEYS:
      //vai buscar o nkeys ao msg_buf
      nkeys = ntohl(*(int *) msg_buf);

      //cria a keys
      keys = (char **) malloc(sizeof(char *) * (nkeys + 1));
      if(keys==NULL){return NULL;}

      //incrementa o msg_buf
      msg_buf += _INT;

      //ciclo que vai criar as nkeys keys
      for(i = 0; i < nkeys; i++){
        //vai buscar o keysize ao msg_buf
        keysize = ntohs(*(short *) msg_buf);
        
        //incrementa o msg_buf
        msg_buf += _SHORT;
        
        //aloca memoria para a key
        key = (char *) malloc(keysize + 1);
        if(key==NULL){return NULL;}

        //vai buscar a key
        memcpy(key, msg_buf, keysize);
        key[keysize] = '\0';
        
        //coloca a key na keys
        keys[i] = key;
        
        //incrementa o msg_buf
        msg_buf += keysize;
      }

      keys[nkeys] = NULL;
      //coloca a keys no content
      msg->content.keys = keys;
      break;

    case CT_ENTRY:
      //vai buscar o keysize ao msg_buf
      keysize = ntohs(*(short *) msg_buf);
      
      //incrementa o msg_buf
      msg_buf += _SHORT;

      //aloca memoria para a key
      key = (char *) malloc(keysize + 1);
      if(key==NULL){return NULL;}
      
      //vai buscar a key
      memcpy(key, msg_buf, keysize);
      key[keysize] = '\0';

      //incrementa o msg_buf
      msg_buf += keysize;

      //vai buscar o datasize ao msg_buf
      datasize = ntohl(*(int *) msg_buf);
      
      //incrementa o msg_buf
      msg_buf += _INT;
      
      //aloca memoria para a data
      data = (void*)malloc(datasize);
      if(data==NULL){return NULL;}
      
      //vai buscar a data
      memcpy(data, msg_buf, datasize);
      
      //cria um novo entry e coloca-o no content
      struct data_t *dataNew = data_create2(datasize,data);
      msg->content.entry = entry_create(key, dataNew);
      free(key);
      free(data);
      data_destroy(dataNew);
      break;
  }

  return msg;
}
