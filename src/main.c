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
    
    tsp_plot_points(&inst, "graph test", false);
    exit(0);
}
