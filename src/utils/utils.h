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

#include <time.h>
#include <string.h>
#include <sys/stat.h> 
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#define COLOR_BOLD  "\033[1m"
#define COLOR_OFF   "\033[m"

#define MAX_COORDINATE 10000
#define MIN_COORDINATE -10000

#define TSP_RAND() ( ((double)rand() / RAND_MAX) * (MAX_COORDINATE - MIN_COORDINATE) + MIN_COORDINATE )

typedef struct {
    double x;
    double y;
} point;


bool utils_file_exists (const char *filename);
bool utils_invalid_input(int i, int argc, bool* help);
void utils_print_error(char* message);
void utils_startclock();
int utils_timeelapsed();

#endif
