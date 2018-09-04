/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#ifndef _LISTPRIVATE_H
#define _LISTPRIVATE_H

#include "list.h"

/* Esta estrutura define o no para a lista
 */
struct node_t {

	struct entry_t *entry;
	struct node_t *next;
};

/* Esta estrutura define a lista ligada por nos 
 */
struct list_t {

	int size;
	struct node_t *node;
};

/* Cria um novo no. Em caso de erro, retorna NULL.
 */
struct node_t *node_create(struct entry_t *entry);

/* Elimina um no, libertando toda a memoria utilizada pelo
 * mesmo
 */
void node_destroy(struct node_t *node);


#endif
