#ifndef HEUR_H_
#define HEUR_H_

#include "../tsp.h"

ERROR_CODE h_greedy(instance* inst, int starting_node);
ERROR_CODE h_Greedy_iterative(instance* inst);
double h_2opt(instance* inst);
void h_swap(int swap[2], double improvement, instance* inst);
ERROR_CODE h_2opt_iterative(instance* inst);

#endif
