#ifndef UTILS_H_
#define UTILS_H_

#include <time.h>
#include <string.h>
#include <sys/stat.h> 
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>


#define COLOR_BOLD  "\033[1m"
#define COLOR_OFF   "\033[m"

typedef struct {
    double x;
    double y;
} point;

typedef struct {
    time_t timelimit;
    int seed;
    char* inputfile;
} options;

typedef struct {
    options options_t;

    int nnodes;
    point* points;
} instance;


void parse_commandline(int argc, char** argv, instance* inst);
bool file_exists (const char *filename);

#endif
