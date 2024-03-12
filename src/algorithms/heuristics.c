#include "heuristics.h"

ERROR_CODE h_greedyutil(instance* inst, int starting_node, int* solution_path, double* solution_cost){
    if(starting_node >= inst->nnodes || starting_node < 0){
        return UNAVAILABLE;
    }

    int* visited = (int*)calloc(inst->nnodes, sizeof(int));

    int curr = starting_node;
    visited[curr] = 1;
    
    double sol_cost = 0;
    bool done = false;

    while(!done){
        // check that we have not exceed time limit
        double ex_time = utils_timeelapsed(inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               free(visited);
                return DEADLINE_EXCEEDED;
            }
        }

        // identify minimum distance from the current node
        int min_idx = -1;
        double min_dist = __DBL_MAX__;

        for(int i=0; i<inst->nnodes; i++){
            // skip iteration if it's already visited
            if(i != curr && visited[i] != 1){
                // update the minimum cost and its node
                double temp = inst->costs[curr][i];
                if(temp != NOT_CONNECTED && temp < min_dist){
                    min_dist = temp;
                    min_idx = i;
                }
            }
        }

        // save the edge
        solution_path[curr] = min_idx;

        if(min_idx == -1){
            // we have visited all nodes
            // close the path
            solution_path[curr] = starting_node;
            done = true;
        }else{
            // mark the node as visited and update the cost of the solution
            visited[min_idx] = 1;
            curr = min_idx;
            sol_cost += min_dist;
        }
    }

    // add last edge
    sol_cost += inst->costs[curr][starting_node];
    *(solution_cost) = sol_cost;

    free(visited);

    return OK;
}

ERROR_CODE h_Greedy(instance* inst){
    int* solution_path = calloc(inst->nnodes, sizeof(int));
    double solution_cost = __DBL_MAX__;

    log_info("running GREEDY");
    ERROR_CODE error = h_greedyutil(inst, inst->starting_node, solution_path, &solution_cost);

    tsp_update_best_solution(inst, solution_cost, solution_path);

    free(solution_path);
    return error;
}

ERROR_CODE h_Greedy_iterative(instance* inst){
    int* solution_path = calloc(inst->nnodes, sizeof(int));
    double solution_cost = __DBL_MAX__;

    for(int i=0; i<inst->nnodes; i++){
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                return DEADLINE_EXCEEDED;
            }
        }

        log_debug("starting greedy with node %d", i);
        ERROR_CODE error = h_greedyutil(inst, i, solution_path, &solution_cost);
        if(error != OK){
            log_error("code %d\n", error);
            break;
        }

        if(solution_cost < inst->best_solution_cost){
            log_info("found new best, node %d", i);
            inst->starting_node = i;
            tsp_update_best_solution(inst, solution_cost, solution_path);
        }
    }

    free(solution_path);

    return OK;
}

// TODO: change to pick the best swap
ERROR_CODE h_2opt(instance* inst){
    // because 2opt works on the best solution, but it may not be feasible
    int* solution_path = calloc(inst->nnodes, sizeof(int));
    memcpy(solution_path, inst->best_solution_path, inst->nnodes * sizeof(int));
    int solution_cost = inst->best_solution_cost;

    double best_cost = solution_cost;               // save the current cost
    int *prev = calloc(inst->nnodes, sizeof(int));      // save the path of the solution without 2opt
    for (int i = 0; i < inst->nnodes; i++) {
        prev[solution_path[i]] = i;
    }

    do {
        // see if it exceeds the time limit
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                log_debug("time limit exceeded");
                return DEADLINE_EXCEEDED;
            }
        }

        // update the best cost
        best_cost = solution_cost;
        log_debug("New best cost: %f", best_cost);

        for (int a = 0; a < inst->nnodes - 1; a++) {
            for (int b = a+1; b < inst->nnodes; b++) {
                int succ_a = solution_path[a]; //successor of a
                int succ_b = solution_path[b]; //successor of b

                // Skip non valid configurations
                if (succ_a == succ_b || a == succ_b || b == succ_a){
                    continue;
                }


                // Compute the delta. If < 0 it means there is a crossing
                double current_cost = inst->costs[a][succ_a] + inst->costs[b][succ_b];
                double swapped_cost = inst->costs[a][b] + inst->costs[succ_a][succ_b];
                double delta = swapped_cost - current_cost;
                if (delta < 0) {
                    //Swap the 2 edges
                    solution_path[a] = b;
                    solution_path[succ_a] = succ_b;
                    
                    //Reverse the path from the b to the successor of a
                    h_reverse_path(inst, b, succ_a, prev, solution_path);
                    
                    //update tour cost
                    solution_cost += delta;
                }
            }
        }
        
    }while(solution_cost < best_cost);
    
    tsp_update_best_solution(inst, solution_cost, solution_path);
    
    free(prev);
    return OK;
}

void h_reverse_path(instance *inst, int start_node, int end_node, int *prev, int* solution_path) {
    int currnode = start_node;
    while (1) {
        int node = prev[currnode];
        solution_path[currnode] = node;
        currnode = node;
        if (node == end_node) {
            break;
        }
    }

    for (int k = 0; k < inst->nnodes; k++) {
        prev[solution_path[k]] = k;
    }
}
