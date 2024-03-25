#include "heuristics.h"

//================================================================================
// NEAREST NEIGHBOUR HEURISTIC
//================================================================================

ERROR_CODE h_Greedy(instance* inst){

    tsp_solution solution = tsp_init_solution(inst->nnodes);

    log_info("running GREEDY");
    ERROR_CODE error = h_greedyutil(inst, inst->starting_node, solution.path, &solution.cost);
    if(!err_ok(error)){
        log_error("code %d : greedy did not finish correctly", error);
    }

    error = tsp_update_best_solution(inst, &solution);
    if(!err_ok(error)){
        log_error("code %d : error in updating solution for greedy", error);
    }

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
        if(!err_ok(error)){
            log_error("code %d : error in iteration %d of greedy iterative", error, i);
            break;
        }

        if(solution.cost < inst->best_solution.cost){
            log_info("found new best, node %d", i);
            inst->starting_node = i;
            error = tsp_update_best_solution(inst, &solution);
            if(!err_ok(error)){
                log_error("code %d : error in updating best solution of greedy iterative in iteration %d", error, i);
            }
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
        if(!err_ok(error)){
            log_error("code %d : error in iteration %i of 2opt greedy", error, i);
            break;
        }

        log_debug("greedy solution: cost: %f", solution.cost);

        error = ref_2opt(inst, &solution);
        if(!err_ok(error)){
            log_error("code %d : error in 2opt", error);
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