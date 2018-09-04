#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "entry.h"
#include "list.h"
#include "list-private.h"
#include "data.h"

/*
 Nuno Narciso Carreiro nº38828
 Ruben Narciso Pavao nº 36011
 Grupo nº 9
*/


/* Cria uma nova lista. Em caso de erro, retorna NULL.
*/
struct list_t *list_create(){

	struct list_t *list;

	list = (struct list_t *) malloc(sizeof(struct list_t));
	
	if(list == NULL)
		return NULL;
	
  	list -> head = NULL;
	
	list->size=0;

	return list;
}

/* Elimina uma lista, libertando *toda* a memoria utilizada pela
* lista.
* Retorna 0 (OK) ou -1 (erro)
*/

int list_destroy(struct list_t *list){

	struct node_t *current;
	
	struct node_t *delete;

	if(list_size(list)==0){


		free(list);
	
	}
	else{
		//current aponta para a cabeca da lista
		current=list->head;
	
		//enquanto houver um node a seguir
		while(current->next){
	
			//atribuir o node actual ao node de destruicao
			delete=current;
	
			//node actual aponta para o seguinte
			current=current->next;
			//destroi a entrada do node de destruicao
			entry_destroy(delete->entry);

			//liberta a memoria do node de destruicao
			free(delete);
		//fim do while
		}
		entry_destroy(current->entry);
		free(current);
		free(list);
	}

}



/* Adiciona uma entry na lista. Como a lista deve ser ordenada,
* a nova entry deve ser colocada no local correto.
* Retorna 0 (OK) ou -1 (erro)
*/
int list_add(struct list_t *list, struct entry_t *entry){
	

	if(list == NULL)
		return -1;

	struct node_t *newnode;
	
	struct node_t *current;
		
	current=list->head;

	newnode= (struct node_t*) malloc(sizeof(struct node_t));

	if(newnode == NULL)
		return -1;

	newnode-> entry = entry;
	
	//Se a lista foi acabada de criar
	if(list_size(list)==0){
		
		newnode -> previous = NULL;
		newnode -> next = NULL;
		
		list->head = newnode;
		
		list->size++;
		
		return 0;
	}
	
	//Se a key dada ja existe na lista
	//Entao actualiza a entry
	if(list_get(list,entry->key)!=NULL){
		
		struct entry_t *delete;
		struct node_t *current;
		
		current=list->head;
		//Percorre a lista ate encontrar o node com a entrada
		while(strcmp(current->entry->key,entry->key)!=0){
			current=current->next;
		}
		//Destroy a entry antiga
		delete = current->entry;
		entry_destroy(delete);
		
		//current recebe a nova entry
		current->entry=entry;
		
		//newnode free que nao foi usado
		free(newnode);
		return 0;
	
	}
	//Insercao na head
	if(strcmp(current -> entry -> key, entry->key)<0){
	
			newnode -> next = current;
			newnode -> previous = NULL;
			
			current -> previous = newnode;
			
			list -> head = newnode;
			
			list-> size++;
			
			return 0;
	}
		
	//Casos restantes
	//Percorre a lista ate encontrar a posicao adequada da entry
	while(current -> next && strcmp(current-> next -> entry->key,entry->key)>0){
		
		current=current->next;
	}
		
	newnode -> next = current -> next;
	current -> next = newnode;
		
	newnode -> previous = current;
		
	list->size++;
	
	return 0;
}

/* Elimina da lista um elemento com a chave key.
* Retorna 0 (OK) ou -1 (erro)
*/
int list_remove(struct list_t *list, char *key){
	
	if(list == NULL)
		return -1;

	if(list->size == 0)
		return -1;

	struct node_t *current,*delete;

	current=list->head;

	
	//Remover a cabeca da lista
	if(strcmp(current->entry->key,key) == 0){
		
		delete = current;
		
		current = current -> next;
		if(current!=NULL)
			current ->previous = NULL;
		
		entry_destroy(delete -> entry);
		free(delete);
		
		list->head=current;
		list->size--;
                return 0;
	}
	
	//Restantes nodes
	while(current->next){
			
		current = current -> next;
		
		//Caso encontre a key na lista
		if(strcmp(current -> entry -> key,key) == 0){
				
			struct node_t *temp;
			delete = current;
				
			temp = delete -> previous;
			current = delete -> next;
			
			temp -> next = delete -> next;
			
			if(current!=NULL)
				current -> previous = temp;
			
				
			entry_destroy(delete-> entry);
			free(delete);
			
			list->size--;
			
			return 0;
			//fim do if
			}
			
	//fim do while	
	}
	//Caso nao exista a chave na lista
	return -1;
}


/* Obtem um elemento da lista com a chave key.
* Retorna a referencia do elemento na lista (ou seja, uma alteração
* implica alterar o elemento na lista).
*/
struct entry_t *list_get(struct list_t *list, char *key){

	//Se a lista for vazia
	if(list->size==0){
		
		return NULL;
	}

	struct node_t *current;

	current = list->head;
	
	//Lista so tem uma entry
	
	if(strcmp(list->head->entry->key,key)==0)
	
		return list->head->entry;
	
	
	
	//Casos restantes
	while(current->next){
		current = current->next;
		
		if(strcmp(current->entry->key, key)==0)
			
			return current->entry;
		
	}
	
	return NULL;

}


/* Retorna o tamanho (numero de elementos) da lista
* Retorna -1 em caso de erro)
*/
int list_size(struct list_t *list){

	return list->size;
}

/* Devolve um array de char* com a cópia de todas as keys da
* lista, com um ultimo elemento a NULL.
*/
char **list_get_keys(struct list_t *list){
	
	if(list->size==0)
		return NULL;
	
	char **keys;
	struct node_t *current;

	keys = malloc( (list_size(list)+1) * sizeof(char *) );
	
	current=list->head;
	
	int i=0;
	
	keys[i] = strdup(list->head->entry->key);
		
	while(current->next){
		
		current= current->next;
		++i;

		if(current->entry->key!=NULL)
		
			keys[i] = strdup(current->entry->key);
				
	}
	
	i++;
	keys[i]=NULL;
	
	return keys;
}


/* Liberta a memoria alocada por list_get_keys
*/
void list_free_keys(char **keys){

	int i;

	for(i=0;keys[i];i++){

		free(keys[i]);

	}
	free(keys);
	
}



