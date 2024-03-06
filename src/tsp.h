#ifndef TSP_H_
#define TSP_H_

#include "utils/utils.h"
#include "utils/plot.h"

typedef struct {
    int timelimit;              // time limit of the algorithm (in seconds)
    int seed;                   // seed for random generation, if not set by the user, defaults to current time
    bool graph_random;          // flag to indicate wheter the graph is randomly generated
    bool graph_input;           // flag to indicate wheter the graph is from the input file
    char* inputfile;            // input file path
} options;

typedef struct {
    options options_t;

    int nnodes;                 // number of nodes to be generated randomly
    point* points;              // dynamic array of points
} instance;


void tsp_parse_commandline(int argc, char** argv, instance* inst);
void tsp_generate_randompoints(instance* inst);
void tsp_plot_points(instance* inst, char* name);
void tsp_free_instance(instance *inst);

void tsp_read_input(instance* inst);

#endif
