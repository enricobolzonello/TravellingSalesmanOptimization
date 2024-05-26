#include "main.h"

ERROR_CODE runAlg(instance* inst){
    ERROR_CODE e = T_OK;
    inst->best_solution.path = (int*) calloc(inst->nnodes, sizeof(int));

    switch (inst->alg)
    {
    case ALG_GREEDY:
        e = h_Greedy(inst);
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
        }        
        break;
    case ALG_GREEDY_ITER:
        e = h_Greedy_iterative(inst);
        if(!err_ok(e)){
            log_fatal("greedy iterative did not finish correctly");
        }  
        log_info("Best starting node: %d", inst->starting_node);
        break;
    case ALG_2OPT_GREEDY:
        e = h_greedy_2opt(inst);
        if(!err_ok(e)){
            log_fatal("greedy did not finish correctly");
        } 
        break;
    case ALG_TABU_SEARCH:
        e = mh_TabuSearch(inst);
        if(!err_ok(e)){
            log_fatal("tabu search did not finish correctly");
        } 
        break;
    case ALG_VNS:
        e = mh_VNS(inst);
        if(!err_ok(e)){
            log_fatal("VNS did not finish correctly");
        } 
        break;
    case ALG_CX_NOSEC:
        e = cx_Nosec(inst);
        if(!err_ok(e)){
            log_fatal("NOSEC did not finish correctly");
        } 
        break;
    case ALG_CX_BENDERS:
        e = cx_BendersLoop(inst, false);
        if(!err_ok(e)){
            log_fatal("Benders Loop did not finish correctly");
        } 
        break;
    case ALG_CX_BENDERS_PAT:
        e = cx_BendersLoop(inst, true);
        if(!err_ok(e)){
            log_fatal("Benders Loop with patching did not finish correctly");
        } 
        break;
    case ALG_EXTRAMILEAGE:
        e = h_ExtraMileage(inst);
        if(!err_ok(e)){
            log_fatal("Extra Mileage did not finish correctly");
        } 
        break;
    case ALG_CX_BRANCH_AND_CUT:
        e = cx_BranchAndCut(inst);
        if(!err_ok(e)){
            log_fatal("CPLEX Branch and Cut did not finish correctly");
        }
        break;
    case ALG_HARD_FIXING:
        e = mh_HardFixing(inst);
        if(!err_ok(e)){
            log_fatal("Hard Fixing did not finish correctly");
        }
        break;
    case ALG_LOCAL_BRANCHING:
        e = mh_LocalBranching(inst);
        if(!err_ok(e)){
            log_fatal("Local branching did not finish correctly");
        }
        break;
    default:
        log_error("cannot run any algorithm");
        break;
    }

    //if(err_ok(e)){
    //    tsp_plot_solution(inst);
    //}

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

    double ex_time = utils_timeelapsed(&inst.c);

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


int main(int argc, char* argv[]){
    instance inst;
    ERROR_CODE e = tsp_parse_commandline(argc, argv, &inst);
    if(!err_ok(e)){
        log_error("error in command line parsing, error code: %d", e);
        tsp_free_instance(&inst);
        exit(EXIT_FAILURE);
    }

    if(inst.options_t.graph_input){
        tsp_read_input(&inst);
    }
    if(inst.options_t.graph_random){
        tsp_generate_randompoints(&inst);
    }

    err_setinfo(inst.alg, inst.nnodes, inst.options_t.graph_random, inst.options_t.inputfile, inst.options_t.timelimit, inst.options_t.seed, inst.options_t.policy, inst.options_t.mileage_init, inst.options_t.init_mip, inst.options_t.skip_policy, inst.options_t.callback_relaxation);


    // start the clock (measures only algorithm time)
    utils_startclock(&inst.c);

    e = runAlg(&inst);
    if(!err_ok(e) && e != DEADLINE_EXCEEDED){
        log_warn("error detected, shutting down application");
        tsp_free_instance(&inst);
        return EXIT_FAILURE;
    }
    
    double ex_time = utils_timeelapsed(&inst.c);
    err_printoutput(inst.best_solution.cost, ex_time, inst.alg);

    tsp_free_instance(&inst);
    
    return EXIT_SUCCESS;
}
