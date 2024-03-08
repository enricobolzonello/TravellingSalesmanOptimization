#include "heuristics.h"

ERROR_CODE h_greedy(instance* inst, int starting_node){
    if(starting_node >= inst->nnodes || starting_node < 0){
        return UNAVAILABLE;
    }

    // probably we will move it to somewhere else
    inst->solution_path = calloc(inst->nnodes, sizeof(int));

    int* visited = (int*)calloc(inst->nnodes, sizeof(int));

    int curr = starting_node;
    visited[curr] = 1;

    struct utils_clock c = utils_startclock();
    
    double sol_cost = 0;
    while(true){
        // check that we have not exceed time limit
        double ex_time = utils_timeelapsed(c);
        if(ex_time > inst->options_t.timelimit){
            free(visited);
            return DEADLINE_EXCEEDED;
        }

        // identify minimum distance from the current node
        int min_idx = -1;
        double min_dist = __DBL_MAX__;
        double temp;
        for(int i=0; i<inst->nnodes; i++){
            // skip iteration if it's already visited
            if(i == curr || visited[i]){
                continue;
            }

            temp = inst->costs[curr][i];
            if(temp < min_dist){
                min_dist = temp;
                min_idx = i;
            }
        }

        // save the edge
        inst->solution_path[curr] = min_idx;

        // we have visited all nodes
        if(min_idx == -1){
            inst->solution_path[curr] = starting_node;
            break;
        }

        curr = min_idx;
        visited[min_idx] = 1;
        sol_cost += min_dist;
    }

    sol_cost += inst->costs[curr][starting_node];
    inst->solution_cost = sol_cost;

    free(visited);

    return OK;
}

ERROR_CODE h_Greedy_iterative(instance* inst){
    int i;
    ERROR_CODE error;
    struct utils_clock c = utils_startclock();

    double best_cost = __DBL_MAX__;
    int* best_path = (int*) calloc(inst->nnodes, sizeof(int*));
    for(i=0; i<inst->nnodes; i++){
        double ex_time = utils_timeelapsed(c);
        if(ex_time > inst->options_t.timelimit){
            return DEADLINE_EXCEEDED;
        }

        log_info("starting greedy with node %d\n", i);
        error = h_greedy(inst, i);
        if(error != OK){
            log_error("code %d\n", error);
            break;
        }

        if(inst->solution_cost < best_cost){
            log_info("found new best, node %d\n", i);
            best_cost = inst->solution_cost;
            memcpy(best_path, inst->solution_path, inst->nnodes * sizeof(int));
        }
    }

    inst->best_solution_cost = best_cost;
    memcpy(inst->best_solution_path, best_path, inst->nnodes * sizeof(int));

    free(best_path);

    return OK;
}
