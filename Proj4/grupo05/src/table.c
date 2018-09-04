/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
 
#include "table-private.h"
 
int key_hash(char *key, int l){
 
  /* Verificar se key é NULL */
	if(key == NULL){return -1;}
 
  /* l tem valor válido? */
	if(l < 0){return -1;}
 
	int soma, i;
	soma = 0;

	for(i = 0; i < strlen(key); i++)
		soma += key[i];
	 
	return soma % l;
}
 
struct table_t *table_create(int n) {
 
  /* n tem valor válido? */
	if(n <= 0){return NULL;}
 
  /* Alocar memória para struct table_t */
 
    struct table_t *new_table;
    new_table = (struct table_t *) malloc(sizeof(struct table_t));
 
    if(new_table == NULL){return NULL;}
 
 
  /* Alocar memória para array de listas com n entradas 
     que ficará referenciado na struct table_t alocada. 
 
     Inicializar listas.
 
     Inicializar atributos da tabela.
  */
 
    new_table-> table_hash = (struct list_t **) malloc(sizeof(struct list_t *) * n);
 
    if((new_table-> table_hash) == NULL){
      free(new_table);
      return NULL;
    }
 
    int i = 0;
 
    while( i < n){
        new_table->table_hash[i] = list_create();
        if(new_table->table_hash[i] == NULL){
            int j;
            for(j = 0; j < i; j++){
                list_destroy(new_table -> table_hash[j]);
            }
            free(new_table);
            return NULL;
        }       
        i++;
    }
 
    new_table -> size = n;
    new_table -> nElem = 0;
 
  return new_table;
}
 
void table_destroy(struct table_t *table) {
 
  /* table é NULL? 
 
     Libertar memória das listas.
 
     Libertar memória da tabela.
 
  */
 
    if(table != NULL){
         
        int i = 0;
 
        while(i < (table-> size)){
            list_destroy(table->table_hash[i]);
            i++;
        }
 
        free(table->table_hash);
        free(table);
    }
}
 
int table_put(struct table_t *table, char * key, struct data_t *value) {
 
  /* Verificar valores de entrada */
 
    if(table == NULL || key == NULL || value == NULL){return -1;}
 
  /* Criar entry com par chave/valor */
 
    struct entry_t *entry;
    entry = entry_create(key, value);
 
    if(entry == NULL){return -1;}
 
  /* Executar hash para determinar onde inserir a entry na tabela */
 
    int result;
    result = key_hash(key, table->size);
 
  /* Inserir entry na tabela */
    //subtrai o tamanho da lista para o caso de fazer uma atualizacao e nao um add
    table -> nElem -= list_size(table -> table_hash[result]);
    
    int i;
    i = list_add(table->table_hash[result], entry);
 
    if (i == -1){
      //volta a adicionar o tamanho da lista, pois a lista nao foi alterada
      table -> nElem += list_size(table -> table_hash[result]);
      return i;
    }

    //volta a adicionar o tamanho da lista
    table -> nElem += list_size(table -> table_hash[result]);
    entry_destroy(entry);

    return i;
 
}
 
int table_update(struct table_t *table, char * key, struct data_t *value) {
 
    //verifica os valores de entrada
    if(table == NULL || key == NULL || value == NULL){return -1;}
 
    int result;
    result = key_hash(key, table->size);
 
    if(list_get(table -> table_hash[result], key) == NULL){return -1;}

    //cria a nova entry
    struct entry_t *entry;
    entry = entry_create(key, value);
 
    if(entry == NULL){return -1;}

    //faz update
    result = list_add(table->table_hash[result], entry);

    if(result == -1){return -1;}

    entry_destroy(entry);

    return result;
 
}
 
struct data_t *table_get(struct table_t *table, char * key){

    //verifica os valores de entrada
    if(table == NULL || key == NULL){return NULL;}
 
    int result;
    result = key_hash(key, table->size);

    if(list_get(table -> table_hash[result], key) == NULL){return NULL;}

    struct entry_t *entry;
    entry = list_get(table -> table_hash[result], key);
 
    if(entry -> value == NULL){return NULL;}
 
    return data_dup(entry -> value);
 
}
 
int table_del(struct table_t *table, char *key){

    //verifica os valores de entrada
    if(table == NULL || key == NULL){return -1;}
 
    int result;
    result = key_hash(key, table->size);
 
    if(list_get(table -> table_hash[result], key) == NULL){return -1;}
 
    int i;
    i = list_remove(table -> table_hash[result], key);
 
    if (i == -1){return i;}

    table -> nElem -=1;
 
    return i;
 
}
 
/* Esta é dada! Ao estilo C! */
int table_size(struct table_t *table) {

  return table == NULL ? -1 : table-> nElem;

}
 
char **table_get_keys(struct table_t *table) {
	
  //verifica os valores de entrada
	if(table == NULL){return NULL;}

  //malloc para a keys
	char **keys;
	keys = (char **) malloc(sizeof(char *) * ((table -> nElem) + 1));

	if(keys == NULL){return NULL;}

  //vai buscar as keys
	int i, j, index_table;
	index_table = 0;

	for(i = 0; i < table->size; i++){
		char **aux;
		aux = list_get_keys(table->table_hash[i]);
		for(j = 0; aux[j] != NULL; j++){
			keys[index_table] = aux[j];
			index_table++;
		}
		
		free(aux);
	}

	keys[index_table] = NULL;

	return keys;
 
}
 
void table_free_keys(char **keys) {
 
    list_free_keys(keys);
 
}
