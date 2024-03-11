#include "tsp.h"
#include "algorithms/heuristics.h"

int main(int argc, char* argv[]){
    printf("Init\n");
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


    switch (inst.alg)
    {
    case ALG_GREEDY:
        log_info("running GREEDY");
        h_greedy(&inst, 0);
        tsp_update_best_solution(&inst);
        printf("Greedy from 0: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst, "greedy", false);
        break;
    case ALG_GREEDY_ITER:
        log_info("running GREEDY-ITER");
        h_Greedy_iterative(&inst);
        printf("Greedy from all nodes: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst, "greedy iterative", false);
        break;
    case ALG_2OPT_GREEDY:
        log_info("running 2OPT-GREEDY");
        h_greedy(&inst, 0);
        h_2opt_iterative(&inst);
        printf("Greedy from 0 + 2-opt: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst, "2-opt greedy", false);
        break;
    default:
        log_error("cannot run any algorithm");
        break;
    }

    exit(0);
}
