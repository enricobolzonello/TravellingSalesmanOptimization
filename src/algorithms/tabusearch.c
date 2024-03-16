#include "tabusearch.h"

ERROR_CODE tabu_fixed_policy(tenure_policy* t, int value){
    if(t->dependent_policy || t->fixed_policy || t->random_policy){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    t->fixed_policy = true;

    t->tenure = value;

    return OK;
}

ERROR_CODE tabu_dependent_policy(tenure_policy* t, int nnodes){
    if(t->fixed_policy || t->random_policy){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    t->dependent_policy = true;
    t->tenure = (int)ceil((MAX_FRACTION + MIN_FRACTION) * nnodes);

    return OK;
}

ERROR_CODE tabu_random_policy(tenure_policy* t, int nnodes){
    //if(t->dependent_policy || t->fixed_policy){
    //    log_warn("policy has already been set");
    //    return ALREADY_EXISTS;
    //}

    t->random_policy = true;

    int max_tenure = MAX_FRACTION * nnodes;
    int min_tenure = MIN_FRACTION * nnodes;

    printf("rand: %f   ", rand()/ RAND_MAX);

    t->tenure = (int)(rand() / RAND_MAX) * (max_tenure - min_tenure) + min_tenure;

    return OK;
}

ERROR_CODE tabu_linear_policy(tenure_policy* t, int current_iteration, int nnodes){
    t->changing_policy = true;
    
    int max_tenure = MAX_FRACTION * nnodes;
    int min_tenure = MIN_FRACTION * nnodes;

    if(max_tenure == t->tenure || min_tenure == t->tenure){
        t->increment = !t->increment;
    }

    if(t->increment){
        t->tenure++;
    }else{
        t->tenure--;
    }

    return OK;
}

ERROR_CODE tabu_search_2opt(instance* inst){
    // init
    tabu_search ts;
    if(init_tabu_search(&ts, inst->nnodes) != OK){ log_fatal("Error in init tabu search"); }
    int* solution_path = calloc(inst->nnodes, sizeof(int));
    double solution_cost = __DBL_MAX__;

    log_debug("tenure: %d, number iterations %d", ts.tenure_policy.tenure, ts.number_iterations);

    // get a solution
    if(h_greedyutil(inst, inst->starting_node, solution_path, &solution_cost) != OK){ log_fatal("Error in greedy solution computation"); }
    log_debug("greedy sol cost: %f", solution_cost);

    // tabu search with 2opt moves
    for(int k=0; k < ts.number_iterations; k++){

        ERROR_CODE e = tabu_best_move(inst, solution_path, &solution_cost, &ts, k);
        if(e != OK){ log_fatal("Error in tabu best move"); }

        tsp_update_best_solution(inst, solution_cost, solution_path);
    }

    return OK;

}

ERROR_CODE init_tabu_search(tabu_search* ts, int nnodes){
    ts->tabu_list = calloc(nnodes, sizeof(int));
    for(int i=0; i< nnodes; i++){
        ts->tabu_list[i] = -1;
    }

    ts->number_iterations = nnodes*nnodes;
    ts->tenure_policy.tenure = 0;

    if(tabu_fixed_policy(&ts->tenure_policy, 10) != OK){ return UNKNOWN; }

    return OK;
    
}

ERROR_CODE tabu_best_move(instance* inst, int* solution_path, double* solution_cost, tabu_search* ts, int current_iteration){
    double best_delta = __DBL_MAX__;
    int best_swap[2] = {-1, -1};

    int *prev = calloc(inst->nnodes, sizeof(int));          // save the path of the solution without 2opt
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
        log_debug("iteration %d: best swap is %d, %d with delta=%f", current_iteration, a, b, best_delta);
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
    }

    free(prev);

    return OK;
}

bool is_in_tabu_list(tabu_search* ts, int node, int current_iteration){
    return current_iteration - ts->tabu_list[node] <= ts->tenure_policy.tenure || ts->tabu_list[node] != -1;
}