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

    struct utils_clock c = utils_startclock();
    
    double sol_cost = 0;
    int min_idx;
    double min_dist;
    double temp;
    bool done = false;

    while(!done){
        // check that we have not exceed time limit
        double ex_time = utils_timeelapsed(c);
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

    inst->best_solution_cost = inst->solution_cost;
    inst->best_solution_path = calloc(inst->nnodes, sizeof(int));
    memcpy(inst->best_solution_path, inst->solution_path, inst->nnodes * sizeof(int));

    return error;
}

ERROR_CODE h_Greedy_iterative(instance* inst){
    int i;
    ERROR_CODE error;
    struct utils_clock c = utils_startclock();

    double best_cost = __DBL_MAX__;
    int* best_path = (int*) calloc(inst->nnodes, sizeof(int*));
    for(i=0; i<inst->nnodes; i++){
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(c);
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
            best_cost = inst->solution_cost;
            memcpy(best_path, inst->solution_path, inst->nnodes * sizeof(int));
        }
    }

    tsp_update_best_solution(inst);

    free(best_path);

    return OK;
}

// TODO: doesn't work
double h_2opt(instance* inst) {
    int best_swap[2] = {-1, -1};
    double improvement = 0;

    // swap between i+1 and j
    for(int i=0; i<inst->nnodes - 1; i++){
        for(int j=i; j<inst->nnodes - 1; j++){
            double current_cost = inst->costs[i][i+1] + inst->costs[j][j+1];
            double swapped_cost = inst->costs[i][j] + inst->costs[i+1][j+1];

            if(current_cost - swapped_cost > improvement){
                best_swap[0] = i+1;
                best_swap[1] = j;
                improvement = current_cost - swapped_cost;
            }
        }
    }

    if(improvement > 0){
        h_swap(best_swap, improvement,inst);
    }

    return improvement;
}

void h_swap(int swap[2], double improvement, instance* inst){
    int temp = inst->solution_path[swap[0]];
    inst->solution_path[swap[0]] = inst->solution_path[swap[1]];
    inst->solution_path[swap[1]] = temp;

    // invert path in the middle
    int start = swap[0] + 1;
    int end = swap[1] - 1;
    while (start < end) {
        int temp = inst->solution_path[start];
        inst->solution_path[start] = inst->solution_path[end];
        inst->solution_path[end] = temp;

        start++;
        end--;
    }

    // update cost
    inst->solution_cost -= improvement;
}

ERROR_CODE h_2opt_iterative(instance* inst){
    struct utils_clock c = utils_startclock();
    double improvement  = 0;
    do{
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(c);
            if(ex_time > inst->options_t.timelimit){
                log_debug("time limit exceeded");
                return DEADLINE_EXCEEDED;
            }
        }

        improvement = h_2opt(inst);
        tsp_update_best_solution(inst);
    }while(improvement > 0);

    return OK;
}

