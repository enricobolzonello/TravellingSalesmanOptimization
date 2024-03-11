#ifndef HEUR_H_
#define HEUR_H_

#include "../tsp.h"

ERROR_CODE h_greedyutil(instance* inst, int starting_node);
ERROR_CODE h_Greedy(instance *inst);
ERROR_CODE h_Greedy_iterative(instance* inst);
double h_2opt(instance* inst);
void h_reverse_path(instance* inst, int start, int end, int* prev);
ERROR_CODE h_2opt_iterative(instance* inst);

#endif
