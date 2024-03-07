#include "tsp.h"

int main(int argc, char* argv[]){
    instance inst;
    tsp_parse_commandline(argc, argv, &inst);

    if(inst.options_t.graph_input){
        tsp_read_input(&inst);
    }
    if(inst.options_t.graph_random){
        tsp_generate_randompoints(&inst);
    }
    
    tsp_plot_points(&inst, "graph_test", true);
    exit(0);
}
