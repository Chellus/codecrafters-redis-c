#include "getmillis.h"

#include <sys/time.h>
#include <stdlib.h>

long current_millis() {
    struct timeval tp;

    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000000;
}