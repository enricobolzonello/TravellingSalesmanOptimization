#include "metaheuristic.h"

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

ERROR_CODE mh_TabuSearch(instance* inst, POLICIES policy){
    // initialize
    tabu_search ts;
    if(!err_ok(tabu_init(&ts, inst->nnodes, policy))){
        log_fatal("code %d : Error in init tabu search"); 
        tsp_handlefatal(inst);
    }

    // file to hold solution value in each iteration
    FILE* f = fopen("results/TabuResults.dat", "w+,ccs=UTF-8");

    tsp_solution solution = tsp_init_solution(inst->nnodes);

    // get a solution with an heuristic algorithm

    if(!err_ok(h_greedy_2opt(inst))){
        log_fatal("code %d : Error in greedy solution computation");
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
        ERROR_CODE e = tabu_linear_policy(&ts);
        if(!err_ok(e)){
            log_warn("using already set policy %d", ts.policy);
        }

        // 2opt move
        e = tabu_best_move(inst, solution.path, &solution.cost, &ts, k);
        if(!err_ok(e)){
            log_fatal("code %d : Error in tabu best move", e); 
            tsp_handlefatal(inst);
            free(solution.path);
        }

        e = tsp_update_best_solution(inst, &solution);
        if(!err_ok(e)){
            log_fatal("code %d : Error in updating best solution", e); 
            tsp_handlefatal(inst);
            free(solution.path);
        }

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
// VARIABLE NEIGHBORHOOD SEARCH
//================================================================================

ERROR_CODE mh_VNS(instance* inst){
    tsp_solution solution = tsp_init_solution(inst->nnodes);

    ERROR_CODE e = h_greedy_2opt(inst);
    if(!err_ok(e)){
            log_fatal("code %d : Error in tabu best move", e); 
            tsp_handlefatal(inst);
            free(solution.path);
        }
    
    // copy the best solution found by greedy+2opt
    memcpy(solution.path, inst->best_solution.path, inst->nnodes * sizeof(int));
    solution.cost = inst->best_solution.cost;

    // call 3 opt k times
    for(int i=0; i<K; i++){
        // check if exceeds time
        double ex_time = utils_timeelapsed(inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               free(solution.path);
                return DEADLINE_EXCEEDED;
            }
        }

        // define kick
        e = vns_kick(inst, &solution);
        if(!err_ok(e)){
            log_fatal("code %d : Error in kick", e); 
            tsp_handlefatal(inst);
            free(solution.path);
        }

        // local search
        e = ref_2opt(inst, &solution);
        if(!err_ok(e)){
            log_fatal("code %d : Error in local search", e); 
            tsp_handlefatal(inst);
            free(solution.path);
        }

        if(solution.cost < inst->best_solution.cost){
            log_info("found new best, node %d", i);
            inst->starting_node = i;
            e = tsp_update_best_solution(inst, &solution);
            if(!err_ok(e)){
                log_error("code %d : error in updating best solution of greedy iterative in iteration %d", e, i);
            }
        }
    }

    free(solution.path);
    return OK;
}

// 3 opt kick
ERROR_CODE vns_kick(instance* inst, tsp_solution* solution){

    int *prev = calloc(inst->nnodes, sizeof(int));          // save the path of the solution without 2opt
    for (int i = 0; i < inst->nnodes; i++) {
        prev[solution->path[i]] = i;
    }

    // scan nodes to find best swap
    for (int a = 0; a < inst->nnodes; a++) {
        int succ_a = solution->path[a]; //successor of a
        for (int b = 1; b < inst->nnodes - 2; b++) {
            int succ_b = solution->path[b]; //successor of b
            for(int c = b + 1; b < inst->nnodes; c++){
                int succ_c = solution->path[c]; //successor of c

                // Calculate the gain for each case
                int bestCase = bestMove(inst, a, succ_a, b, succ_b, c, succ_c, solution);

                if(bestCase == 3 || bestCase == 6 || bestCase == 7){
                    makeMove(inst, prev, solution->path, bestCase, a, succ_a, b, succ_b, c, succ_c);
                    log_info("distance after 3opt: %d", solution->cost);
                }
            }
        }
    }

    free(prev);

    return OK;
}


//================================================================================
// UTILS
//================================================================================

bool is_in_tabu_list(tabu_search* ts, int node, int current_iteration){
    return current_iteration - ts->tabu_list[node] < ts->tenure && ts->tabu_list[node] != -1;
}

void tabu_free(tabu_search* ts){
    free(ts->tabu_list);
}

// Finds the case that results in maximum length gain
int bestMove(instance* inst, int a, int b, int c, int d, int e, int f, tsp_solution* solution) { 

	double gains[8];

	gains[0] = 0;

	gains[1] = tsp_get_cost(inst, a, e) + tsp_get_cost(inst, b, f) - tsp_get_cost(inst, a, b) - tsp_get_cost(inst, e, f);

	gains[2] = tsp_get_cost(inst, c, e) + tsp_get_cost(inst, d, f) - tsp_get_cost(inst, c, d) - tsp_get_cost(inst, e, f);

	gains[3] = tsp_get_cost(inst, a, c) + tsp_get_cost(inst, b, d) - tsp_get_cost(inst, a, b) - tsp_get_cost(inst, c, d);

	int deletedEdges = tsp_get_cost(inst, a, b) + tsp_get_cost(inst, c, d) + tsp_get_cost(inst, e, f);

	gains[4] = tsp_get_cost(inst, a, c) + tsp_get_cost(inst, b, e) + tsp_get_cost(inst, d, f) - deletedEdges;

	gains[5] = tsp_get_cost(inst, a, e) + tsp_get_cost(inst, d, b) + tsp_get_cost(inst, c, f) - deletedEdges;

	gains[6] = tsp_get_cost(inst, a, d) + tsp_get_cost(inst, e, c) + tsp_get_cost(inst, b, f) - deletedEdges;

	gains[7] = tsp_get_cost(inst, a, d) + tsp_get_cost(inst, e, b) + tsp_get_cost(inst, c, f) - deletedEdges;

	double maxGain = 0;
	int bestCase = 0;
	for (int i = 1; i < 8; i++) {
		if (gains[i] < 0 && gains[i] < maxGain) {
			bestCase = i;
			maxGain = gains[i];
		}
	}

    log_debug("max gain: %f", maxGain);
	solution->cost += maxGain;

	return bestCase;
}

// https://tsp-basics.blogspot.com/2017/03/3-opt-move.html
void makeMove(instance *inst, int *prev, int* solution_path, int bestCase, int i, int succ_i, int j, int succ_j, int k, int succ_k) {
    switch (bestCase){
        case 1:                
            ref_reverse_path(inst, i, succ_i, k, succ_k, prev, solution_path);
            break;
        case 2:
            ref_reverse_path(inst, j, succ_j, k, succ_k, prev, solution_path);
            break;
        case 3:
            ref_reverse_path(inst, i, succ_i, j, succ_j, prev, solution_path);
            break;
        // TODO: from case 4
        case 4:
            ref_reverse_path(inst, i, succ_i, j, succ_j, prev, solution_path);
            ref_reverse_path(inst, succ_i, j, prev, solution_path);
            break;
        case 5:
            solution_path[succ_k] = succ_i;
            solution_path[i] = succ_j;

            ref_reverse_path(inst, succ_k, i, prev, solution_path);

            solution_path[j] = k;
            solution_path[i] = succ_j;

            ref_reverse_path(inst, succ_i, j, prev, solution_path);
            break;
        case 6:
            solution_path[k] = succ_i;
            solution_path[i] = j;

            ref_reverse_path(inst, succ_k, i, prev, solution_path);

            solution_path[succ_j] = succ_k;
            solution_path[i] = j;

            ref_reverse_path(inst, succ_j, k, prev, solution_path);
            break;
        // not done
        case 7:
            solution_path[k] = succ_i;
            solution_path[i] = j;

            ref_reverse_path(inst, succ_k, i, prev, solution_path);
            ref_reverse_path(inst, succ_i, j, prev, solution_path);
            ref_reverse_path(inst, succ_j, k, prev, solution_path);
            break;
        
        default:
            break;
    }
}
