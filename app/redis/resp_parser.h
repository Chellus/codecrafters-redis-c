#ifndef _RESP_PARSER_H
#define _RESP_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "types.h"

#define BULK_STRING '$'
#define NUMBER ':'
#define SIMPLE_STRING '+'
#define BOOLEAN '#'
#define DOUBLE ','
#define BIG_NUM '('
#define ARRAY '*'

#define BUFFER_SIZE 4096

int get_array_len(char*);
int get_len_element(char, char**);
struct bulk_string parse_bulk_string(char**);
struct array_element* parse_array(char*);
command_t get_command(struct array_element*, int);
#endif