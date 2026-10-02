#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "types.h"
#include "../hash_table/hash_table.h"


char* redis_ping();
char* redis_echo(struct array_element* elements, int len);
char* redis_set(hash_table* memory, struct array_element* elements, int len);
char* redis_get(hash_table* memory, struct array_element* elements, int len,
                long received_at);

#endif