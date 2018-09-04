#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
/*
 Nuno Narciso Carreiro nº38828
 Ruben Narciso Pavao nº 36011
 Grupo nº 9
*/

/* Funcao que cria um novo bloco de dados (isto e, que inicializa
* a estrutura e aloca a memoria necessaria).
*/
struct data_t *data_create(int size){

	struct data_t *d; // apontador para a estrutura
	d = (struct data_t *) malloc(sizeof(struct data_t));
	if(d == NULL)
		return NULL;
		
	d -> datasize = size;
	d -> data = malloc(size*sizeof(void *));
	if(d -> data == NULL)
		return NULL;
	return d;
}

/* Funcao identica a anterior, mas com uma assinatura diferente.
*/
struct data_t *data_create2(int size, void *data){
	
	struct data_t *d;
	d = (struct data_t *) malloc(sizeof(struct data_t));
	if(d == NULL)
		return NULL;
	d -> datasize = size;
	d -> data = data; 
	return d;

} 

/* Funcao que destroi um bloco de dados e liberta toda a memoria.
*/
void data_destroy(struct data_t *data){
	free(data->data);
	free(data);
}

/* Funcao que duplica um bloco de dados. Quando se criam duplicados
* e necessario efetuar uma COPIA dos dados (e nao somente alocar a
* memoria necessaria).
*/
struct data_t *data_dup(struct data_t *data){

	struct data_t *dup;
	dup = (struct data_t *) malloc(sizeof(struct data_t));
	if(dup == NULL)
		return NULL;
	dup -> datasize = data->datasize;
	dup -> data = malloc(data->datasize*sizeof(void *));
	if(dup -> data ==NULL)
		return NULL;
	memcpy(dup->data,data->data,dup->datasize);
	
	return dup;
	
}
