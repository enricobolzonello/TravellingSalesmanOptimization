#include "tsp.h"
#include "algorithms/heuristics.h"

int main(int argc, char* argv[]){
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
    
    h_greedy(&inst, 0);
    log_trace("from 0: %f\n", inst.best_solution_cost);

    h_Greedy_iterative(&inst);
    log_trace("from 0: %f\n", inst.best_solution_cost);

    exit(0);
}
