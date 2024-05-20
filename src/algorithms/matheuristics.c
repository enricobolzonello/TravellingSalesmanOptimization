#include "matheuristics.h"

ERROR_CODE mh_HardFixing(instance* inst){
    ERROR_CODE e = T_OK;

    log_info("running CPLEX without SECs");

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

    // WARM START
    // run all nearest neighbor heuristic
	e = h_Greedy_iterative(inst);

    // initialize the current solution as the best solution found by heuristic
    tsp_solution solution = tsp_init_solution(inst->nnodes);
    memcpy(solution.path, inst->best_solution.path, inst->nnodes * sizeof(int));
    solution.cost = inst->best_solution.cost;

    while(1){
        // check if exceeds time
        double ex_time = utils_timeelapsed(&inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               e = DEADLINE_EXCEEDED;
               break;
            }
        }

        // add solution as a MIP start
        e = hf_addMIPstart(env, lp, inst, &solution);
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

        // FIXING UNDO 
		e = hf_undofixing(env, lp, inst);
        if(!err_ok(e)){
            log_error("error in undo fixing");
            goto mh_free;
        }

    }

    e = tsp_update_best_solution(inst, &solution);
    if(!err_ok(e)){
        log_error("code %d : error in updating best solution of VNS");
    }

    mh_free:
        utils_safe_free(solution.path);

    return e;
}

ERROR_CODE hf_addMIPstart(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution) {
    ERROR_CODE e = T_OK;
    int* varindices = (int*) calloc(inst->nnodes, sizeof(int));
	double* values = (double*) calloc(inst->nnodes, sizeof(double));

	// initialize values for CPLEX
	// An entry values[j] greater than or equal to CPX_INFBOUND specifies that no value is set for the variable varindices[j]
	for(int i=0; i<inst->nnodes; i++){
		values[i] = CPX_INFBOUND;
	}

	int k = 0;
	for(int i=0; i<inst->nnodes; i++){
		int j = solution->path[i];

		varindices[k] = cx_xpos(i,j,inst);
		values[k] = 1.0;

		k++;
	}

	const int beg = 0;
	const int effortlevel = CPX_MIPSTART_NOCHECK;
	if( CPXaddmipstarts(env, lp, 1, inst->nnodes, &beg, varindices, values, &effortlevel, NULL) ){
		log_error("CPXaddmipstarts error");
		e = INTERNAL;
        goto mh_free;
	}

	log_info("heuristic solution added to MIP start");

    mh_free:
        utils_safe_free(values);
        utils_safe_free(varindices);
        return e;
}

ERROR_CODE hf_fixing(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution) {
    // TODO: set probability as parameter
    // choose E^tilde and set lb 

    ERROR_CODE e = T_OK;

    double one = 1.0;
    const char lb = 'L';
    for(int i = 0; i < inst->nnodes; i++){
        unsigned int seed = (unsigned) inst->options_t.seed;
        double prob = ( (double) rand_r(&seed) ) / RAND_MAX;

        if (prob > 0.7){
            log_debug("add edge (%d,%d) to E^tilde", i, solution->path[i]);
            int index = cx_xpos(i, solution->path[i], inst);

            if(CPXchgbds(env, lp, 1, &index, &lb, &one)){
                log_error("error in changing the bound");
                e = INTERNAL;
                goto hf_free;
            }
        }
    }

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

	// with the optimal found by CPLEX, build the corresponding solution
	cx_build_sol(xstar, inst, comp, &ncomp, solution);

    hf_free:
        utils_safe_free(xstar);
        utils_safe_free(comp);
        return e;
}
