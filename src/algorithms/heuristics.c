#include "heuristics.h"

// if the time limit is exceeded, do we want to just return the value obtained 
// until that point or we do not return anything?
ERROR_CODE h_greedy(instance* inst, int starting_node){
    if(starting_node >= inst->nnodes || starting_node < 0){
        return UNAVAILABLE;
    }

    int* visited = (int*)calloc(inst->nnodes, sizeof(int));

    int curr = starting_node;
    visited[curr] = 1;

    clock_t begin = clock();
}
