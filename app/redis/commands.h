#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "resp_parser.h"
#include "hash_table/hash_table.h"

#define PING 1
#define ECHO 2
#define SET  3
#define GET  4

char* redis_ping();
char* redis_echo(struct array_element* elements, int len);
char* redis_set(hash_table* memory, struct array_element* elements, int len);
char* redis_get(hash_table* memory, struct array_element* elements, int len);

#endif