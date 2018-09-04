#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#define _SHORT 2
#define _INT 4
#define OC_RT_ERROR	99

#include "table-private.h" /* For table_free_keys() */
#include "message.h"

void print_msg(struct message_t *msg);

#endif