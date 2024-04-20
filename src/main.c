#include "main.h"

ERROR_CODE runAlg(instance* inst){
    ERROR_CODE e = OK;
    inst->best_solution.path = (int*) calloc(inst->nnodes, sizeof(int));

    switch (inst->alg)
    {
    case ALG_GREEDY:
        e = h_Greedy(inst);
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
            tsp_handlefatal(inst);
        }        
        printf("Greedy from 0: %f\n", inst->best_solution.cost);
        tsp_plot_solution(inst);
        break;
    case ALG_GREEDY_ITER:
        log_info("running GREEDY-ITER");
        e = h_Greedy_iterative(inst);
        if(!err_ok(e)){
            log_fatal("greedy iterative did not finish correctly");
            tsp_handlefatal(inst);
        }  
        printf("Greedy from all nodes: %f\n", inst->best_solution.cost);
        log_info("Best starting node: %d", inst->starting_node);
        tsp_plot_solution(inst);
        break;
    case ALG_2OPT_GREEDY:
        log_info("running 2OPT-GREEDY");
        e = h_greedy_2opt(inst);
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
            tsp_handlefatal(inst);
        } 
        printf("Greedy from %d + 2-opt: %f\n", inst->starting_node, inst->best_solution.cost);
        tsp_plot_solution(inst);
        break;
    case ALG_TABU_SEARCH:
        log_info("running Tabu Search");
        e = mh_TabuSearch(inst, POL_LINEAR);
        if(!err_ok(e)){
            log_fatal("tabu search did not finish correctly");
            tsp_handlefatal(inst);
        } 
        printf("Tabu search: %f\n", inst->best_solution.cost);
        tsp_plot_solution(inst);
        break;
    case ALG_VNS:
        log_info("running VNS");
        e = mh_VNS(inst);
        if(!err_ok(e)){
            log_fatal("VNS did not finish correctly");
            tsp_handlefatal(inst);
        } 
        printf("VNS: %f\n", inst->best_solution.cost);
        tsp_plot_solution(inst);
        break;
    case ALG_CX_NOSEC:
        log_info("running NOSEC");
        e = cx_Nosec(inst);
        if(!err_ok(e)){
            log_fatal("NOSEC did not finish correctly");
            tsp_handlefatal(inst);
        } 
        printf("No SEC: %f\n", inst->best_solution.cost);
        tsp_plot_solution(inst);
        break;
    case ALG_CX_BENDERS:
        log_info("running BENDERS LOOP");
        e = cx_BendersLoop(inst, false);
        if(!err_ok(e)){
            log_fatal("Benders Loop did not finish correctly");
            tsp_handlefatal(inst);
        } 
        printf("Benders Loop: %f\n", inst->best_solution.cost);
        tsp_plot_solution(inst);
        break;
    case ALG_CX_BENDERS_PAT:
        log_info("running BENDERS LOOP with PATCHING");
        e = cx_BendersLoop(inst, true);
        if(!err_ok(e)){
            log_fatal("Benders Loop with patching did not finish correctly");
            tsp_handlefatal(inst);
        } 
        printf("Benders Loop: %f\n", inst->best_solution.cost);
        //tsp_plot_solution(inst);
        break;
    case ALG_EXTRAMILEAGE:
        log_info("running EXTRA MILEAGE");
        e = h_ExtraMileage(inst);
        if(!err_ok(e)){
            log_fatal("Extra Mileage did not finish correctly");
            tsp_handlefatal(inst);
        } 
        printf("Extra Mileage: %f\n", inst->best_solution.cost);
        tsp_plot_solution(inst);
        break;
    case ALG_CX_BRANCH_AND_CUT:
        log_info("running Branch and Cut");
        e = cx_BranchAndCut(inst);
        if(!err_ok(e)){
            log_fatal("CPLEX Branch and Cut did not finish correctly");
            tsp_handlefatal(inst);
        } 
        printf("CPLEX Branch and Cut: %f\n", inst->best_solution.cost);
        break;
    default:
        log_error("cannot run any algorithm");
        break;
    }

    return e;
}

return_struct* webapp_run(const char* path, int seed, int time_limit, int alg){
    log_info("webapp_run started!");
    instance inst;
    return_struct* rs = malloc(sizeof(return_struct));

    ERROR_CODE e;
    tsp_init(&inst);

    // start options

    if(seed != -1){
        inst.options_t.seed = seed;
    }

    if(time_limit > 0){
        inst.options_t.timelimit = time_limit;
    }

    inst.alg = alg;

    err_setverbosity(VERY_VERBOSE);
    inst.options_t.tofile = true;

    // use files
    if(!utils_file_exists(path)){
                log_fatal("%s : file does not exist", path);
                tsp_handlefatal(&inst);
            }

    inst.options_t.inputfile = (char*) calloc(strlen(path), sizeof(char));
    strcpy(inst.options_t.inputfile, path);


    inst.options_t.graph_input = true;

    if(inst.options_t.graph_input){
        tsp_read_input(&inst);
    }

    // end options

    // run selected algorithm
    e = runAlg(&inst);
    if(!err_ok(e)){
        log_error("error while running the algorithm");
        return rs;
    }

    double ex_time = utils_timeelapsed(inst.c);

    // save result in a struct to be processed by Node-Addon-API
    rs->nnodes = inst.nnodes;
    
    rs->cost = inst.best_solution.cost;

    rs->path = (int*)calloc(inst.nnodes, sizeof(int));
    for(int i=0; i<inst.nnodes; i++){
        rs->path[i] = inst.best_solution.path[i];
    }

    rs->points = (point*) calloc(inst.nnodes, sizeof(point));
    for(int i=0; i<inst.nnodes; i++){
        rs->points[i] = inst.points[i];
    }

    rs->execution_time = ex_time;

    return rs;
}


// TODO: better handling of errors

int main(int argc, char* argv[]){
    log_info("program started!");
    instance inst;
    ERROR_CODE e = tsp_parse_commandline(argc, argv, &inst);
    if(!err_ok(e)){
        log_error("error in command line parsing, error code: %d", e);
        tsp_free_instance(&inst);
        exit(0);
    }

    if(inst.options_t.graph_input){
        tsp_read_input(&inst);
    }
    if(inst.options_t.graph_random){
        tsp_generate_randompoints(&inst);
    }

    e = runAlg(&inst);

    double ex_time = utils_timeelapsed(inst.c);
    log_info("execution time %.4f seconds", ex_time);

    tsp_free_instance(&inst);

    exit(0);
}
