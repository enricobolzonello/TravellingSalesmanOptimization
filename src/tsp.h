#include "utils.h"

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


void tsp_parse_commandline(int argc, char** argv, instance* inst);

void tsp_generate_randompoints(instance* inst);

void tsp_free_instance(instance *inst);

void tsp_read_input(instance* inst);