#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "entry.h"

/*
 Nuno Narciso Carreiro nº38828
 Ruben Narciso Pavao nº 36011
 Grupo nº 9
*/

struct list_t {
	struct node_t *head;
  	int size;
};

struct node_t {
	struct entry_t *entry; 
	struct node_t *next; //aponta para o proximo node
	struct node_t *previous; //aponta para o node anterior
};

#endif

