#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "resp_parser.h"


int get_array_len(char* data)
{
    int len = 0;
    if (data[0] != '*') {
        printf("The data is not a a valid RESP array!\n");
        return NULL;
    }

    data++;
    while(*data != '\r') {
        len = (len*10)+(*data - '0');
        data++;
    }

    return len;
}


int get_len_element(char type, char** data)
{
    int len = 0;
    if (**data != type) {
        printf("Invalid data for type %c!\n", type);
        return -1;
    }

    (*data)++;
    while(**data != '\r') {
        len = (len*10)+(**data - '0');
        (*data)++;
    }

    return len;
}

struct bulk_string parse_bulk_string(char** data)
{
    struct bulk_string bulk_str;

    int len = get_len_element(BULK_STRING, data);
    
    // data now points to the first character in the string to be parsed
    (*data) += 2;

    char* buffer = malloc(len + 1);

    if (buffer) {
        memcpy(buffer, *data, len);
        buffer[len] = '\0';
    }

    bulk_str.len = len;
    bulk_str.data = buffer;

    *data += len + 2;

    return bulk_str;
}

struct array_element* parse_array(char* data)
{
    int len = get_len_element(ARRAY, &data);

    data += 2;

    struct array_element* elements = 
        (struct array_element*)malloc(sizeof(struct array_element) * len);
    
    for (int i = 0; i < len; i++) {
        if (data[0] == '$') {
            elements[i].type = BULK_STRING;
            struct bulk_string* str = (struct bulk_string*)malloc(sizeof(struct bulk_string));
            *str = parse_bulk_string(&data);
            elements[i].data = str;
            
        }

    }

    return elements;
}

char* build_response(struct array_element* command, int len)
{

    struct bulk_string first = *(struct bulk_string*)command[0].data;
    char* buffer = (char*)malloc(sizeof(char)*1024);

    for (int i = 0; i < first.len; i++) {
        first.data[i] = tolower(first.data[i]);
    }

    if (strcmp(first.data, "echo") == 0) {
        sprintf(buffer, "$%d\r\n%s\r\n", first.len, first.data);
    }

    else if (strcmp(first.data, "ping")) {
        sprintf(buffer, "+PONG\r\n");
    }

    return buffer;
}