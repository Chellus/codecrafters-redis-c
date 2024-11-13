#include "resp_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* redis_ping()
{
    return "+PONG\r\n";
}

char* redis_echo(struct array_element* elements, int len)
{
    // bad echo usage
    if (len != 2) {
        return NULL;
    }

    char* buffer = (char*)malloc(sizeof(char)*BUFFER_SIZE);
    struct bulk_string arg = *(struct bulk_string*)elements[1].data;
    sprintf(buffer, "$%d\r\n%s\r\n", arg.len, arg.data);

    return buffer;
}

char* redis_set(hash_table* memory, struct array_element* elements, int len)
{
    // bad set usage
    if (len < 3) {
        return NULL;
    }

    struct bulk_string arg1 = *(struct bulk_string*)elements[1].data;
    struct bulk_string arg2 = *(struct bulk_string*)elements[2].data;

    ht_set(memory, (char*)arg1.data, arg2.data);

    return "+OK\r\n";
}

char* redis_get(hash_table* memory, struct array_element* elements, int len)
{
    // bad get usage
    if (len != 2) {
        return NULL;
    }
    
    struct bulk_string arg1 = *(struct bulk_string*)elements[1].data;

    char* value = ht_get(memory, arg1.data);

    if (value == NULL) {
        return "$-1\r\n";
    }

    char* buffer = (char*)malloc(sizeof(char)*BUFFER_SIZE);

    sprintf(buffer, "$%d\r\n%s\r\n", strlen(value), value);

    return buffer;

}