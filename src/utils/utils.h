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

#include "errors.h"


#define COLOR_BOLD  "\033[1m"
#define COLOR_OFF   "\033[m"

#define MAX_COORDINATE 10000
#define MIN_COORDINATE -10000

#define TSP_RAND() ( ((double)rand() / RAND_MAX) * (MAX_COORDINATE - MIN_COORDINATE) + MIN_COORDINATE )

#define NOT_CONNECTED -1.0f

static char* algs_string[4];

typedef struct {
    double x;
    double y;
} point;

struct utils_clock{
    bool started;
    clock_t starting_time;
};


bool utils_file_exists(const char *filename);
bool utils_invalid_input(int i, int argc, bool* help);
struct utils_clock utils_startclock(void);
double utils_timeelapsed(struct utils_clock c);
void utils_plotname(char* buffer, int buffersize);
void utils_format_title(char *fname, int alg);

#endif
