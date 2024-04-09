#include "heuristics.h"

struct edge{
    int i;
    int j;
};

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
    ERROR_CODE e = OK;
    tsp_solution solution = tsp_init_solution(inst->nnodes);

    for(int i=0; i<inst->nnodes; i++){
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                e = DEADLINE_EXCEEDED;
                break;
            }
        }

        log_debug("starting greedy with node %d", i);
        ERROR_CODE error = h_greedyutil(inst, i, solution.path, &solution.cost);
        if(!err_ok(error)){
            log_error("code %d : error in iteration %d of greedy iterative", error, i);
            continue;
        }

        if(solution.cost < inst->best_solution.cost){
            log_info("found new best, node %d", i);
            inst->starting_node = i;
            error = tsp_update_best_solution(inst, &solution);
            if(!err_ok(error)){
                log_error("code %d : error in updating best solution of greedy iterative in iteration %d", error, i);
                continue;
            }
        }
    }

    free(solution.path);

    return e;
}

ERROR_CODE h_greedy_2opt(instance* inst){
    ERROR_CODE e = OK;
    tsp_solution solution = tsp_init_solution(inst->nnodes);

    for(int i=0; i<inst->nnodes; i++){
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                e = DEADLINE_EXCEEDED;
                break;
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

    return e;
}

//================================================================================
// EXTRA MILEAGE HEURISTIC
//================================================================================

ERROR_CODE h_ExtraMileage(instance* inst){
    ERROR_CODE error = OK;

    // choose the maximum distance edge (A,B) to start
    double max_distance = 0.0;
    int nodeA, nodeB;
    for(int i=0; i<inst->nnodes; i++){
        for(int j=i+1; j<inst->nnodes; j++){
            double distance = tsp_get_cost(inst, i, j);
            if(distance > max_distance){
                nodeA = i;
                nodeB = j;
                max_distance = distance;
            }
        }
    }

    log_debug("max edge : (%d, %d) with distance %f", nodeA, nodeB, max_distance);

    // initialize partial solution
    tsp_solution solution = tsp_init_solution(inst->nnodes);
    solution.cost = 2 * tsp_get_cost(inst, nodeA, nodeB);

    solution.path[nodeA] = nodeB;
    solution.path[nodeB] = nodeA;

    log_debug("initial cost: %f", solution.cost);

    // initalize visited array
    bool* visited = (bool*) calloc(inst->nnodes, sizeof(int));
    visited[nodeA] = true;
    visited[nodeB] = true;

    int num_visited = 0;
    struct edge edges[inst->nnodes];

    struct edge e1 = {.i = nodeA, .j = nodeB};
    struct edge e2 = {.i = nodeB, .j = nodeA};
    edges[num_visited++] = e1;
    edges[num_visited++] = e2;

    while(num_visited < inst->nnodes){
        // time limit check
        if(inst->options_t.timelimit != -1.0){
            double ex_time = utils_timeelapsed(inst->c);
            if(ex_time > inst->options_t.timelimit){
                error = DEADLINE_EXCEEDED;
                break;
            }
        }

        double mileage = __DBL_MAX__;
        int best_edge_idx = -1;
        int best_new_node = -1;
        struct edge best_edge;

        for(int i=0; i<inst->nnodes; i++){
            if(visited[i]){
                continue;
            }

            // find the nearest edge to the tour
            for(int j=0; j<num_visited; j++){
                int u = edges[j].i;
                int v = edges[j].j;

                double delta = tsp_get_cost(inst, u, i) + tsp_get_cost(inst, i, v) - tsp_get_cost(inst, u, v);

                if(delta < mileage){
                    mileage = delta;
                    best_edge_idx = j;
                    best_new_node = i;
                    best_edge = edges[j];
                }
            }
        }

        // no edge found so we are done
        if(best_edge_idx == -1){
            break;
        }

        // replace edge with two new edges
        // update the visited edges
        struct edge temp1 = {.i = best_edge.i, .j = best_new_node};
        solution.path[temp1.i] = temp1.j;
        edges[best_edge_idx] = temp1;

        struct edge temp2 = {.i = best_new_node, .j = best_edge.j};
        solution.path[temp2.i] = temp2.j;
        edges[num_visited++] = temp2;

        // mark the new node as visited
        visited[best_new_node] = true;

        // update cost
        solution.cost += mileage;

        log_debug("current cost: %f", solution.cost);
    }

    // save solution
    tsp_update_best_solution(inst, &solution);

    return error;
}

//================================================================================
// UTILS
//================================================================================

ERROR_CODE h_greedyutil(instance* inst, int starting_node, int* solution_path, double* solution_cost){
    if(starting_node >= inst->nnodes || starting_node < 0){
        return UNAVAILABLE;
    }

    ERROR_CODE e = OK;

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
                e = DEADLINE_EXCEEDED;
                break;
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

    return e;
}
