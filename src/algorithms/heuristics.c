#include "heuristics.h"

// TODO: variabili temporanee dentro al ciclo, non ha senso definirle fuori dal for se non mi serve il valore precedente
// nelle implementazioni moderne si risparmia tempo dato che vengono salvate in registri

ERROR_CODE h_greedyutil(instance* inst, int starting_node){
    int i;
    if(starting_node >= inst->nnodes || starting_node < 0){
        return UNAVAILABLE;
    }

    // TODO: initialize somewhere else, since greedy iterative gives memory leak
    inst->solution_path = calloc(inst->nnodes, sizeof(int));

    int* visited = (int*)calloc(inst->nnodes, sizeof(int));

    int curr = starting_node;
    visited[curr] = 1;
    
    double sol_cost = 0;
    int min_idx;
    double min_dist;
    double temp;
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
        min_idx = -1;
        min_dist = __DBL_MAX__;

        for(i=0; i<inst->nnodes; i++){
            // skip iteration if it's already visited
            if(i != curr && visited[i] != 1){
                // update the minimum cost and its node
                temp = inst->costs[curr][i];
                if(temp != NOT_CONNECTED && temp < min_dist){
                    min_dist = temp;
                    min_idx = i;
                }
            }
        }

        // save the edge
        inst->solution_path[curr] = min_idx;

        if(min_idx == -1){
            // we have visited all nodes
            // close the path
            inst->solution_path[curr] = starting_node;
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
    inst->solution_cost = sol_cost;

    free(visited);

    return OK;
}

ERROR_CODE h_Greedy(instance* inst){
    log_info("running GREEDY");
    ERROR_CODE error = h_greedyutil(inst, 0);

    tsp_update_best_solution(inst);

    return error;
}

ERROR_CODE h_Greedy_iterative(instance* inst){
    int i;
    ERROR_CODE error;

    int* best_path = (int*) calloc(inst->nnodes, sizeof(int*));
    for(i=0; i<inst->nnodes; i++){
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                return DEADLINE_EXCEEDED;
            }
        }

        log_debug("starting greedy with node %d", i);
        error = h_greedyutil(inst, i);
        if(error != OK){
            log_error("code %d\n", error);
            break;
        }

        if(inst->solution_cost < inst->best_solution_cost){
            log_info("found new best, node %d", i);
            tsp_update_best_solution(inst);
        }
    }

    free(best_path);

    return OK;
}

// TODO: change to pick the best swap
ERROR_CODE h_2opt(instance* inst){
    // because 2opt works on the best solution, but it may not be feasible
    memcpy(inst->solution_path, inst->best_solution_path, inst->nnodes * sizeof(int));
    inst->solution_cost = inst->best_solution_cost;

    double best_cost=inst->solution_cost;               // save the current cost
    int *prev = calloc(inst->nnodes, sizeof(int));      // save the path of the solution without 2opt
    for (int i = 0; i < inst->nnodes; i++) {
        prev[inst->solution_path[i]] = i;
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
        best_cost=inst->solution_cost;
        log_debug("New best cost: %f", best_cost);

        for (int a = 0; a < inst->nnodes - 1; a++) {
            for (int b = a+1; b < inst->nnodes; b++) {
                int succ_a = inst->solution_path[a]; //successor of a
                int succ_b = inst->solution_path[b]; //successor of b

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
                    inst->solution_path[a] = b;
                    inst->solution_path[succ_a] = succ_b;
                    
                    //Reverse the path from the b to the successor of a
                    h_reverse_path(inst, b, succ_a, prev);
                    
                    //update tour cost
                    inst->solution_cost += delta;
                }
            }
        }
        
    }while(inst->solution_cost < best_cost);
    
    tsp_update_best_solution(inst);
    
    free(prev);
    return OK;
}

void h_reverse_path(instance *inst, int start_node, int end_node, int *prev) {
    int currnode = start_node;
    while (1) {
        int node = prev[currnode];
        inst->solution_path[currnode] = node;
        currnode = node;
        if (node == end_node) {
            break;
        }
    }

    for (int k = 0; k < inst->nnodes; k++) {
        prev[inst->solution_path[k]] = k;
    }
}
