#include "heuristics.h"

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
// NEAREST NEIGHBOUR HEURISTIC
//================================================================================

ERROR_CODE h_Greedy(instance* inst){

    tsp_solution solution = tsp_init_solution(inst->nnodes);

    log_info("running GREEDY");
    ERROR_CODE error = h_greedyutil(inst, inst->starting_node, solution.path, &solution.cost);

    tsp_update_best_solution(inst, &solution);

    free(solution.path);
    return error;
}

ERROR_CODE h_Greedy_iterative(instance* inst){
    tsp_solution solution = tsp_init_solution(inst->nnodes);

    for(int i=0; i<inst->nnodes; i++){
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                return DEADLINE_EXCEEDED;
            }
        }

        log_debug("starting greedy with node %d", i);
        ERROR_CODE error = h_greedyutil(inst, i, solution.path, &solution.cost);
        if(error != OK){
            log_error("code %d\n", error);
            break;
        }

        if(solution.cost < inst->best_solution.cost){
            log_info("found new best, node %d", i);
            inst->starting_node = i;
           tsp_update_best_solution(inst, &solution);
        }
    }

    free(solution.path);

    return OK;
}

ERROR_CODE h_greedy_2opt(instance* inst){
    tsp_solution solution = tsp_init_solution(inst->nnodes);

    for(int i=0; i<inst->nnodes; i++){
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                return DEADLINE_EXCEEDED;
            }
        }

        log_debug("starting greedy with node %d", i);
        ERROR_CODE error = h_greedyutil(inst, i, solution.path, &solution.cost);
        if(error != OK){
            log_error("greedy error: code %d\n", error);
            break;
        }
        log_debug("greedy solution: cost: %f", solution.cost);


        error = ref_2opt(inst, &solution);
        if(error == INVALID_ARGUMENT){
            log_error("2opt error: code %d\n", error);
            break;
        }else if (error == OK)
        {
            log_info("found new best solution: starting node %d, cost %f", i, inst->best_solution.cost);
            inst->starting_node = i;
        }
        
    }

    free(solution.path);

    return OK;
}

//================================================================================
// TABU SEARCH
//================================================================================

ERROR_CODE tabu_init(tabu_search* ts, int nnodes, POLICIES policy){

    ts->policy = policy;

    ts->tabu_list = (int*) calloc(nnodes, sizeof(int));
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
    // initialize
    tabu_search ts;
    if(tabu_init(&ts, inst->nnodes, policy) != OK){
        log_fatal("Error in init tabu search"); 
        tsp_handlefatal(inst);
    }

    // file to hold solution value in each iteration
    FILE* f = fopen("results/TabuResults.dat", "w+,ccs=UTF-8");

    tsp_solution solution = tsp_init_solution(inst->nnodes);

    // get a solution with an heuristic algorithm

    if(h_greedy_2opt(inst) != OK){
        log_fatal("Error in greedy solution computation");
        tsp_handlefatal(inst);
        free(solution.path);
    }

    solution.cost = inst->best_solution.cost;
    memcpy(solution.path, inst->best_solution.path, inst->nnodes * sizeof(int));
    log_debug("2opt greedy sol cost: %f", solution.cost);

    // tabu search with 2opt moves
    for(int k=0; k < inst->nnodes * inst->nnodes; k++){

        // check if exceeds time
        double ex_time = utils_timeelapsed(inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               free(solution.path);
                return DEADLINE_EXCEEDED;
            }
        }

        // update tenure
        tabu_linear_policy(&ts);

        // 2opt move
        ERROR_CODE e = tabu_best_move(inst, solution.path, &solution.cost, &ts, k);

        if(e != OK){
            log_fatal("Error in tabu best move"); 
            tsp_handlefatal(inst);
            free(solution.path);
        }

        tsp_update_best_solution(inst, &solution);

        // save current iteration and current solution cost to file for the plot
        fprintf(f, "%d,%f\n", k, solution.cost);
    }

    fclose(f);

    // plot the solution progression during iterations
    PLOT plot = plot_open("TabuIterationsPlot");
    
    if(inst->options_t.tofile){
        plot_tofile(plot, "TabuIterationsPlot");
    }

    plot_stats(plot);
    plot_free(plot);

    // free resources
    free(solution.path);
    tabu_free(&ts);
    return OK;

}

ERROR_CODE tabu_best_move(instance* inst, int* solution_path, double* solution_cost, tabu_search* ts, int current_iteration){
    double best_delta = __DBL_MAX__;
    int best_swap[2] = {-1, -1};

    int *prev = (int*)calloc(inst->nnodes, sizeof(int));          // save the path of the solution without 2opt
    for (int i = 0; i < inst->nnodes; i++) {
        prev[solution_path[i]] = i;
    }

    // scan nodes to find best swap
    for (int a = 0; a < inst->nnodes - 1; a++) {
        for (int b = a+1; b < inst->nnodes; b++) {
            int succ_a = solution_path[a]; //successor of a
            int succ_b = solution_path[b]; //successor of b
            
            // Skip non valid configurations
            if (succ_a == succ_b || a == succ_b || b == succ_a){
                continue;
            }

            if(is_in_tabu_list(ts, a, current_iteration) || is_in_tabu_list(ts, b, current_iteration) || is_in_tabu_list(ts, succ_a, current_iteration) || is_in_tabu_list(ts, succ_b, current_iteration)){ 
                continue; 
            }

            // Compute the delta
            double current_cost = tsp_get_cost(inst, a, succ_a) + tsp_get_cost(inst, b, succ_b);
            double swapped_cost = tsp_get_cost(inst, a, b) + tsp_get_cost(inst, succ_a, succ_b);
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
        ref_reverse_path(inst, b, succ_a, prev, solution_path);
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
                double temp = tsp_get_cost(inst, curr, i);
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
    sol_cost += tsp_get_cost(inst, curr, starting_node);
    *(solution_cost) = sol_cost;

    free(visited);

    return OK;
}

bool is_in_tabu_list(tabu_search* ts, int node, int current_iteration){
    return current_iteration - ts->tabu_list[node] < ts->tenure && ts->tabu_list[node] != -1;
}

void tabu_free(tabu_search* ts){
    free(ts->tabu_list);
}
