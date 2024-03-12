#include "tsp.h"
#include "algorithms/heuristics.h"

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


    switch (inst.alg)
    {
    case ALG_GREEDY:
        e = h_Greedy(&inst);
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
            tsp_handlefatal(&inst);
        }        
        printf("Greedy from 0: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst, "greedy", false);
        break;
    case ALG_GREEDY_ITER:
        log_info("running GREEDY-ITER");
        e = h_Greedy_iterative(&inst);
        if(!err_ok(e)){
            log_fatal("greedy iterative did not finish correctly");
            tsp_handlefatal(&inst);
        }  
        printf("Greedy from all nodes: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst, "greedy iterative", false);
        break;
    case ALG_2OPT_GREEDY:
        log_info("running 2OPT-GREEDY");
        e = h_Greedy(&inst);
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
            tsp_handlefatal(&inst);
        }  
        log_info("finished greedy, starting 2opt");
        e = h_2opt(&inst);
        if(!err_ok(e)){
            log_fatal("2opt did not finish correctly");
            tsp_handlefatal(&inst);
        }  
        printf("Greedy from 0 + 2-opt: %f\n", inst.best_solution_cost);
        tsp_plot_solution(&inst, "2-opt greedy", false);
        break;
    default:
        log_error("cannot run any algorithm");
        break;
    }

    exit(0);
}
