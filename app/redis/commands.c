
#include "commands.h"
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

    long expiry = NULL;

    if (len > 5) {
        struct bulk_string* px = (struct bulk_string*)elements[4].data;
        expiry = strtol(px->data, NULL, 10);
    }

    struct bulk_string* key = (struct bulk_string*)elements[1].data;
    struct bulk_string* value = (struct bulk_string*)elements[2].data;

    char* key_copy = (char*)malloc(key->len + 1);
    char* value_copy = (char*)malloc(value->len + 1);

    memcpy(key_copy, key->data, key->len);
    key_copy[key->len] = '\0';

    memcpy(value_copy, value->data, value->len);
    value_copy[value->len] = '\0';


    ht_set(memory, key_copy, value_copy, expiry);

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
    char* buffer;

    if (value != NULL) {
        buffer = (char*)malloc(sizeof(char)*BUFFER_SIZE);
        sprintf(buffer, "$%d\r\n%s\r\n", strlen(value), value);
    }
    else {
        char* response = "$-1\r\n";
        buffer = (char*)malloc(sizeof(char)*strlen(response));
        strcpy(buffer, response);
    }


    return buffer;
}