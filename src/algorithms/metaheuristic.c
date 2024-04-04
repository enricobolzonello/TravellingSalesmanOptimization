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

    ERROR_CODE e = OK;

    // file to hold solution value in each iteration
    FILE* f = fopen("results/TabuResults.dat", "w+");

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
    for(int k=0; k < inst->options_t.k; k++){

        // check if exceeds time
        double ex_time = utils_timeelapsed(inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
                e = DEADLINE_EXCEEDED;
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

    plot_stats(plot, "results/TabuResults.dat");
    plot_free(plot);

    // free resources
    free(solution.path);
    tabu_free(&ts);

    return e;

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

        ref_reverse_path(inst, a, succ_a, b, succ_b, prev, solution_path);
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

    ERROR_CODE e = h_Greedy_iterative(inst); // start with a bad solution
    if(!err_ok(e)){
            log_fatal("code %d : Error in greedy", e);
            tsp_handlefatal(inst);
            free(solution.path);
        }
    log_info("Greedy done!");
    
    // copy the best solution found by greedy
    memcpy(solution.path, inst->best_solution.path, inst->nnodes * sizeof(int));
    solution.cost = inst->best_solution.cost;

    tsp_solution best_vns = tsp_init_solution(inst->nnodes);
    best_vns.cost = solution.cost;

    // file to hold solution value in each iteration
    FILE* f = fopen("results/VNSResults.dat", "w+");
    
    e  = OK;
    // call 3 opt k times
    for(int i=0; i<inst->options_t.k; i++){
        // check if exceeds time
        double ex_time = utils_timeelapsed(inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               e = DEADLINE_EXCEEDED;
               break;
            }
        }

        // local search
        e = ref_2opt(inst, &solution);
        if(!err_ok(e)){
            log_fatal("code %d : Error in local search", e); 
            tsp_handlefatal(inst);
            free(solution.path);
            free(best_vns.path);
        }

        if(solution.cost < best_vns.cost){
            log_info("found new best: %f ", solution.cost);
            best_vns.cost = solution.cost;
            memcpy(best_vns.path, solution.path, inst->nnodes * sizeof(int));
        }

        // save current iteration and current solution cost to file for the plot
        fprintf(f, "%d,%f\n", i, solution.cost);

        // kick
        int r = rand() % (UPPER - LOWER + 1) - LOWER;
        for(int j=0; j<r; j++){
            e = vns_kick(inst, &solution);
            if(!err_ok(e)){
                log_fatal("code %d : Error in kick", e); 
                tsp_handlefatal(inst);
                free(solution.path);
                free(best_vns.path);
            }
        }
        
    }

    fclose(f);

    e = tsp_update_best_solution(inst, &best_vns);
    if(!err_ok(e)){
        log_error("code %d : error in updating best solution of VNS");
    }

    // plot the solution progression during iterations
    PLOT plot = plot_open("VNSIterationsPlot");
    
    if(inst->options_t.tofile){
        plot_tofile(plot, "VNSIterationsPlot");
    }

    plot_stats(plot, "results/VNSResults.dat");
    plot_free(plot);

    free(best_vns.path);
    free(solution.path);
    return e;
}

// 3 opt kick
ERROR_CODE vns_kick(instance* inst, tsp_solution* solution){

    log_debug("KICK");

    int *prev = calloc(inst->nnodes, sizeof(int));          // save the path of the solution without kick
    for (int i = 0; i < inst->nnodes; i++) {
        prev[solution->path[i]] = i;
    }

    //From list of successor to Tour
    int* tour = (int*)calloc(inst->nnodes, sizeof(int));
    int node=0;
    int idx=0;
    while(idx<inst->nnodes){
        tour[idx]=node;
        idx+=1;
        node=solution->path[node];
    }

    int indexes[3];
    int i, j;
    for (i = 0; i < 3; i++) {
        int random_number;
        do {
            random_number = rand() % (inst->nnodes);
            // Check if the number is already generated
            for (j = 0; j < i; j++) {
                // no same number or predecessor/successor
                if (random_number == indexes[j] || random_number == tour[indexes[j]-1] || random_number == tour[indexes[j]+1]) {
                    random_number = -1; // Mark as invalid
                    break;
                }
            }
        } while (random_number == -1); // Repeat if the number is already generated
        indexes[i] = random_number;

        // make them in order
        for (int j = i; j > 0; j--) {
                if (indexes[j] < indexes[j - 1])
                    swap(&indexes[j], &indexes[j-1]);
            }
    }

    log_debug("random number: %d %d %d", indexes[0], indexes[1], indexes[2]);

    int nodeA = tour[indexes[0]];
    int nodeSuccA = tour[(indexes[0]+1) % inst->nnodes];
    int nodeB = tour[indexes[1]];
    int nodeSuccB = tour[(indexes[1]+1) % inst->nnodes];
    int nodeC = tour[indexes[2]];
    int nodeSuccC = tour[(indexes[2]+1) % inst->nnodes];

    log_debug("A:(%d,%d)\t B:(%d,%d)\t C:(%d,%d)\n", nodeA, nodeSuccA, nodeB, nodeSuccB, nodeC, nodeSuccC);

    ERROR_CODE e = makeMove(inst, prev, solution, 7, nodeA, nodeSuccA, nodeB, nodeSuccB, nodeC, nodeSuccC);
    if(!err_ok(e)){
        log_fatal("code %d : Error in make move", e); 
        tsp_handlefatal(inst);
        free(prev);
    }

    // free resources
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

// https://tsp-basics.blogspot.com/2017/03/3-opt-move.html
ERROR_CODE makeMove(instance *inst, int *prev, tsp_solution* solution, int bestCase, int i, int succ_i, int j, int succ_j, int k, int succ_k) {
    ERROR_CODE e = OK;
    int temp;
    switch (bestCase){
        case 1: 
            log_debug("case 1");

            // invert segment a
            ref_reverse_path(inst, k, succ_k, i, succ_i, prev, solution->path);

            break;
        case 2:
            log_debug("case 2");
            
            // invert segment c
            ref_reverse_path(inst, j, succ_j, k, succ_k, prev, solution->path);

            break;
        case 3:
            log_debug("case 3");
            
            // invert segment b
            ref_reverse_path(inst, i, succ_i, j, succ_j, prev, solution->path);

            break;
        case 4:
            // inverts segments b and c
            log_debug("case 4");
            ref_reverse_path(inst, i, succ_i, j, succ_j, prev, solution->path);
            temp = j;
            j = succ_i;
            succ_i = j;
            ref_reverse_path(inst, j, succ_j, k, succ_k, prev, solution->path);

            /*solution->path[i] = j;
            solution->path[succ_i] = k;
            solution->path[succ_j] = succ_k;*/
            break;
        case 5:
            // inverts segments a and b
            log_debug("invert segment a and b");
            ref_reverse_path(inst, k, succ_k, i, succ_i, prev, solution->path);
            temp = i;
            i = succ_k;
            succ_k = temp;
            ref_reverse_path(inst, i, succ_i, j, succ_j, prev, solution->path);

            /*solution->path[i] = k;
            solution->path[succ_j] = succ_i;
            solution->path[j] = succ_k;*/
            break;
        case 6:
            // inverts segments a and c
            log_debug("invert segment a and c");
            ref_reverse_path(inst, j, succ_j, k, succ_k, prev, solution->path);
            temp = k;
            k = succ_j;
            succ_j = temp;
            ref_reverse_path(inst, k, succ_k, i, succ_i, prev, solution->path);

            /*solution->path[i] = succ_j;
            solution->path[k] = j;
            solution->path[succ_i] = succ_k;*/

            break;
        case 7:
            log_debug("case 7");

            // swap edges
            solution->path[i] = succ_j; 
            solution->path[k] = succ_i; 
            solution->path[j] = succ_k; 

            // doesn't need reverse!

            break;
        
        default:
            break;
    }

    return e;
}
