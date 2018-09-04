#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "list.h"

struct table_t {
	int size; //Quantas buckets tem a table
	struct list_t **buckets; //Apontador que representa cada um dos buckets na forma de uma lista
};


/**
Funcao Hash
-Para chaves <= 5 caracteres soma-se o valor ASCII de 
todos os caracteres da chave e depois calcula o resto da divisao pelo table->size;
-Para chaves maiores que 5 soma-se o valor ASCII dos primeiros 3 
caracteres da chave e dos 2 ultimos e depois calcula o resto da divisao dessa soma por table->size
**/
static int hash (char *key , struct table_t *table);
#endif
