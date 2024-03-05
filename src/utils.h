#ifndef UTILS_H_
#define UTILS_H_

#include <time.h>

typedef struct {
    double x;
    double y;
} point;

typedef struct {
    time_t timelimit;
    int seed;
} options;

typedef struct {
    options options_t;

    int nnodes;
    point* points;
    char* inputfile;
} instance;

#endif