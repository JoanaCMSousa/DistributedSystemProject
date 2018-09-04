#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entry.h"
#include "list.h"
#include "list-private.h"
#include "data.h"
#include "table-private.h"

/*
 Nuno Narciso Carreiro nº38828
 Ruben Narciso Pavao nº 36011
 Grupo nº 9
*/

/* Função para criar/inicializar uma nova tabela hash, com n 
 * linhas(n = módulo da função hash) 
 */ 
struct table_t *table_create(int n){

	struct table_t *table;
	//Alocar espaco para a table
	table=(struct table_t*) malloc(sizeof(struct table_t));
	
	if(table==NULL)
		return NULL;
	
	//Numero de buckets que a table contem
	table->size = n;
	//Alocar espaco para o "array" das listas
	table->buckets=(struct list_t**) malloc (table->size * sizeof(struct list_t*));
	
	if(table->buckets==NULL)
		return NULL;
	
	//Alocar espaco para cada um dos buckets(listas vazias)
	int i;

	for(i=0;i<n;i++)

		table->buckets[i]=list_create();

	return table;

}

/* Eliminar/desalocar toda a memoria 
 */ 
void table_destroy(struct table_t *table){
	
	//Para cada bucket destruir a lista correspondente
	int i;
	
	for(i=0; i < table->size ;i++)
		
		list_destroy(table->buckets[i]);

	free(table->buckets);

	free(table);

}

/**
Funcao Hash
-Para chaves <= 5 caracteres soma-se o valor ASCII de 
todos os caracteres da chave e depois calcula o resto da divisao pelo table->size;
-Para chaves maiores que 5 soma-se o valor ASCII dos primeiros 3 
caracteres da chave e dos 2 ultimos e depois calcula o resto da divisao dessa soma por table->size
**/
static int hash (char *key , struct table_t *table){

	int hash=0;
	
	//Caso a chave seja menor ou igual a 5 caracteres
	if(strlen(key)<=5){
		int i;

		for(i=0;i<strlen(key);i++){
	
			hash+=key[i];
	
		}
		hash = hash % table->size;

		return hash;
	}
	//Caso contrario
	else{
		//Confirmar posicao final
		hash+=key[0]+key[1]+key[2]+key[strlen(key)-1]+key[strlen(key)-2];
		
		hash = hash % table->size;
		
		return hash;
	}
}


/* Funcao para adicionar um elemento na tabela. 
 * A função vai copiar a key (string) e os dados num 
 * espaco de memoria alocado por malloc(). 
 * Se a key ja existe, vai substituir essa entrada 
 * pelos novos dados. 
 * Devolve 0 (ok) ou -1 (out of memory) 
 */ 
int table_put(struct table_t *table, char *key, struct data_t *data){

	struct entry_t *temp;
	
	struct data_t *dtemp=data_dup(data);
	
	char *tkey= strdup(key);
	//funcao hash para saber em qual dos buckets vai se colocar a entry
	int h = hash(key, table);
	//Alocar memoria para a nova entry
	temp = entry_create(tkey,dtemp);
	
	if(temp==NULL)
		return -1;
	
	//Adiciona entry no bucket(lista)
	return list_add ( table->buckets[h] , temp);

}

/* Funcao para adicionar um elemento na tabela caso a chave 
 * associada ao elemento nao exista na tabela. Caso a chave 
 * exista, retorna erro. 
 * A função vai copiar a key (string) e os dados num 
 * espaco de memoria alocado por malloc(). 
 * Devolve 0 (ok) ou -1 (chave existe ou out of memory) 
 */ 
int table_conditional_put(struct table_t *table, char *key, struct
data_t *data){

	int h = hash(key, table);
	//Confirma se a chave dada nao contem nenhuma entry
	if(list_get(table->buckets[h],key)!=NULL)

		return -1;
		
	//Nova key chama a funcao tipica: table_put
	
	return table_put(table,key,data);

}

/* Funcao para obter um elemento da tabela. 
 * O argumento key indica a chave da entrada da tabela. 
 * A função aloca memoria para armazenar uma *COPIA* 
 * dos dados da tabela e retorna este endereco. 
 * O programa a usar essa funcao deve 
 * desalocar (utilizando free()) esta memoria. 
 * Em caso de erro, devolve NULL 
 */ 
struct data_t *table_get(struct table_t *table, char *key){

	int h = hash(key, table);
	
	struct entry_t *temp;
	
	struct data_t *data;
	//Caso a key nao exista devolve NULL
	if(!list_get(table->buckets[h], key))
		return NULL;
		
	//Cria uma entry temporaria para extrair o value(data)
	temp = entry_dup( list_get ( table -> buckets[h], key));
	//Copia a data 
	data = data_dup(temp->value);
	//destroi a entry auxiliar
	entry_destroy(temp);
	
	return data;


}


/* Funcao para remover um elemento da tabela. Vai desalocar 
 * toda a memoria alocada na respetiva operação table_put(). 
 * Devolve: 0 (ok), -1 (key not found) 
 */ 
int table_del(struct table_t *table, char *key){
	
	int h = hash(key, table);
	
	return list_remove( table -> buckets[h] , key);

}

/* Devolve o numero de elementos da tabela. 
 */ 
int table_size(struct table_t *table){

	int sum=0;

	int i;
	//percorre todas as listas somando os seus respectivos tamanhos
	for(i=0;i<table->size;i++)

		sum+= list_size(table->buckets[i]);
	
	return sum;

}

/* Devolve um array de char * com a copia de todas as 
 * keys da tabela, e um ultimo elemento a NULL. 
 */ 
char **table_get_keys(struct table_t *table){

	char **keys,**aux;

	keys = malloc( (table_size(table)+1) * sizeof(char *) );
	
	if(keys==NULL)
		return NULL;
	
	
	int i,j,temp=0;
	//Para cada bucket i da table
	for(i=0;i < table->size ;i++){
	
		aux = list_get_keys(table->buckets[i]);
		
		//Para cada entry na lista desse bucket i
		for(j=0; j < list_size (table->buckets[i]) ;j++){
			//Colocar cada key no buffer de todas as keys da table(nao da lista)
            		keys[temp] = strdup(aux[j]);
            		
            		temp++;
		}
		//liberta as chaves da lista do bucket i
		if(table->buckets[i]->size>0)
			list_free_keys(aux);	
	}

	keys[temp]=NULL;
	return keys;

}

/* Desaloca a memoria alocada por table_get_keys() 
 */ 
void table_free_keys(char **keys){

	int i;
	//Enquanto houver um char* na posicao i do array.Faz o free
	for(i=0;keys[i];i++){

		free(keys[i]);

	}

	free(keys);


}
