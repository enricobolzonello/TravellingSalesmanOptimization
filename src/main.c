#include "algorithms/matheuristics.h"
#include "algorithms/metaheuristic.h"

ERROR_CODE tsp_run_algorithm(){
    ERROR_CODE e = T_OK;
    tsp_inst.best_solution.path = (int*) calloc(tsp_inst.nnodes, sizeof(int));

    switch (tsp_inst.alg)
    {
    case ALG_GREEDY:
        e = h_Greedy();
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
        }        
        break;
    case ALG_GREEDY_ITER:
        e = h_Greedy_iterative();
        if(!err_ok(e)){
            log_fatal("greedy iterative did not finish correctly");
        }  
        log_info("Best starting node: %d", tsp_inst.starting_node);
        break;
    case ALG_2OPT_GREEDY:
        e = h_greedy_2opt();
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
        } 
        break;
    case ALG_TABU_SEARCH:
        e = mh_TabuSearch();
        if(!err_ok(e)){
            log_fatal("tabu search did not finish correctly");
        } 
        break;
    case ALG_VNS:
        e = mh_VNS();
        if(!err_ok(e)){
            log_fatal("VNS did not finish correctly");
        } 
        break;
    case ALG_CX_NOSEC:
        e = cx_Nosec();
        if(!err_ok(e)){
            log_fatal("NOSEC did not finish correctly");
        } 
        break;
    case ALG_CX_BENDERS:
        e = cx_BendersLoop(tsp_env.bl_patching);
        if(!err_ok(e)){
            log_fatal("Benders Loop did not finish correctly");
        } 
        break;
    case ALG_EXTRAMILEAGE:
        e = h_ExtraMileage();
        if(!err_ok(e)){
            log_fatal("Extra Mileage did not finish correctly");
        } 
        break;
    case ALG_CX_BRANCH_AND_CUT:
        e = cx_BranchAndCut();
        if(!err_ok(e)){
            log_fatal("CPLEX Branch and Cut did not finish correctly");
        }
        break;
    case ALG_HARD_FIXING:
        e = mh_HardFixing();
        if(!err_ok(e)){
            log_fatal("Hard Fixing did not finish correctly");
        }
        break;
    case ALG_LOCAL_BRANCHING:
        e = mh_LocalBranching();
        if(!err_ok(e)){
            log_fatal("Local branching did not finish correctly");
        }
        break;
    default:
        log_error("cannot run any algorithm");
        break;
    }

    if(err_ok(e)){
        tsp_plot_solution();
    }

    return e;
}

return_struct* tsp_webapp_run(const char* path, int seed, int time_limit, int alg){
    log_info("webapp_run started!");
    return_struct* rs = malloc(sizeof(return_struct));

    ERROR_CODE e;
    tsp_init();

    // start options

    if(seed != -1){
        tsp_env.seed = seed;
    }

    if(time_limit > 0){
        tsp_env.timelimit = time_limit;
    }

    tsp_inst.alg = alg;

    err_setverbosity(VERY_VERBOSE);
    tsp_env.tofile = true;

    // use files
    if(!utils_file_exists(path)){
                log_fatal("%s : file does not exist", path);
                tsp_handlefatal();
            }

    tsp_env.inputfile = (char*) calloc(strlen(path), sizeof(char));
    strcpy(tsp_env.inputfile, path);


    tsp_env.graph_input = true;

    if(tsp_env.graph_input){
        tsp_read_input();
    }

    // end options

    // run selected algorithm
    e = tsp_run_algorithm();
    if(!err_ok(e)){
        log_error("error while running the algorithm");
        return rs;
    }

    double ex_time = utils_timeelapsed(&tsp_inst.c);

    // save result in a struct to be processed by Node-Addon-API
    rs->nnodes = tsp_inst.nnodes;
    
    rs->cost = tsp_inst.best_solution.cost;

    rs->path = (int*)calloc(tsp_inst.nnodes, sizeof(int));
    for(int i=0; i<tsp_inst.nnodes; i++){
        rs->path[i] = tsp_inst.best_solution.path[i];
    }

    rs->points = (point*) calloc(tsp_inst.nnodes, sizeof(point));
    for(int i=0; i<tsp_inst.nnodes; i++){
        rs->points[i] = tsp_inst.points[i];
    }

    rs->execution_time = ex_time;

    return rs;
}

int main(int argc, char* argv[]){
    ERROR_CODE e = tsp_parse_commandline(argc, argv);
    if(!err_ok(e)){
        log_error("error in command line parsing, error code: %d", e);
        tsp_free_instance();
        exit(EXIT_FAILURE);
    }

    if(tsp_env.graph_input){
        tsp_read_input();
    }
    if(tsp_env.graph_random){
        tsp_generate_randompoints();
    }

    err_setinfo(tsp_inst.alg, tsp_inst.nnodes, tsp_env.graph_random, tsp_env.inputfile, tsp_env.timelimit, tsp_env.seed, tsp_env.policy, tsp_env.mileage_init, tsp_env.init_mip, tsp_env.skip_policy, tsp_env.callback_relaxation, tsp_env.lb_improv, tsp_env.lb_delta, tsp_env.lb_kstar);


    // start the clock (measures only algorithm time)
    utils_startclock(&tsp_inst.c);

    e = tsp_run_algorithm();
    if(!err_ok(e) && e != DEADLINE_EXCEEDED){
        log_warn("error detected, shutting down application");
        tsp_free_instance();
        return EXIT_FAILURE;
    }
    
    double ex_time = utils_timeelapsed(&tsp_inst.c);
    err_printoutput(tsp_inst.best_solution.cost, ex_time, tsp_inst.alg);

    tsp_free_instance();
    
    return EXIT_SUCCESS;
}
