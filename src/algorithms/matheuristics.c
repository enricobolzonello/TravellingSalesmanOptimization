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

    int i=0;
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

        log_info("iteration: %d", i);

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

        i++;
    }

    mh_free:
        utils_safe_free(solution.path);

        // free and close cplex model   
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env); 

    return e;
}

ERROR_CODE hf_fixing(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution) {
    // choose E^tilde and set lb 

    char* message = "executing fixing";
    log_info(message);

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

    log_status(message);

    log_info("edges added: %d\n", k);

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

    log_info("cost: %.2f", solution->cost);

    hf_free:
        utils_safe_free(xstar);
        utils_safe_free(comp);
        return e;
}

ERROR_CODE mh_LocalBranching(instance* inst){
    ERROR_CODE e = T_OK;

    log_info("running Local Branching");

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

    int K = 10;

    tsp_solution solution = tsp_init_solution(inst->nnodes);
    // initialize the current solution as the best solution found by heuristic
    memcpy(solution.path, inst->best_solution.path, inst->nnodes * sizeof(int));
    solution.cost = inst->best_solution.cost;

    if(inst->options_t.lb_kstar){

        int Kstar = 0;

        // ESTENSIONE

        // 0) get the heuristic solution xh (must be done when the model is MIP)
        /*
            status = CPXgetmipstarts (env, lp, &nzcnt, beg, varindices,
                                    values, effortlevel, startspace,
                                    &surplus, 0, cur_numstarts-1);
        */

       int nzcnt = inst->ncols;
       int beg;
       int* varindices = (int*) calloc(inst->ncols, sizeof(int));
       double* values = (double*) calloc(inst->ncols, sizeof(double));
       int effortlevel;
       int startspace = inst->ncols;
       int surplus;

        int error = CPXgetmipstarts(env, lp, &nzcnt, &beg, varindices, values, &effortlevel, inst->ncols, &surplus, 0, 0);
        if(error){
            log_error("error code: %d, surplus: %d", error, surplus);
            e = INTERNAL;
            goto mh_free;
        }

        // 1) LP relaxation

        // 1.1) change variable types
        int* indices = (int*) calloc(inst->ncols, sizeof(int));
        char* xctype = (char*) calloc(inst->ncols, sizeof(char));

        for(int i=0; i<inst->ncols; i++){
            indices[i] = i;
            xctype[i] = 'C';
        }

        if(CPXchgctype(env, lp, inst->ncols, indices, xctype)){
            log_error("error in chgctype");
            e = INTERNAL;
            goto mh_free;
        }

        // 1.2) change the current problem to a related problem
        if(CPXchgprobtype(env, lp, CPXPROB_LP)){
            log_error("error in chgprobtype");
            e = INTERNAL;
            goto mh_free;
        }

        // 2) solve lp relaxation
        if(CPXlpopt(env, lp)){
            log_error("error in lpopt");
            e = INTERNAL;
            goto mh_free;
        }

        // 3) get xstar
        // check that cplex solved it right
        double* xstar = (double*) calloc(inst->ncols, sizeof(double));
        if ( CPXgetx(env, lp, xstar, 0, inst->ncols-1) ){
            log_fatal("CPX : CPXgetx() error");	
            goto mh_free;
        } 

        // 4) compute kstar
        double temp = 0.0;

        for(int i=0; i<nzcnt; i++){
            if(values[i] > 0.5){
                // positions should match
                temp += (1 - xstar[i]);
            }
        }

        // 5) revert to MIP
        for(int i=0; i<inst->ncols; i++){
            indices[i] = i;
            xctype[i] = 'B';
        }

        if(CPXchgctype(env, lp, inst->ncols, indices, xctype)){
            log_error("error in chgctype");
            e = INTERNAL;
            goto mh_free;
        }

        if(CPXchgprobtype(env, lp, CPXPROB_MILP)){
            log_error("error in chgprobtype");
            e = INTERNAL;
            goto mh_free;
        }

        Kstar = (int) (temp + 0.5);

        log_info("K-star: %d", Kstar);

        // K = average between 0 and Kstar
        K = Kstar / 2;
    }

    log_info("K: %d", K);

    double objval;
    double objbest = __DBL_MAX__;

    int st_counter = 0;

    while(1){
        // check if exceeds time
        double ex_time = utils_timeelapsed(&inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               log_warn("deadline exceeded in local branching");
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

        // Add LB constraint
        e = lb_add_constraint(env, lp, inst, &solution, K);
        if(!err_ok(e)){
            log_error("error in adding local braching constraint");
            goto mh_free;
        }

        // set remaining time limit for cplex
        // 1/10 of the total remaining time
        double time_remain = (inst->options_t.timelimit - ex_time) / 3;
        if(CPXsetdblparam(env, CPXPARAM_TimeLimit, time_remain)){
            log_error("error setting cplex time limit");
            goto mh_free;
        }
        
        log_info("time assigned to mip solver : %.4f", time_remain);

        // MIP SOLVER
        e = hf_mipsolver(env, lp, inst, &solution);
        if(!err_ok(e)){
            log_error("error in mip solver");
            goto mh_free;
        }

        log_info("MIP SOLVER DONE");

        if(CPXgetobjval(env, lp, &objval)){
            log_error("there is no solution");
            e = INTERNAL;
            goto mh_free;
        }

        // save the best solution
        e = tsp_update_best_solution(inst, &solution);
        if(!err_ok(e)){
            log_error("code %d : error in updating best solution of Hard Fixing");
        }

        // update k
        // POLICY
        // keep a stagnation counter to count how many iterations do not improve the solution, if it becomes greater than a threshold, increase k
        // 
        // if the new solution is better:
        //  if the improvement is not much (for now 2%), we increase k to generate deeper cuts
        //  otherwise we stay at the k we are in

        if(objval < objbest){

            double improvement = 1.0 - objval / objbest;

            if(improvement < inst->options_t.lb_improv){
                K += inst->options_t.lb_delta;
            }else{
                K = max(K - inst->options_t.lb_delta, 10);
            }
            
        }else{
            st_counter++;

            if(st_counter <= STAGNATION_THRESHOLD){
                st_counter = 0;
                K += inst->options_t.lb_delta;
            }
        }

        // Remove LB constraint
        e = lb_remove_constraint(env, lp);
        if(!err_ok(e)){
            log_error("error in removing local braching constraint");
            goto mh_free;
        }
    }

    mh_free:
        utils_safe_free(solution.path);

        // free and close cplex model   
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env); 

    return e;
}

ERROR_CODE lb_add_constraint(CPXENVptr env, CPXLPptr lp, instance *inst, tsp_solution *solution, int k){
    ERROR_CODE e = T_OK;

    int* index = (int*) calloc(inst->ncols, sizeof(int));
    double* value = (double*) calloc(inst->ncols, sizeof(double));
    int nnz = 0;
    for(int i=0; i<inst->nnodes; i++){
		index[nnz] = cx_xpos(i,solution->path[i],inst);
		value[nnz] = 1.0;
		nnz++;
	}

    double rhs = inst->nnodes - k;

    int izero = 0;
    const char sense='G';

    if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, NULL)){
        e = INTERNAL;
        log_error("error in CPXaddrows");
    } 


    utils_safe_free(index);
    utils_safe_free(value);

    return e;
}


ERROR_CODE lb_remove_constraint(CPXENVptr env, CPXLPptr lp){
    ERROR_CODE e = T_OK;

    int num_rows = CPXgetnumrows(env, lp);

    if (CPXdelrows(env, lp, num_rows-1, num_rows-1)){
        e = INTERNAL;
        log_error("error in CPXdelrows");
    }

    return e;
}
