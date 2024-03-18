#include "tabusearch.h"


//================================================================================
// POLICIES
//================================================================================

ERROR_CODE tabu_fixed_policy(tabu_search* t, int value){
    if(t->policy != POL_FIXED){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    t->tenure = value;

    return OK;
}

ERROR_CODE tabu_dependent_policy(tabu_search* t){
    if(t->policy != POL_SIZE){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    t->tenure = (int)ceil((t->max_tenure + t->min_tenure)/2);

    return OK;
}

ERROR_CODE tabu_random_policy(tabu_search* t){
    if(t->policy != POL_RANDOM){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    t->tenure = (int)(rand() / RAND_MAX) * (t->max_tenure - t->min_tenure) + t->min_tenure;

    return OK;
}

ERROR_CODE tabu_linear_policy(tabu_search* ts){
    if(ts->policy != POL_LINEAR){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    // if we reached the maximum or minimum, we need to go the opposite direction
    if(ts->max_tenure == ts->tenure || ts->min_tenure == ts->tenure){
        ts->increment = !ts->increment;
    }

    // increment or decrement 
    if(ts->increment){
        ts->tenure++;
    }else{
        ts->tenure--;
    }

    return OK;
}

//================================================================================
// TABU SEARCH
//================================================================================

ERROR_CODE tabu_init(tabu_search* ts, int nnodes, POLICIES policy){

    ts->policy = policy;

    ts->tabu_list = (int*) calloc(nnodes, sizeof(int*));
    for(int i=0; i< nnodes; i++){
        ts->tabu_list[i] = -1;
    }

    ts->increment = true;

    ts->tenure = MIN_FRACTION * nnodes + 1;

    // initialize min and max tenure
    ts->max_tenure = MAX_FRACTION * nnodes;
    ts->min_tenure = MIN_FRACTION * nnodes;

    return OK;
    
}

ERROR_CODE tabu_search_2opt(instance* inst, POLICIES policy){
    // inititialize
    tabu_search ts;
    if(tabu_init(&ts, inst->nnodes, policy) != OK){
        log_fatal("Error in init tabu search"); 
        tsp_handlefatal(inst);
    }

    int* solution_path = (int*) calloc(inst->nnodes, sizeof(int*));
    double solution_cost = __DBL_MAX__;

    // get a solution with an heuristic algorithm
    if(h_greedyutil(inst, inst->starting_node, solution_path, &solution_cost) != OK){
        log_fatal("Error in greedy solution computation");
        tsp_handlefatal(inst);
        free(solution_path);
    }

    log_debug("greedy sol cost: %f", solution_cost);

    // tabu search with 2opt moves
    for(int k=0; k < inst->nnodes * inst->nnodes; k++){

        // check if exceeds time
        double ex_time = utils_timeelapsed(inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               free(solution_path);
                return DEADLINE_EXCEEDED;
            }
        }

        // update tenure
        tabu_linear_policy(&ts);

        // 2opt move
        ERROR_CODE e = tabu_best_move(inst, solution_path, &solution_cost, &ts, k);

        if(e != OK){
            log_fatal("Error in tabu best move"); 
            tsp_handlefatal(inst);
            free(solution_path);
        }

        tsp_update_best_solution(inst, solution_cost, solution_path);
    }

    free(solution_path);
    return OK;

}

ERROR_CODE tabu_best_move(instance* inst, int* solution_path, double* solution_cost, tabu_search* ts, int current_iteration){
    double best_delta = __DBL_MAX__;
    int best_swap[2] = {-1, -1};

    int *prev = (int*)calloc(inst->nnodes, sizeof(int*));          // save the path of the solution without 2opt
    for (int i = 0; i < inst->nnodes; i++) {
        prev[solution_path[i]] = i;
    }

    // scan nodes to find best swap
    for (int a = 0; a < inst->nnodes - 1; a++) {
        if(is_in_tabu_list(ts, a, current_iteration)) { continue; }
        for (int b = a+1; b < inst->nnodes; b++) {
            if(is_in_tabu_list(ts, b, current_iteration)) { continue; }
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
            if (delta < best_delta) {
                best_delta = delta;
                best_swap[0] = a;
                best_swap[1] = b;
            }
        }
    }

    // execute best swap
    if(best_delta < __DBL_MAX__){    
        int a = best_swap[0];
        int b = best_swap[1];
        //log_debug("iteration %d: best swap is %d, %d with delta=%f", current_iteration, a, b, best_delta);
        int succ_a = solution_path[a]; //successor of a
        int succ_b = solution_path[b]; //successor of b

        //Swap the 2 edges
        solution_path[a] = b;
        solution_path[succ_a] = succ_b;
                    
        //Reverse the path from the b to the successor of a
        h_reverse_path(inst, b, succ_a, prev, solution_path);
        *solution_cost += best_delta;

        // update tabu list
        ts->tabu_list[a] = current_iteration;
        ts->tabu_list[b] = current_iteration;
        ts->tabu_list[succ_a] = current_iteration;
        ts->tabu_list[succ_b] = current_iteration;
    }

    free(prev);

    return OK;
}

//================================================================================
// UTILS
//================================================================================

bool is_in_tabu_list(tabu_search* ts, int node, int current_iteration){
    return current_iteration - ts->tabu_list[node] <= ts->tenure || ts->tabu_list[node] != -1;
}
