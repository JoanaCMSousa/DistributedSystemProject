/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "list-private.h"

/* Cria uma nova lista. Em caso de erro, retorna NULL.
 */
struct list_t *list_create(){

	struct list_t *lista;
	lista = (struct list_t *) malloc(sizeof(struct list_t));
	//verifica se o malloc foi bem sucedido
 	if(lista == NULL){ return NULL;}
	
	//inicia a lista com tamanho zero e um node nulo
	lista -> size = 0;
	lista -> node = NULL;
	return lista;
}

/* Elimina uma lista, libertando *toda* a memoria utilizada pela
 * lista.
 */
void list_destroy(struct list_t *list){
	struct node_t *node;
	if(list != NULL){
		node = list -> node;
		//elimina todos os nos da lista
		while(node != NULL){
			node = list -> node -> next;
			node_destroy(list->node);
			list-> node = node;	
		}
		free(list);
	}
}

/* Adiciona uma entry na lista. Como a lista deve ser ordenada, 
 * a nova entry deve ser colocada no local correto.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_add(struct list_t *list, struct entry_t *entry){
	
	//verifica se a lista e a entry sao validas
	if(list == NULL || entry == NULL){return -1;}

	//caso nao tenha nos
	if(list-> node == NULL){
		list->node = node_create(entry);
		if(list->node == NULL){return -1;}

		list-> size = 1;
		return 0;
	}

	//primeiro no
	if(strcmp(((list->node)->entry)->key, entry->key) < 0){

		struct node_t *aux;
		aux = node_create(entry);

		if(aux == NULL){return -1;}

		aux->next = list->node;
		list->node = aux;
		list->size = (list->size) + 1;

		return 0;
		
	}

	else if(strcmp(list->node->entry->key, entry->key) == 0){
		
		entry_destroy(list->node->entry);
		list->node->entry = entry_dup(entry);
		return 0;
	}

	struct node_t *curNode;//current Node
	struct node_t *prevNode;//previous Node

	curNode = list->node->next;
	prevNode = list->node;

	while(curNode != NULL){

		//<0 node1 menor que node2
		if(strcmp(entry->key, curNode->entry->key) < 0){
			prevNode = curNode;
			curNode = curNode->next;
		}		
		
		//>0 node1 maior que node2
		else if(strcmp(entry->key, curNode->entry->key) > 0){
			struct node_t *aux;
			aux = node_create(entry);

			if(aux == NULL){ return -1;}

			prevNode->next = aux;
			aux->next = curNode;
			list->size = (list->size) +1;
			return 0;
		}
		
		else{//caso sejam iguais
			entry_destroy(curNode->entry);
			curNode->entry = entry_dup(entry);
			return 0;
		}
			
	}

	prevNode->next = node_create(entry);
	//verifica se foi criado o novo no
	if(prevNode->next == NULL){ return -1;}

	list->size = (list->size) +1;
	return 0;
}

/* Elimina da lista um elemento com a chave key. 
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_remove(struct list_t *list, char* key){

	//verificar se a key e a lista sao validas
	if(list == NULL || key == NULL){return -1;}
	//verifica se o primeiro no eh o correspondente ah key
	struct node_t *aux;
	aux = list-> node;
	if(strcmp(aux->entry->key, key) == 0){
		list->node = list->node->next;
		node_destroy(aux);
		list-> size = (list->size) - 1;
		return 0;
	}
	//se nao for o primeiro, procura nos seguintes
	struct node_t *prevNode;//previous
	struct node_t *curNode;//current
	prevNode = list->node;
	curNode = list->node->next;
	while(curNode != NULL){
		if(strcmp(curNode->entry->key, key) == 0){
			prevNode->next = curNode->next;
			node_destroy(curNode);
			list-> size = (list->size) - 1;
			return 0;
		}
		prevNode = curNode;
		curNode = curNode->next;
	}
	//se nao encontrar, devolve -1
	return -1;
}

/* Obtem um elemento da lista que corresponda à chave key. 
 * Retorna a referência do elemento na lista (ou seja, uma alteração
 * implica alterar o elemento na lista). 
 */
struct entry_t *list_get(struct list_t *list, char *key){

	//verifica se a key e a list sao validas
	if(list == NULL || key == NULL){return NULL;}

	struct node_t *aux;
	
	//verifica se a lista tem algum node
	if(list->node == NULL){return NULL;}

	aux = list->node;
	while(strcmp((aux->entry)->key,key) != 0){

		if(aux->next != NULL)
			aux = aux->next;

		else//caso a chave nao seja encontrada
			return NULL;
	}

	return aux-> entry;
}

/* Retorna o tamanho (numero de elementos) da lista 
 * Retorna -1 em caso de erro.  */
int list_size(struct list_t *list){
	//verifica se a lista eh valida
	if(list == NULL){return -1;}
	return list -> size;
}

/* Devolve um array de char * com a cópia de todas as keys da 
 * tabela, e um último elemento a NULL.
 */
char **list_get_keys(struct list_t *list){
	
	//verifica se a lista eh valida
	if(list == NULL){return NULL;}

	int i = 0;
	struct node_t *aux;
	aux = list -> node;

	char **keys;
	keys = (char **) malloc(sizeof(char *) * ((list-> size) + 1));
	
	//verifica se o malloc foi bem sucedido
	if(keys == NULL){ return NULL; }
	
	while(aux != NULL){
		keys[i] = (char *) malloc(sizeof(char) * ((strlen(aux-> entry-> key)) + 1));
		//verificar eh null
		memcpy(keys[i], aux-> entry-> key, (strlen(aux-> entry-> key) + 1));
		aux = aux-> next;
		i++;
	}

	//coloca o ultimo elemento a null
	keys[i] = NULL;
	return keys;
}

/* Liberta a memoria reservada por list_get_keys.
 */
void list_free_keys(char **keys){

	//verifica se as keys sao validas
	if(keys == NULL){return;}

	int i = 0;

	while(keys[i] != NULL){
		free(keys[i]);
		i++;
	}
	
	free(keys);

}

/* Cria um novo no. Em caso de erro, retorna NULL.
 */
struct node_t *node_create(struct entry_t *entry){

	struct node_t *new_node;
	
	//verifica se a entry eh valida
	if(entry == NULL){ return NULL; }
	
	new_node = (struct node_t *) malloc(sizeof(struct node_t));
	
	//verifica se o malloc foi bem sucedido
	if(new_node == NULL){return NULL;}

	new_node->entry = entry_dup(entry);
	new_node->next = NULL;
	return new_node;
}

/* Elimina um no, libertando toda a memoria utilizada pelo
 * mesmo
 */
void node_destroy(struct node_t *node){

	if(node != NULL){
		entry_destroy(node-> entry);
		free(node);
	}	

}
