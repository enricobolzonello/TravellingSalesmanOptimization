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

#define MAX_COORDINATE 10000
#define MIN_COORDINATE -10000

#define TSP_RAND() ( ((double)rand() / RAND_MAX) * (MAX_COORDINATE - MIN_COORDINATE) + MIN_COORDINATE )


bool file_exists (const char *filename);
bool invalid_input(int i, int argc, bool* help);

#endif
