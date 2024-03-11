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

    if (strncmp(inst.algorithm, "GREEDY-ONCE", 11) == 0) {
        printf("GREEDY-ONCE method selected.\n");
        h_greedy(&inst, 0);
        tsp_update_best_solution(&inst);
        log_trace("Greedy from 0: %f\n", inst.best_solution_cost);
        printf("Greedy from 0: %f\n", inst.best_solution_cost);
    } else if (strncmp(inst.algorithm, "GREEDY-ALLNODES", 15) == 0) {
        printf("GREEDY-ALLNODES method selected.\n");
        h_Greedy_iterative(&inst);
        log_trace("Greedy from all nodes: %f\n", inst.best_solution_cost);
        printf("Greedy from all nodes: %f\n", inst.best_solution_cost);
    } else if (strncmp(inst.algorithm, "2OPT-GREEDY", 11) == 0) {
        printf("2OPT-GREEDY method selected.\n");
        h_greedy(&inst, 0);
        h_2opt_iterative(&inst);
        log_trace("Greedy from 0 + 2-opt: %f\n", inst.best_solution_cost);
        printf("Greedy from 0 + 2-opt: %f\n", inst.best_solution_cost);
    }

    exit(0);
}
