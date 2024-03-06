#ifndef PLOT_H_
#define PLOT_H_

#include "utils.h"

typedef FILE* PLOT;

PLOT plot_open(char* title);
void plot_point(PLOT plot, point* p);
void plot_tofile(PLOT plot, char* filename);
void plot_args(PLOT plot, char* args);
void plot_free(PLOT plot);

#endif
