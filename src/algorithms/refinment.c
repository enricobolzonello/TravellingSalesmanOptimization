#include "refinment.h"

ERROR_CODE ref_2opt(tsp_solution* solution, double* costs, bool update_incumbent){

    // re-initialize cost for VNS
    solution->cost = 0;
    for(int i=0; i<tsp_inst.nnodes; i++){
        solution->cost += costs[i * tsp_inst.nnodes + solution->path[i]];
    }

    ERROR_CODE e = T_OK;
    
    double delta = 0;

    do {
        // see if it exceeds the time limit
        if(tsp_env.timelimit != -1.0){
            double ex_time = utils_timeelapsed(&tsp_inst.c);
            if(ex_time > tsp_env.timelimit){
                log_debug("time limit exceeded in 2opt");
                e = DEADLINE_EXCEEDED;
                break;
            }
        }

        delta = ref_2opt_once( solution, costs);
    }while(delta < EPSILON);
    
    if(update_incumbent){
        ERROR_CODE error = tsp_update_best_solution( solution);
        if(!err_ok(error)){
            log_error("code %d : Error in 2opt solution update", error);
        }
    }
    
    return e;
}

double ref_2opt_once( tsp_solution* solution, double* costs){
    double best_delta = 0;
    int best_swap[2] = {-1, -1};

    int *prev = calloc(tsp_inst.nnodes, sizeof(int));          // save the path of the solution without 2opt
    for (int i = 0; i < tsp_inst.nnodes; i++) {
        prev[solution->path[i]] = i;
    }

    // scan nodes to find best swap
    for (int a = 0; a < tsp_inst.nnodes - 1; a++) {
        for (int b = a+1; b < tsp_inst.nnodes; b++) {
            int succ_a = solution->path[a]; //successor of a
            int succ_b = solution->path[b]; //successor of b
            
            // Skip non valid configurations
            if (succ_a == succ_b || a == succ_b || b == succ_a){
                continue;
            }

            // Compute the delta. If < 0 it means there is a crossing
            double current_cost = costs[a * tsp_inst.nnodes + succ_a] + costs[b * tsp_inst.nnodes + succ_b];
            double swapped_cost = costs[a * tsp_inst.nnodes + b] + costs[succ_a * tsp_inst.nnodes + succ_b];
            double delta = swapped_cost - current_cost;
            if (delta < best_delta) {
                best_delta = delta;
                best_swap[0] = a;
                best_swap[1] = b;
            }
        }
    }


    // execute best swap

    if(best_delta < EPSILON){
        int a = best_swap[0];
        int b = best_swap[1];
        log_debug("best swap is %d, %d: executing swap...", a, b);
        int succ_a = solution->path[a]; //successor of a
        int succ_b = solution->path[b]; //successor of b
                    
        //Reverse the path from the b to the successor of a
        ref_reverse_path(a, succ_a, b, succ_b, prev, solution->path);

        // update solution cost
        solution->cost += best_delta;
        log_debug("2-opt improved solution: new cost: %f", solution->cost);
    }

    utils_safe_free(prev);

    return best_delta;

}

void ref_reverse_path(int a, int succ_a, int b, int succ_b, int *prev, int* solution_path) {
    //Swap the 2 edges
    solution_path[a] = b;
    solution_path[succ_a] = succ_b;

    // reverse path from b to succ_a
    int currnode = b;
    while (1) {
        int node = prev[currnode];
        solution_path[currnode] = node;
        currnode = node;
        if (node == succ_a) {
            break;
        }
    }

    for (int k = 0; k < tsp_inst.nnodes; k++) {
        prev[solution_path[k]] = k;
    }
}
