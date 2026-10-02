#ifndef _TYPES_H
#define _TYPES_H
struct array_element {
    int type;
    void* data;
};

struct bulk_string {
    int len;
    char* data;
};

enum command {
    PING,
    ECHO,
    SET,
    GET,
    RPUSH,
    LPUSH
};

typedef enum command command_t;
#endif