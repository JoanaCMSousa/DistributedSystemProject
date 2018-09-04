#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "entry.h"
/*
 Nuno Narciso Carreiro nº38828
 Ruben Narciso Pavao nº 36011
 Grupo nº 9
*/

/* Funcao que cria um novo par chave-valor (isto e, que inicializa
* a estrutura e aloca a memoria necessaria).
*/
struct entry_t *entry_create(char *key, struct data_t *data){

	struct entry_t *e;
	e = (struct entry_t *) malloc(sizeof(struct entry_t));
	//alocar espaco para a estrutura entry_t
	
	if(e == NULL)
		return NULL;
	
	e -> key = key;
		
	e -> timestamp = 0; //Primeiro projeto timestamp a 0
	
	e -> value = data;

	return e;

}

/* Funcao que destroi um par chave-valor e liberta toda a memoria.
*/
void entry_destroy(struct entry_t *entry){

	free( entry -> key );

	data_destroy( entry -> value );

	free( entry );
}

/* Funcao que duplica um par chave-valor. */
struct entry_t *entry_dup(struct entry_t *entry){
	
	struct entry_t *dup;
	dup = (struct entry_t *) malloc(sizeof(struct entry_t));
	if(dup == NULL)
		return NULL;
	
	//strdup ja faz o malloc
	dup->key = strdup(entry-> key);
	
	if(dup->key == NULL)
		return NULL;

	dup -> timestamp = entry -> timestamp;

	//data_dup ja aloca a memoria
	dup -> value = data_dup(entry->value);
	if(dup -> value == NULL)
		return NULL;
	
	return dup;


}


