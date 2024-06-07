#include "heuristics.h"

struct edge{
    int i;
    int j;
};

//================================================================================
// NEAREST NEIGHBOUR HEURISTIC
//================================================================================

ERROR_CODE h_Greedy(){

    log_info("running Nearest Neighbour");

    tsp_solution solution;
	tsp_init_solution(tsp_inst.nnodes, &solution);

    log_info("running GREEDY");
    ERROR_CODE error = h_greedyutil( tsp_inst.starting_node, &solution, tsp_inst.costs);
    if(!err_ok(error)){
        log_error("code %d : greedy did not finish correctly", error);
    }

    error = tsp_update_best_solution( &solution);
    if(!err_ok(error)){
        log_error("code %d : error in updating solution for greedy", error);
    }

    //utils_safe_free(solution.path);
    return error;
}

ERROR_CODE h_Greedy_iterative(){

    log_info("running All Nearest Neighbour");

    ERROR_CODE e = T_OK;
    tsp_solution solution;
	tsp_init_solution(tsp_inst.nnodes, &solution);

    for(int i=0; i<tsp_inst.nnodes; i++){
        if(tsp_env.timelimit != -1.0){
            double ex_time = utils_timeelapsed(&tsp_inst.c);
            if(ex_time > tsp_env.timelimit){
                e = DEADLINE_EXCEEDED;
                break;
            }
        }

        log_debug("starting greedy with node %d", i);
        ERROR_CODE error = h_greedyutil( i, &solution, tsp_inst.costs);
        if(!err_ok(error)){
            log_error("code %d : error in iteration %d of greedy iterative", error, i);
            continue;
        }

        if(solution.cost < tsp_inst.best_solution.cost){
            log_info("found new best, node %d", i);
            tsp_inst.starting_node = i;
            error = tsp_update_best_solution( &solution);
            if(!err_ok(error)){
                log_error("code %d : error in updating best solution of greedy iterative in iteration %d", error, i);
                continue;
            }
        }
    }

    utils_safe_free(solution.path);

    return e;
}

ERROR_CODE h_greedy_2opt(){

    log_info("running All Nearest Neighbour + 2OPT");

    ERROR_CODE e = T_OK;
    tsp_solution solution;
	tsp_init_solution(tsp_inst.nnodes, &solution);

    for(int i=0; i<tsp_inst.nnodes; i++){
        if(tsp_env.timelimit != -1.0){
            double ex_time = utils_timeelapsed(&tsp_inst.c);
            if(ex_time > tsp_env.timelimit){
                log_warn("time limit exceeded in greedy 2opt");
                e = DEADLINE_EXCEEDED;
                break;
            }
        }

        log_debug("starting greedy with node %d", i);
        ERROR_CODE error = h_greedyutil(i, &solution, tsp_inst.costs);
        if(!err_ok(error)){
            log_error("code %d : error in iteration %i of 2opt greedy", error, i);
            break;
        }

        log_debug("greedy solution: cost: %f", solution.cost);

        error = ref_2opt( &solution, tsp_inst.costs, true);
        if(!err_ok(error)){
            log_error("code %d : error in 2opt", error);
            break;
        }else if (error == T_OK)
        {
            log_debug("found new best solution: starting node %d, cost %f", i, tsp_inst.best_solution.cost);
            tsp_inst.starting_node = i;
        }
        
    }

    utils_safe_free(solution.path);

    return e;
}

ERROR_CODE h_Greedy_2opt_mod_costs( tsp_solution* solution, double* costs){

    ERROR_CODE e = T_OK;
    //tsp_solution solution = tsp_init_solution(tsp_inst.nnodes);

    for(int i=0; i<tsp_inst.nnodes; i++){
        if(tsp_env.timelimit != -1.0){
            double ex_time = utils_timeelapsed(&tsp_inst.c);
            if(ex_time > tsp_env.timelimit){
                e = DEADLINE_EXCEEDED;
                break;
            }
        }

        //log_debug("starting greedy with node %d", i);
        ERROR_CODE error = h_greedyutil( i, solution, costs);
        if(!err_ok(error)){
            log_error("code %d : error in iteration %d of greedy iterative", error, i);
            continue;
        }

        error = ref_2opt( solution, costs, false);
        if(!err_ok(error)){
            log_error("code %d : error in 2opt", error);
            continue;
        }        


    }

    return e;
}


//================================================================================
// EXTRA MILEAGE HEURISTIC
//================================================================================

ERROR_CODE h_ExtraMileage(){

    log_info("running Extra Mileage");

    ERROR_CODE error = T_OK;

    double max_distance = 0.0;
    int nodeA = 0; 
    int nodeB = 1;

    switch (tsp_env.mileage_init)
    {
    case EM_MAX:
        for(int i=0; i<tsp_inst.nnodes; i++){
            for(int j=i+1; j<tsp_inst.nnodes; j++){
                double distance = tsp_get_cost( i, j);
                if(distance > max_distance){
                    nodeA = i;
                    nodeB = j;
                    max_distance = distance;
                }
            }
        }

        break;
    case EM_RANDOM:
        nodeA = rand() % (tsp_inst.nnodes + 1);
        nodeB = rand() % (tsp_inst.nnodes - nodeA + 1) + nodeA;
        break;
    default:
        log_warn("aborted");
        return ABORTED;
    }

    log_debug("max edge : (%d, %d) with distance %f", nodeA, nodeB, max_distance);

    // initialize partial solution
    tsp_solution solution;
	tsp_init_solution(tsp_inst.nnodes, &solution);
    
    solution.cost = 2 * tsp_get_cost( nodeA, nodeB);

    solution.path[nodeA] = nodeB;
    solution.path[nodeB] = nodeA;

    log_debug("initial cost: %f", solution.cost);

    // execute extra mileage algorithm
    error = h_extramileage_util( &solution, nodeA, nodeB);

    // save solution
    tsp_update_best_solution( &solution);

    return error;
}

//================================================================================
// UTILS
//================================================================================

ERROR_CODE h_greedyutil( int starting_node, tsp_solution* solution, double* costs){

    if(costs == NULL){
        log_error("matrix of costs not found");
        return INTERNAL;
    }

    if(starting_node >= tsp_inst.nnodes || starting_node < 0){
        log_error("starting node not correct");
        return UNAVAILABLE;
    }

    ERROR_CODE e = T_OK;

    int* visited = (int*)calloc(tsp_inst.nnodes, sizeof(int));

    int curr = starting_node;
    visited[curr] = 1;
    
    double sol_cost = 0;
    bool done = false;

    while(!done){
        // check that we have not exceed time limit
        double ex_time = utils_timeelapsed(&tsp_inst.c);
        if(tsp_env.timelimit != -1.0){
            if(ex_time > tsp_env.timelimit){
                log_warn("time limit exceeded in greedy util");
                e = DEADLINE_EXCEEDED;
                break;
            }
        }

        // identify minimum distance from the current node
        int min_idx = -1;
        double min_dist = __DBL_MAX__;

        for(int i=0; i<tsp_inst.nnodes; i++){
            // skip iteration if it's already visited
            if(i != curr && visited[i] != 1){
                // update the minimum cost and its node
                double temp = costs[curr * tsp_inst.nnodes + i];
                if(temp != NOT_CONNECTED && temp < min_dist){
                    min_dist = temp;
                    min_idx = i;
                }
            }
        }

        // save the edge
        solution->path[curr] = min_idx;

        if(min_idx == -1){
            // we have visited all nodes
            // close the path
            solution->path[curr] = starting_node;
            done = true;
        }else{
            // mark the node as visited and update the cost of the solution
            visited[min_idx] = 1;
            curr = min_idx;
            sol_cost += min_dist;
        }
    }

    // add last edge
    sol_cost += costs[curr * tsp_inst.nnodes + starting_node];
    solution->cost = sol_cost;

    utils_safe_free(visited);

    return e;
}

ERROR_CODE h_extramileage_util( tsp_solution* solution, int nodeA, int nodeB){
    ERROR_CODE error = T_OK;    

    // initalize visited array
    bool* visited = (bool*) calloc(tsp_inst.nnodes, sizeof(int));
    visited[nodeA] = true;
    visited[nodeB] = true;

    int num_visited = 0;
    struct edge edges[tsp_inst.nnodes];

    struct edge e1 = {.i = nodeA, .j = nodeB};
    struct edge e2 = {.i = nodeB, .j = nodeA};
    edges[num_visited++] = e1;
    edges[num_visited++] = e2;

    while(num_visited < tsp_inst.nnodes){
        // time limit check
        if(tsp_env.timelimit != -1.0){
            double ex_time = utils_timeelapsed(&tsp_inst.c);
            if(ex_time > tsp_env.timelimit){
                error = DEADLINE_EXCEEDED;
                break;
            }
        }

        double mileage = __DBL_MAX__;
        int best_edge_idx = -1;
        int best_new_node = -1;
        struct edge best_edge;

        for(int i=0; i<tsp_inst.nnodes; i++){
            if(visited[i]){
                continue;
            }

            // find the nearest edge to the tour
            for(int j=0; j<num_visited; j++){
                int u = edges[j].i;
                int v = edges[j].j;

                double delta = tsp_get_cost( u, i) + tsp_get_cost( i, v) - tsp_get_cost( u, v);

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
        solution->path[temp1.i] = temp1.j;
        edges[best_edge_idx] = temp1;

        struct edge temp2 = {.i = best_new_node, .j = best_edge.j};
        solution->path[temp2.i] = temp2.j;
        edges[num_visited++] = temp2;

        // mark the new node as visited
        visited[best_new_node] = true;

        // update cost
        solution->cost += mileage;

        log_debug("current cost: %f", solution->cost);
    }

    return error;
}
