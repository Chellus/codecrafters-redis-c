#ifndef _GETMILLIS_H
#define _GETMILLIS_H

#include <sys/time.h>
#include <stdlib.h>

long currentMillis() {
    struct timeval tp;

    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 100000;
}

#endif