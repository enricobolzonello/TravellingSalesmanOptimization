#include "heuristics.h"

ERROR_CODE h_greedyutil(instance* inst, int starting_node){
    int i;
    if(starting_node >= inst->nnodes || starting_node < 0){
        return UNAVAILABLE;
    }

    // probably we will move it to somewhere else
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
                if(temp != -1.0f && temp < min_dist){
                    min_dist = temp;
                    min_idx = i;
                }
            }
        }

        // save the edge
        inst->solution_path[curr] = min_idx;

        // we have visited all nodes
        if(min_idx == -1){
            inst->solution_path[curr] = starting_node;
            done = true;
        }else{
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

    double best_cost = __DBL_MAX__;
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

        if(inst->solution_cost < best_cost){
            log_info("found new best, node %d", i);
            tsp_update_best_solution(inst);
        }
    }

    free(best_path);

    return OK;
}

double h_2opt(instance* inst) {
    int best_i, best_j, best_succ_i, best_succ_j;
    double best_delta = 0;

    // copy the current path to then perform the reversing
    int* prev = calloc(inst->nnodes, sizeof(int));
    memcpy(prev, inst->solution_path, inst->nnodes * sizeof(int));
    
    for(int i=0; i<inst->nnodes - 1; i++){
        for(int j=i+1; j<inst->nnodes; j++){

            int succ_i = inst->solution_path[i];    // successor of a
            int succ_j = inst->solution_path[j];    // successor of b

            if(succ_i != succ_j && i != succ_j && j != succ_i){
                double current_cost = inst->costs[i][succ_i] + inst->costs[j][succ_j];
                double swapped_cost = inst->costs[i][j] + inst->costs[succ_i][succ_j];
                double delta = swapped_cost - current_cost;
                if(delta < best_delta){
                    best_succ_i = succ_i;
                    best_succ_j = succ_j;

                    best_i = i;
                    best_j = j;
                    best_delta = delta;
                }
            }
        }
    }

    if(best_delta < 0){
        inst->solution_path[best_i] = best_j;
        inst->solution_path[best_succ_i] = best_succ_j;
        //h_swap(best_swap, improvement,inst);
        h_reverse_path(inst, best_j, best_succ_i, prev);

        inst->solution_cost += best_delta;
    }

    return best_delta;
}

ERROR_CODE h_2opt_iterative(instance* inst){
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

        for (int a = 0; a < inst->nnodes - 1; a++) {
            for (int b = a+1; b < inst->nnodes; b++) {
                int succ_a = inst->solution_path[a]; //successor of a
                int succ_b = inst->solution_path[b]; //successor of b

                // Skip non valid configurations
                // a1 == b1 never occurs because the edges are repsresented as directed. a->a1 then a1->b so it cannot be a->a1 b->a1
                if (succ_a == succ_b || a == succ_b || b == succ_a){
                    continue;
                }


                // Compute the delta. If < 0 it means there is a crossing
                double current_cost = inst->costs[a][succ_a] + inst->costs[b][succ_b];
                double swapped_cost = inst->costs[a][b] + inst->costs[succ_a][succ_b];
                double delta = swapped_cost - current_cost;
                if (delta < 0) {
                    //Swap the 2 edges
                    succ_a = inst->solution_path[a];
                    succ_b = inst->solution_path[b];
                    inst->solution_path[a] = b;
                    inst->solution_path[succ_a] = succ_b;
                    
                    //Reverse the path from minb to a1
                    h_reverse_path(inst, b, succ_a, prev);
                    
                    //update tour cost
                    inst->solution_cost += delta;
                }
            }
        }

        // If couldn't find a crossing, stop the algorithm
        if (inst->solution_cost < best_cost) {
            //Update best cost seen till now
            best_cost=inst->solution_cost;
            log_debug("%f\n", best_cost);
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
