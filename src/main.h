#include "tsp.h"
#include "algorithms/heuristics.h"
#include "algorithms/metaheuristic.h"
#include "algorithms/matheuristics.h"

typedef struct {
    double cost;
    int* path;
    point* points;
    int nnodes;
    double execution_time;
} return_struct;

ERROR_CODE runAlg(instance* inst);
return_struct* webapp_run(const char* path, int seed, int time_limit, int alg);
