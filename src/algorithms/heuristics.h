#ifndef HEUR_H_
#define HEUR_H_

#include "../tsp.h"

ERROR_CODE h_greedy(instance* inst, int starting_node);
ERROR_CODE h_Greedy_iterative(instance* inst);
void h_2opt();

#endif
