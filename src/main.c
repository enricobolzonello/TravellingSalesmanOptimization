#include "tsp.h"
#include "algorithms/tabusearch.h"
#include "algorithms/heuristics.h"

// TODO: better handling of errors

int main(int argc, char* argv[]){
    log_info("program started!");
    instance inst;
    ERROR_CODE e = tsp_parse_commandline(argc, argv, &inst);
    if(!err_ok(e)){
        exit(0);
    }

    if(inst.options_t.graph_input){
        tsp_read_input(&inst);
    }
    if(inst.options_t.graph_random){
        tsp_generate_randompoints(&inst);
    }

    inst.best_solution_path = (int*) calloc(inst.nnodes, sizeof(int));

    switch (inst.alg)
    {
    case ALG_GREEDY:
        e = h_Greedy(&inst);
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
            tsp_handlefatal(&inst);
        }        
        printf("Greedy from 0: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst);
        break;
    case ALG_GREEDY_ITER:
        log_info("running GREEDY-ITER");
        e = h_Greedy_iterative(&inst);
        if(!err_ok(e)){
            log_fatal("greedy iterative did not finish correctly");
            tsp_handlefatal(&inst);
        }  
        printf("Greedy from all nodes: %f\n", inst.best_solution_cost);
        log_info("Best starting node: %d", inst.starting_node);
        tsp_plot_solution(&inst);
        break;
    case ALG_2OPT_GREEDY:
        log_info("running 2OPT-GREEDY");
        e = h_greedy_2opt(&inst);
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
            tsp_handlefatal(&inst);
        } 
        printf("Greedy from 0 + 2-opt: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst);
        break;
    case ALG_TABU_SEARCH:
        log_info("running Tabu Search");
        e = tabu_search_2opt(&inst);
        if(!err_ok(e)){
            log_fatal("tabu search did not finish correctly");
            tsp_handlefatal(&inst);
        } 
        printf("Tabu search: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst);
        break;
    default:
        log_error("cannot run any algorithm");
        break;
    }

    exit(0);
}
