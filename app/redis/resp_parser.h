#ifndef _RESP_PARSER_H
#define _RESP_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "commands.h"

#define BULK_STRING '$'
#define NUMBER ':'
#define SIMPLE_STRING '+'
#define BOOLEAN '#'
#define DOUBLE ','
#define BIG_NUM '('
#define ARRAY '*'

#define BUFFER_SIZE 4096

struct array_element {
    int type;
    void* data;
};

struct bulk_string {
    int len;
    char* data;
};

int get_array_len(char*);
int get_len_element(char, char**);
struct bulk_string parse_bulk_string(char**);
struct array_element* parse_array(char*);
int get_command(struct array_element*, int);
#endif