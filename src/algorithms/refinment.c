#include "refinment.h"

ERROR_CODE ref_2opt(instance* inst, tsp_solution* solution){
    
    double delta = 0;

    do {
        // see if it exceeds the time limit
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                log_debug("time limit exceeded");
                return DEADLINE_EXCEEDED;
            }
        }

        delta = ref_2opt_once(inst, solution->path);
        if (delta < 0 ){
            solution->cost += delta;
            log_info("2-opt improved solution: new cost: %f", solution->cost);
        }
        
    }while(delta < 0);
    
    ERROR_CODE error = tsp_update_best_solution(inst, solution);
    if(!err_ok(error)){
        log_error("code %d : ", error);
    }
    
    return OK;
}

double ref_2opt_once(instance* inst, int* solution_path){
    double best_delta = 0;
    int best_swap[2] = {-1, -1};

    int *prev = calloc(inst->nnodes, sizeof(int));          // save the path of the solution without 2opt
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

            // Compute the delta. If < 0 it means there is a crossing
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

    if(best_delta < 0){
        int a = best_swap[0];
        int b = best_swap[1];
        log_debug("best swap is %d, %d: executing swap...", a, b);
        int succ_a = solution_path[a]; //successor of a
        int succ_b = solution_path[b]; //successor of b
                    
        //Reverse the path from the b to the successor of a
        ref_reverse_path(inst, a, succ_a, b, succ_b, prev, solution_path);
    }

    free(prev);

    return best_delta;

}

void ref_reverse_path(instance *inst, int a, int succ_a, int b, int succ_b, int *prev, int* solution_path) {
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

    for (int k = 0; k < inst->nnodes; k++) {
        prev[solution_path[k]] = k;
    }
}
