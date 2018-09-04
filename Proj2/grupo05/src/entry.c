/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "entry.h"

/* Função que cria um novo par {chave, valor} (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 */
struct entry_t *entry_create(char *key, struct data_t *data){

	struct entry_t *entry;

	//verifica se a key e a data sao validas	
	if(key == NULL || data == NULL){ return NULL; }

	entry = (struct entry_t *) malloc(sizeof(struct entry_t));
	
	//verifica se o malloc foi bem sucedido
	if(entry == NULL){ return NULL;}

	entry-> key = strdup(key);
	entry-> value = data_dup(data);

	return entry;

}

/* Função que destrói um par {chave-valor} e liberta toda a memória.
 */
void entry_destroy(struct entry_t *entry){

	if(entry != NULL){
		data_destroy(entry-> value);
		free(entry-> key);
		free(entry);
	}

}

/* Função que duplica um par {chave, valor}.
 */
struct entry_t *entry_dup(struct entry_t *entry){

	struct entry_t *entry_copy;

	//verifica se a entry eh valida
	if(entry == NULL){ return NULL; }

	char *key_copy;

	//verifica se o value e a key sao validos
	if(entry-> value == NULL || entry-> key == NULL){ return NULL; }

	entry_copy = (struct entry_t *) malloc(sizeof(struct entry_t));
	
	//verifica se o malloc foi bem sucedido
	if(entry_copy == NULL){ return NULL; }

	entry_copy-> value = data_dup(entry-> value);

	//se o value for null, liberta a entry
	if(entry_copy-> value == NULL){
		free(entry_copy);
		return NULL;
	}
	
	key_copy= (char *) malloc((strlen(entry-> key)) + 1);
	
	//verifica se o malloc foi bem sucedido
	//e liberta a entry caso nao seja
	if(key_copy == NULL){
		data_destroy(entry_copy-> value);
		free(entry_copy);
		return NULL;
	}

	memcpy(key_copy, entry-> key, (strlen(entry-> key)) + 1);
	entry_copy-> key = key_copy;

	return entry_copy;

}

