/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "table.h"

struct table_t{
	struct list_t **table_hash/* continuar definição */;
	int size; /* Dimensão da tabela */
	int nElem; /* numero de elementos na tabela */
};

int key_hash(char *key, int l);

#endif
