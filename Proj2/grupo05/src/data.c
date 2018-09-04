/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "data.h"

/* Função que cria um novo elemento de dados data_t e reserva a memória
 * necessária, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size){

	struct data_t *estrutura;

	//verifica se o size eh valido
	if(size <= 0){ return NULL;}
	estrutura = (struct data_t *) malloc(sizeof(struct data_t));

	//verifica se o malloc foi bem sucedido
	if(estrutura == NULL){ return NULL;}

	estrutura-> data =(void *) malloc(size);

	//verifica se o malloc foi bem sucedido
	//e liberta a estrutura caso nao seja
	if(estrutura-> data == NULL){
		free(estrutura);
		return NULL;
	}

	estrutura-> datasize = size;
	return estrutura;
}

/* Função idêntica à anterior, mas que inicializa os dados de acordo com
 * o parâmetro data.
 */
struct data_t *data_create2(int size, void *data){

	struct data_t *estrutura;

	//verifica se o size e a data sao validos
	if(size <= 0 || data == NULL) {return NULL;}

	estrutura = data_create(size);

	//verifica se a estrutura foi criada com sucesso
	if(estrutura == NULL){ return NULL;}

	memcpy(estrutura->data, data, size);
	estrutura-> datasize = size;
	return estrutura;

}

/* Função que destrói um bloco de dados e liberta toda a memória.
 */
void data_destroy(struct data_t *data){

	if(data != NULL){

		free(data-> data);
		free(data);
	}
}

/* Função que duplica uma estrutura data_t.
 */
struct data_t *data_dup(struct data_t *data){

	struct data_t *copy;

	//verifica se a data eh valida
	if(data == NULL || data->datasize <= 0) { return NULL; }

	void *copy_data;	

	//verifica se a data guardada em data eh valida
	if(data-> data == NULL) {return NULL;}
	
	copy = (struct data_t *) malloc(sizeof(struct data_t));

	//verifica se o malloc foi bem sucedido
	if(copy == NULL){ return NULL; }

	copy_data = (void *) malloc(data-> datasize);
	
	//verifica se o malloc foi bem sucedido
	//e liberta o copy caso nao seja
	if(copy_data == NULL){
		free(copy);
		return NULL;
	}

	memcpy(copy_data, data-> data, data-> datasize);
	copy-> data = copy_data;
	copy-> datasize = data-> datasize;

}
