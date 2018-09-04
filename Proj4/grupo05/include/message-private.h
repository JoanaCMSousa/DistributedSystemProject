/*Grupo 05
  Joana Sousa no47084, Joao Leal no46394,
  Liliana Delgadinho no43334*/

#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#define _SHORT 2
#define _INT 4
#define OC_RT_ERROR	99
#define OC_HELLO	60
#define OC_DONE		70
#define OC_SERVER	80

#include "table-private.h" /* For table_free_keys() */
#include "message.h"

void print_msg(struct message_t *msg);

#endif
