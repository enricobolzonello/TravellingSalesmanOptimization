#include "matheuristics.h"

ERROR_CODE mh_HardFixing(instance* inst){
    ERROR_CODE e = T_OK;

    log_info("running Hard Fixing");

	// open CPLEX model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	if ( error ){
		log_fatal("CPX code %d : CPXopenCPLEX() error", error);
		e = FAILED_PRECONDITION;
		goto mh_free;
	} 
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1"); 
	if ( error ) {
		log_fatal("CPX code %d : CPXcreateprob() error", error);
		e = FAILED_PRECONDITION;
		goto mh_free;
	}

	// initialize CPLEX model
	e = cx_initialize(inst, env, lp);
	if(!err_ok(e)){
		log_error("error in initializing cplex model");
		e = FAILED_PRECONDITION;
		goto mh_free;
	}

	log_info("CPLEX initialized correctly");

    // initialize the current solution as the best solution found by heuristic
    tsp_solution solution = tsp_init_solution(inst->nnodes);
    memcpy(solution.path, inst->best_solution.path, inst->nnodes * sizeof(int));
    solution.cost = inst->best_solution.cost;

    while(1){
        // check if exceeds time
        double ex_time = utils_timeelapsed(&inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               log_warn("deadline exceeded in hard fixing");
               e = DEADLINE_EXCEEDED;
               break;
            }
        }

        // add solution as a MIP start
        e = cx_add_mip_starts(env, lp, inst, &solution);
        if(!err_ok(e)){
            log_error("error add mip start");
            goto mh_free;
        }

        // set remaining time limit for cplex
        // 1/10 of the total remaining time
        double time_remain = (inst->options_t.timelimit - ex_time) / 10;
        if(CPXsetdblparam(env, CPXPARAM_TimeLimit, time_remain)){
            log_error("error setting cplex time limit");
            goto mh_free;
        }
        
        log_debug("time assigned to mip solver : %.4f", time_remain);

        // FIXING
        e = hf_fixing(env, lp, inst, &solution);
        if(!err_ok(e)){
            log_error("error in fixing");
            goto mh_free;
        }

        // MIP SOLVER
        e = hf_mipsolver(env, lp, inst, &solution);
        if(!err_ok(e)){
            log_error("error in mip solver");
            goto mh_free;
        }

        log_info("MIP SOLVER DONE");

        e = tsp_update_best_solution(inst, &solution);
        if(!err_ok(e)){
            log_error("code %d : error in updating best solution of Hard Fixing");
        }

        // FIXING UNDO 
		e = hf_undofixing(env, lp, inst);
        if(!err_ok(e)){
            log_error("error in undo fixing");
            goto mh_free;
        }

    }

    mh_free:
        utils_safe_free(solution.path);

    return e;
}

ERROR_CODE hf_fixing(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution) {
    // TODO: set probability as parameter
    // choose E^tilde and set lb 

    ERROR_CODE e = T_OK;
    int k = 0;

    double one = 1.0;
    const char lb = 'L';
    for(int i = 0; i < inst->nnodes; i++){
        //unsigned int seed = (unsigned) inst->options_t.seed;
        double prob = ( (double) rand() ) / RAND_MAX;

        if (prob < inst->options_t.hf_prob){
            k++;
            log_debug("add edge (%d,%d) to E^tilde", i, solution->path[i]);
            int index = cx_xpos(i, solution->path[i], inst);

            if(CPXchgbds(env, lp, 1, &index, &lb, &one)){
                log_error("error in changing the bound");
                e = INTERNAL;
                goto hf_free;
            }
        }
    }

    log_info("EDGES ADDED: %d\n", k);

    hf_free:
        return e;
}

ERROR_CODE hf_undofixing(CPXENVptr env, CPXLPptr lp, instance* inst) {
    ERROR_CODE e = T_OK;

    double zero = 0.0;
    const char lb = 'L';
	for (int i = 0; i < inst->ncols; i++) {
		int pos = i;
		if (CPXchgbds(env, lp, 1, &pos, &lb, &zero)){
            log_error("error in changing the bound");
            e = INTERNAL;
            goto hf_free;
        }
	}

    hf_free:
        return e;
}

ERROR_CODE hf_mipsolver(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution) {
    ERROR_CODE e = T_OK;

    double* xstar = (double *) calloc(inst->ncols, sizeof(double));
    int ncomp = 0;
	int* comp = (int*) calloc(inst->ncols, sizeof(int));

    e = cx_branchcut_util(env, lp, inst, inst->ncols, xstar);
    if(!err_ok(e)){
        log_error("error in branch&cut util");
        goto hf_free;
    }

	// with the solution found by CPLEX, build the corresponding solution
	cx_build_sol(xstar, inst, comp, &ncomp, solution);

    hf_free:
        utils_safe_free(xstar);
        utils_safe_free(comp);
        return e;
}
