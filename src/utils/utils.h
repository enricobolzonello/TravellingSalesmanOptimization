#ifndef UTILS_H_
#define UTILS_H_

/**
 * @file utils.h
 * @author Enrico Bolzonello (enrico.bolzonello@studenti.unidp.it)
 * @brief General utilities for TSP
 * @version 0.1
 * @date 2024-03-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <string.h>
#include <sys/stat.h> 
#include <errno.h>
#include <time.h>

#include "errors.h"


#define MAX_COORDINATE 5000
#define MIN_COORDINATE -5000

#define TSP_RAND() ( ((double)rand() / RAND_MAX) * (MAX_COORDINATE - MIN_COORDINATE) + MIN_COORDINATE )

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define utils_safe_free(pointer) utils_safe_memory_free((void **) &(pointer))

#define NOT_CONNECTED -1.0f

typedef struct {
    double x;
    double y;
} point;

typedef struct {
    double cost;
    int* path;
    int ncomp;
    int* comp;
}tsp_solution;

void utils_safe_memory_free (void ** pointer_address);
bool utils_file_exists(const char *filename);
bool utils_invalid_input(int i, int argc, bool* help);
void utils_startclock(struct timespec* c);
double utils_timeelapsed(struct timespec* c);
void utils_plotname(char* buffer, int buffersize);
void utils_format_title(char *fname, int alg);
void swap(int* a, int* b);

/**
 * @brief Initialize solution struct
 * 
 * @param nnodes number of nodes of the instance
 * @return tsp_solution 
 */
ERROR_CODE tsp_init_solution(int nnodes, tsp_solution* solution);

#endif
