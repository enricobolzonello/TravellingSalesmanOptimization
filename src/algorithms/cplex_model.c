#include "cplex_model.h"

ERROR_CODE cx_Nosec(instance *inst){  

	ERROR_CODE e = T_OK;

	log_info("running CPLEX without SECs");

	// open CPLEX model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	if ( error ){
		log_fatal("CPX code %d : CPXopenCPLEX() error", error);
		e = FAILED_PRECONDITION;
		goto cx_free;
	} 
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1"); 
	if ( error ) {
		log_fatal("CPX code %d : CPXcreateprob() error", error);
		e = FAILED_PRECONDITION;
		goto cx_free;
	}

	// initialize CPLEX model
	e = cx_initialize(inst, env, lp);
	if(!err_ok(e)){
		log_error("error in initializing cplex model");
		e = FAILED_PRECONDITION;
		goto cx_free;
	}

	log_info("CPLEX initialized correctly");

	// solve with cplex
	error = CPXmipopt(env,lp);
	if ( error ) 
	{
		log_fatal("CPX code %d : CPXmipopt() error", error); 
		e = INTERNAL;
		goto cx_free;
	}

	int ncols = CPXgetnumcols(env, lp);

	// get the optimal value
	double* xstar = (double *) calloc(ncols, sizeof(double));

	// check that cplex solved it right
	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ){
		log_fatal("CPX : CPXgetx() error");	
		e = INTERNAL;
		goto cx_free;
	} 

	e = cx_handle_cplex_status(env, lp);
	if(!err_ok(e)){
		log_error("cplex did not finish correctly, error code %d", e);
		goto cx_free;
	}

	log_info("Optimal found");

	int ncomp = 0;
	int* comp = (int*) calloc(ncols, sizeof(int));

	// with the optimal found by CPLEX, build the corresponding solution
	tsp_solution solution = tsp_init_solution(inst->nnodes);
	cx_build_sol(xstar, inst, comp, &ncomp, &solution);

	// update best solution (not with tsp_update_solution since it is not a cycle)
	memcpy(inst->best_solution.path, solution.path, inst->nnodes * sizeof(int));
    inst->best_solution.cost = solution.cost;
    log_debug("new best solution: %f", solution.cost);

	log_info("Number of independent components: %d", ncomp);

	cx_free:
		utils_safe_free(comp);
		utils_safe_free(solution.path);

		utils_safe_free(xstar);
	
		// free and close cplex model   
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env); 

	return e; 
}

ERROR_CODE cx_BendersLoop(instance* inst, bool patching){

	ERROR_CODE e = T_OK;

	log_info("running CPLEX Benders Loop %s Patching", patching ? "with" : "without");

	// open CPLEX model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	if(error){
		log_fatal("CPX code %d : CPXopenCPLEX() error", e);
		e = FAILED_PRECONDITION;
		goto cx_free;
	} 
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1"); 
	if(error){
		log_fatal("CPX code %d : CPXcreateprob() error", e);
		e = FAILED_PRECONDITION;
		goto cx_free;
	}

	// initialize CPLEX model

	e = cx_initialize(inst, env, lp);
	if(!err_ok(e)){
		log_fatal("code %d : error in initialize", e);
		goto cx_free;
	}

	log_info("CPLEX initialized correctly");

	tsp_solution solution = tsp_init_solution(inst->nnodes);
	int iteration = 0;
	while(1){
		log_info("iteration %d", iteration);
		double ex_time = utils_timeelapsed(&inst->c);
        if(inst->options_t.timelimit != -1.0){
			CPXsetdblparam(env, CPX_PARAM_TILIM, inst->options_t.timelimit - ex_time); 

            if(ex_time > inst->options_t.timelimit){
				log_warn("exceeded time, saving best solution found until now");
				e = DEADLINE_EXCEEDED;
				break;
            }
        }

		// solve with cplex
		error = CPXmipopt(env,lp);
		if ( error ){
			log_fatal("CPX code %d : CPXmipopt() error", e); 
			e = INTERNAL;
			goto cx_free;
		}

		int ncols = CPXgetnumcols(env, lp);

		// get the optimal value
		double* xstar = (double *) calloc(ncols, sizeof(double));

		// check that cplex solved it right
		if ( CPXgetx(env, lp, xstar, 0, ncols-1) ){
			log_fatal("CPX : CPXgetx() error");	
			e = INTERNAL;
			goto cx_free;
		} 

		// check cplex status code on exit
		e = cx_handle_cplex_status(env, lp);
		if(!err_ok(e)){
			log_error("cplex did not finish correctly, error code %d", e);
			goto cx_free;
		}

		int ncomp = 0;
		int* comp = (int*) calloc(ncols, sizeof(int));
		cx_build_sol(xstar, inst, comp, &ncomp, &solution);

		log_info("number of components: %d", ncomp);
		log_info("current solution cost: %f", solution.cost);
		log_info("is solution a tour? %d", isTour(solution.path, inst->nnodes));
		log_info("cost re-computed: %f", solutionCost(inst, solution.path));

		// only one component it means that we have found an Hamiltonian cycle
		if(ncomp == 1){
			break;
		}

		error = cx_add_sec(env, lp, comp, ncomp, inst);
		if(!err_ok(error)){
			log_fatal("code %d : error in add_sec", error);
			goto cx_free;
		}

		// patch solution
		if(patching){
			while(ncomp > 1){
				cx_patching(inst, comp, &ncomp, &solution);
				log_info("number of components: %d", ncomp);
				log_info("current solution cost: %f", solution.cost);
				log_info("is solution a tour? %d", isTour(solution.path, inst->nnodes));
				log_info("cost re-computed: %f", solutionCost(inst, solution.path));
			}
		}

		iteration++;

		utils_safe_free(xstar);
		utils_safe_free(comp);
	}

	log_info("Optimal found");

	// save the best solution
	tsp_update_best_solution(inst, &solution);

	cx_free:
		utils_safe_free(solution.path);
		
		// free and close cplex model   
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env); 

	return error;
}


ERROR_CODE cx_BranchAndCut(instance *inst){ 

	ERROR_CODE e = T_OK; 

	log_info("running CPLEX Branch&Cut");

	// open CPLEX model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	if ( error ){
		log_fatal("CPX code %d : CPXopenCPLEX() error", e);
		e = FAILED_PRECONDITION;
		goto cx_free;
	} 
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1"); 
	if ( error ) {
		log_fatal("CPX code %d : CPXopenCPLEX() error", e);
		e = FAILED_PRECONDITION;
		goto cx_free;
	}

	// initialize CPLEX model
	e = cx_initialize(inst, env, lp);
	if(!err_ok(e)){
		log_error("error in initialization");
		goto cx_free;
	}

	log_info("CPLEX initialized correctly");

	// initialize seeds for different threads
	// https://selkie.macalester.edu/csinparallel/modules/MonteCarloSimulationExemplar/build/html/SeedingThreads/SeedEachThread.html
	int threads = -1;
	if(CPXgetintparam(env, CPXPARAM_Threads, &threads)){
		log_error("CPXgetintparam error in threads");
		e = ABORTED;
		goto cx_free;
	}
	inst->threads_seeds = (int*) calloc(threads, sizeof(int));

	for(int i=0; i<threads; i++){
		unsigned int seed = (inst->options_t.seed == -1) ? (unsigned) time(NULL) : (unsigned) inst->options_t.seed;

		// clears the last 5 bits of seed and replace them with i+1
		// has place for 32 threads (cplex defaults to at most 32 threads or the number of cores, whetever is smaller)
 		inst->threads_seeds[i] = (seed & 0xFFFFFFE0) | (i + 1);
	}

	int ncols = CPXgetnumcols(env, lp);
	double* xstar = (double *) calloc(ncols, sizeof(double));

	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
	if(inst->options_t.callback_relaxation){
		contextid |= CPX_CALLBACKCONTEXT_RELAXATION;
	}

	if ( CPXcallbacksetfunc(env, lp, contextid, callback_branch_and_cut, inst) ) {
		log_fatal("CPXcallbacksetfunc() error");
		e = INTERNAL;
		goto cx_free;
	}

	// solve with cplex
	error = CPXmipopt(env,lp);
	log_debug("cpx opt end");
	if ( error ) 
	{
		log_fatal("CPX code %d : CPXmipopt() error", error); 
		e = INTERNAL;
		goto cx_free;
	}

	// check that cplex solved it right
	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ){
		log_fatal("CPX : CPXgetx() error");	
		CPXcloseCPLEX(&env); 
		tsp_handlefatal(inst);
	} 

	// check cplex status code on exit
	e = cx_handle_cplex_status(env, lp);
	if(!err_ok(e)){
		log_error("cplex did not finish correctly, error code %d", e);
		goto cx_free;
	}

	log_info("Optimal found");

	int ncomp = 0;
	int* comp = (int*) calloc(ncols, sizeof(int));

	// with the optimal found by CPLEX, build the corresponding solution
	tsp_solution solution = tsp_init_solution(inst->nnodes);
	cx_build_sol(xstar, inst, comp, &ncomp, &solution);

	// update best solution
	tsp_update_best_solution(inst, &solution);

	log_info("number of independent components: %d", ncomp);
	log_info("is solution a tour? %s", isTour(solution.path, inst->nnodes) ? "yes" : "no");

	utils_safe_free(comp);

	cx_free:
		utils_safe_free(xstar);
	
		// free and close cplex model   
		//CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env); 

	return e; 
}

//================================================================================
// CPLEX UTILS
//================================================================================	

ERROR_CODE cx_initialize(instance* inst, CPXENVptr env, CPXLPptr lp){

	log_info("initializing CPLEX parameters");

	ERROR_CODE error = T_OK;

	cx_build_model(inst, env, lp);
	
	// Cplex's parameter setting
	
	// save CPLEX output to a log file
	if(err_dolog()){
		CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF); // output to screen

		mkdir("logs", 0777);
    	char log_path[1024];

		char buffer[40];
		utils_plotname(buffer, 40);

    	sprintf(log_path, "logs/%s.log", buffer);

		log_info("CPLEX logs will be saved in file %s", log_path);
    	CPXsetlogfilename(env, log_path, "w");
	}

	// disable clone log in parallel optimization
	CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);

	if(inst->options_t.timelimit > 0.0){
		CPXsetdblparam(env, CPX_PARAM_TILIM, inst->options_t.timelimit); 
	}

	if(inst->options_t.init_mip){

		log_info("computing heuristic for MIP Start");

		// Run one of our heurstics to be added to the MIP starts
		// Can be faster than CPLEX heuristics since ours are specific for TSP

		double old_timelimit = inst->options_t.timelimit;
		// set the new timelimit to 1/10 of the total time, to make it so the heuristic doesnt consume all of the available time
		inst->options_t.timelimit = old_timelimit / 10.0;

		// run all nearest neighbor heuristic
		error = h_Greedy_iterative(inst);

		// change the timelimit back to the original one
		inst->options_t.timelimit = old_timelimit;

		int* varindices = (int*) calloc(inst->nnodes, sizeof(int));
		double* values = (double*) calloc(inst->nnodes, sizeof(double));

		// initialize values for CPLEX
		// An entry values[j] greater than or equal to CPX_INFBOUND specifies that no value is set for the variable varindices[j]
		for(int i=0; i<inst->nnodes; i++){
			values[i] = CPX_INFBOUND;
		}

		int k = 0;
		for(int i=0; i<inst->nnodes; i++){
			int j = inst->best_solution.path[i];

			varindices[k] = cx_xpos(i,j,inst);
			values[k] = 1.0;

			k++;
		}

		const int beg = 0;
		const int effortlevel = CPX_MIPSTART_NOCHECK;
		if( CPXaddmipstarts(env, lp, 1, inst->nnodes, &beg, varindices, values, &effortlevel, NULL) ){
			log_error("CPXaddmipstarts error");
			error = INTERNAL;
		}

		log_info("heuristic solution added to MIP start");

		utils_safe_free(varindices);
		utils_safe_free(values);
	}

	return error;
}

int cx_xpos(int i, int j, instance *inst){ 

	if ( i == j ){
		log_fatal(" i == j in cx_xpos" );
		tsp_handlefatal(inst);
	} 

	if ( i > j ){
		return cx_xpos(j,i,inst);
	} 

	int pos = i * inst->nnodes + j - (( i + 1 ) * ( i + 2 )) / 2;

	return pos;
}

ERROR_CODE cx_add_sec(CPXENVptr env, CPXLPptr lp, int* comp, int ncomp, instance* inst){
	if(ncomp == 1){
		return INVALID_ARGUMENT;
	}

	const char sense='L';

	// initialize index and value
	int ncols = CPXgetnumcols(env, lp);
	int* index = (int*) calloc(ncols, sizeof(int));
	double* value = (double*) calloc(ncols, sizeof(double));

	// non più di n**2
	for(int k=1; k<=ncomp; k++){
		int nnz=0;

		int number_nodes = 0; // |S|

		for(int i=0; i<inst->nnodes; i++){

			// skip iteration if it does belong to the current component k
			if(comp[i] != k){
				continue;
			}

			number_nodes++;

			for(int j=i+1; j<inst->nnodes; j++){
				// skip iteration if it does belong to the current component k
				if(comp[j] != k){
					continue;
				}

				index[nnz] = cx_xpos(i,j,inst);
				value[nnz] = 1.0;

				nnz++;
			}
		}

		double rhs = number_nodes - 1.0; // |S|-1
		// https://www.ibm.com/docs/en/icos/22.1.0?topic=cpxxaddrows-cpxaddrows
		int izero = 0;
		CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, NULL);
	}

	// utils_safe_free resources
	utils_safe_free(index);
	utils_safe_free(value);

	return T_OK;
}

// construct sec
ERROR_CODE cx_compute_cuts(int* comp, int ncomp, instance* inst, int* nnz, double* rhs, char* sense, int* matbeg, int* matind, double* matval){

	log_info("starting sec computing");

	if (ncomp <= 1 || comp == NULL || inst == NULL) {
		log_debug("invalid argument");
        return INVALID_ARGUMENT;
    }

	*nnz = 0;

	// initialize index and value
	log_debug("finish init");

	// non più di n**2
	for(int k=1; k<=ncomp; k++){

		matbeg[k-1] = *nnz;

		int cnt = 0;
		int number_nodes = 0; // |S|

		for(int i=0; i<inst->nnodes; i++){

			// skip iteration if it does belong to the current component k
			if(comp[i] != k){
				continue;
			}

			number_nodes++;

			for(int j=i+1; j<inst->nnodes; j++){
				// skip iteration if it does belong to the current component k
				if(comp[j] != k){
					continue;
				}

				matind[*nnz + cnt] = cx_xpos(i,j,inst);
				matval[*nnz + cnt] = 1.0;

				cnt++;
			}

		}
		
		*nnz += cnt;
    	rhs[k-1] = number_nodes - 1.0;
		sense[k-1] = 'L';

	}

	log_debug("finish computing cuts");
	log_debug("done. returning.\n");

	return T_OK;
}

void cx_build_model(instance *inst, CPXENVptr env, CPXLPptr lp){    
	char binary = 'B'; 

	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	// add binary var.s x(i,j) for i < j  
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		for ( int j = i+1; j < inst->nnodes; j++ )
		{
			sprintf(cname[0], "x(%d,%d)", i+1,j+1);  		// ... x(1,2), x(1,3) ....
			double obj = tsp_get_cost(inst, i, j);
			double lb = 0.0;
			double ub = 1.0;
			if ( CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname) ){
				log_fatal(" wrong CPXnewcols on x var.s");
				tsp_handlefatal(inst);
			}
    		if ( CPXgetnumcols(env,lp)-1 != cx_xpos(i,j, inst) ){
				log_fatal(" wrong position for x var.s");
				tsp_handlefatal(inst);
			}
		}
	} 

// add the degree constraints 

	int *index = (int *) calloc(inst->nnodes, sizeof(int));
	double *value = (double *) calloc(inst->nnodes, sizeof(double));

	const double rhs = 2.0;
	const char sense = 'E';                            // 'E' for equality constraint 
	for ( int h = 0; h < inst->nnodes; h++ )  		// add the degree constraint on node h
	{
		
		sprintf(cname[0], "degree(%d)", h+1);   
		int nnz = 0;
		for ( int i = 0; i < inst->nnodes; i++ )
		{
			if ( i == h ) continue;
			index[nnz] = cx_xpos(i,h, inst);
			value[nnz] = 1.0;
			nnz++;
		}
		int izero = 0;
		if ( CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]) ){
			log_fatal("CPXaddrows(): error 1");
			tsp_handlefatal(inst);
		} 
	}

	inst->ncols = CPXgetnumcols(env, lp);

	utils_safe_free(value);
	utils_safe_free(index);

	utils_safe_free(cname[0]);
	utils_safe_free(cname);

	if(err_dolog()){
		CPXwriteprob(env, lp, "results/model.lp", NULL);   
	}

}

void cx_build_sol(const double *xstar, instance *inst, int *comp, int *ncomp, tsp_solution* solution) 
{   
	// initialize number of components and array of components
	*ncomp = 0;
	for ( int i = 0; i < inst->nnodes; i++ )
	{
		comp[i] = -1;
	}
	
	// initialize solution cost
	solution->cost = 0.0;

	for ( int start = 0; start < inst->nnodes; start++ )
	{
		if ( comp[start] >= 0 ) continue;  // node "start" was already visited, just skip it

		// a new component is found
		(*ncomp)++;
		int i = start;
		int done = 0;
		while ( !done )  // go and visit the current component
		{
			comp[i] = *ncomp;
			done = 1;
			for ( int j = 0; j < inst->nnodes; j++ )
			{
				if ( i != j && xstar[cx_xpos(i,j,inst)] > 0.5 && comp[j] == -1 ) // the edge [i,j] is selected in xstar and j was not visited before 
				{
					solution->path[i] = j;
					solution->cost += tsp_get_cost(inst, i, j);
					i = j;
					done = 0;
					break;
				}
			}
		}	
		solution->path[i] = start;  // last arc to close the cycle
		solution->cost += tsp_get_cost(inst, i, start);
		
		// go to the next component...
	}
}

// TODO: handle ctrl+c by user gracefully
ERROR_CODE cx_handle_cplex_status(CPXENVptr env, CPXLPptr lp){
	int status = CPXgetstat(env, lp);

	switch (status)
	{
	case CPXMIP_TIME_LIM_FEAS:
		return DEADLINE_EXCEEDED;
	case CPXMIP_TIME_LIM_INFEAS:
		return RESOURCE_EXHAUSTED;
	case CPXMIP_INFEASIBLE:
		return NOT_FOUND;
	case CPXMIP_ABORT_FEAS:
		return CANCELLED;
	case CPXMIP_ABORT_INFEAS:
		return NOT_FOUND;	
	case CPXMIP_OPTIMAL_TOL:
		return T_OK;
	case CPXMIP_OPTIMAL:
		return T_OK;
	default:
		log_error("unhandled cplex status code %d\n", status);
		return UNIMPLEMENTED;
	}
}

void cx_patching(instance *inst, int *comp, int *ncomp, tsp_solution* solution){
	double best_delta = __DBL_MAX__;
	int best_nodes[2] = {-1, -1};

	// compute nodes to patch together
	for (int a = 0; a < inst->nnodes; a++){
		for(int b = 0; b < inst->nnodes; b++){
			if(comp[a] == comp[b]){
				continue;
			}
			int succ_a = solution->path[a]; //successor of a
            int succ_b = solution->path[b]; //successor of b

            double current_cost = tsp_get_cost(inst, a, succ_a) + tsp_get_cost(inst, b, succ_b);
            double swapped_cost = tsp_get_cost(inst, a, succ_b) + tsp_get_cost(inst, b, succ_a);
            double delta = swapped_cost - current_cost;

			if(delta < best_delta && delta > 0){
				best_delta = delta;
				best_nodes[0] = a;
				best_nodes[1] = b;
			}
		}
	}
	// patch components
	log_debug("best_delta: %f", best_delta);
	if(best_delta > EPSILON){
		log_debug("  patching...");
        int a = best_nodes[0];
        int b = best_nodes[1];

        int succ_a = solution->path[a]; //successor of a
        int succ_b = solution->path[b]; //successor of b
        
		solution->path[a] = succ_b;
		solution->path[b] = succ_a;

        // update solution cost
        solution->cost += best_delta;

		// update comp
		int i = succ_b;
		int comp_index = comp[a];
		bool finish = false; 
		while(!finish){
			comp[i] = comp_index;
			if(i == b){
				finish = true;
			}			
			i = solution->path[i];
		}

		(*ncomp) = (*ncomp)-1;
    }
	
}

static int CPXPUBLIC callback_branch_and_cut(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle){
	log_debug("callback called");
	instance* inst = (instance*) userhandle;

	switch (contextid)
	{
	case CPX_CALLBACKCONTEXT_CANDIDATE:
		return callback_candidate(context, inst);
	case CPX_CALLBACKCONTEXT_RELAXATION:
		return callback_relaxation(context, inst);
	default:
		log_error("Callback error");
		return 1;
	}
}

static int CPXPUBLIC callback_candidate(CPXCALLBACKCONTEXTptr context, instance* inst){
	log_debug("CANDIDATE CALLBACK");

	// return value, if everything is ok, should be 0, else 1
	int ret_value = 0;

	int ispoint = 0;

	// check wheter the candidate callback occured if CPLEX found a candidate integer feasible point or because it has detected an unbounded relaxation
	CPXcallbackcandidateispoint(context, &ispoint);

	if(!ispoint){
		log_error("unbounded relaxation detected, skipping candidate callback execution");
		ret_value = 1;
		goto cx_free;
	}

	double* xstar = (double *) calloc(inst->ncols, sizeof(double));
	double objval = CPX_INFBOUND; 

	// get the candidate 
	if ( CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval) ){
		log_error("CPXcallbackgetcandidatepoint error");
		ret_value = 1;
		goto cx_free;
	} 

	int ncomp = 0;
	int* comp = (int*) calloc(inst->ncols, sizeof(int));

	// build the solution from xstar
	tsp_solution solution = tsp_init_solution(inst->nnodes);
	cx_build_sol(xstar, inst, comp, &ncomp, &solution);
	
	// reject the candidate if the solution is not a single tour
	if(ncomp > 1){
		int nnz = 0;
		double* rhs = (double*) calloc(ncomp, sizeof(double));
		char* sense = (char*) calloc(ncomp, sizeof(char));
		int* matbeg = (int*) calloc(ncomp, sizeof(int));
		double* matval = (double*) calloc(ncomp * inst->ncols, sizeof(double));
		int* matind = (int*) calloc(ncomp * inst->ncols, sizeof(int));
		cx_compute_cuts(comp, ncomp, inst, &nnz, rhs, sense, matbeg, matind, matval);

		// reject candidate and add new cut
		int error = CPXcallbackrejectcandidate(context, ncomp, nnz, rhs, sense, matbeg, matind, matval );
		if ( error ) {
			log_error("CPXcallbackrejectcandidate() error, code %d", error);
			ret_value = 1;
		}
		
		log_info("candidate solution rejected and cut added");

		// free resources
		utils_safe_free(rhs);
		utils_safe_free(sense);
		utils_safe_free(matbeg);
		utils_safe_free(matval);
		utils_safe_free(matind);

		// means that reject candidate has encountered an error
		// dont continue and go to free
		if(ret_value == 1){
			goto cx_free;
		}
	}

	cx_free:
		utils_safe_free(xstar);
		utils_safe_free(comp);

	return ret_value;
}

static int CPXPUBLIC callback_relaxation(CPXCALLBACKCONTEXTptr context, instance* inst){

	log_debug("RELAXATION CALLBACK");

	// return value, if everything is ok, should be 0, else 1
	int ret_value = 0;

	// execute this code only an handful of times, since this callback will be called milion of times

	// three methods:
	//  1) generate random number (needs to be thread-safe)
	//  2) use node count
	//  3) use depth, if depth<3 run the code
	int threadid = -1;
	int nodes = -1;
	int depth = -1;

	switch (inst->options_t.skip_policy){
	case BC_PROB:
		// method 1 
		if(CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &threadid)){
			log_error("CPXcallbackgetinfoint on thread id");
		};
		unsigned int seed = inst->threads_seeds[threadid];
		inst->threads_seeds[threadid] = seed+1;

		double prob = ( (double) rand_r(&seed) ) / RAND_MAX;
		if(prob > 0.1){
			log_debug("skipped");
			return 0;
		}

		break;
	case BC_NODES:
		// method 2
		if(CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &nodes)){
			log_error("CPXcallbackgetinfoint error on node count");
		}

		if(nodes % 10 != 0){
			log_debug("skipped");
			return 0;
		}

		break;
	case BC_DEPTH:
		// method 3
		if(CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODEDEPTH, &depth)){
			log_error("CPXcallbackgetinfoint error on depth");
		}

		if(depth >= 3){
			log_debug("skipped");
			return 0;
		}
		break;
	default:
		log_error("not found");
		break;
	}

	// callback code
	double* xstar = (double *) calloc(inst->ncols, sizeof(double));
	double objval = CPX_INFBOUND; 

	if ( CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols-1, &objval) ){
		log_error("CPXcallbackgetrelaxationpoint error");
	} 

	int ncomp = 0;
	int* comps = NULL;
	int* compscount = NULL;

	// transform into elist format for Concorde
	// elist[2*i] contains one node of the i-th edge, and elist[2*i+1] contains the other node
	int* elist = (int*) calloc(2 * inst->ncols, sizeof(int));
	double* new_xstar = (double*) calloc(inst->ncols, sizeof(double));
	int num_edges = 0;
	int k=0;

	int kpos = 0;

	for(int i=0; i<inst->nnodes; i++){
		for(int j=i+1; j<inst->nnodes; j++){

			// take only points that contribute to the solution
			if(xstar[cx_xpos(i,j, inst)] > 0.001){
				elist[k++] = i;
				elist[k++] = j;

				new_xstar[num_edges] = xstar[cx_xpos(i,j,inst)];

				// verified correspondence between edges and cplex columns
				if(cx_xpos(i,j,inst) != kpos){
					log_error("cx_xpos error");
				}

				num_edges++;
			}

			kpos++;
		}
	}

	// Detect the connected components of the graph. Receives:
	// ncomp - number of connected components
	// compscount - array to receive the number of connected components
	// comps - array to receive the edges pertaining to each component
	if(CCcut_connect_components(inst->nnodes, num_edges, elist, new_xstar, &ncomp, &compscount, &comps)){
		log_error("CCcut_connect_components");
		ret_value = 1;
		goto cx_free;
	}

	log_debug("number of components: %d", ncomp);

	if(ncomp == 1){
		// connected graph, but it may not be a tsp solution

		violatedcuts_passparams userhandle = {.context = context, .inst = inst};

		// find the cut that violate the 2.0-EPSILON_BC threshold
		if(CCcut_violated_cuts(inst->nnodes, num_edges, elist, new_xstar, 2.0-EPSILON_BC, cc_add_violated_sec, &userhandle)){
			log_error("CCcut_violated_cuts");
			ret_value = 1;
			goto cx_free;
		}
	}else{

		log_info("Found %d components, adding cuts", ncomp);

		// transform components array from concorde format to ours
		// ours: index is the node, value is the components (starting from 1)
		
		int* components = (int*) calloc(inst->nnodes, sizeof(int));

		int start = 0;
		for(int sub=0; sub<ncomp; sub++){
			for(int i = start; i < start + compscount[sub]; i++){
				components[comps[i]] = sub+1;
			}
			start += compscount[sub];
		}

		// add all the subtour elimination constraints
		
		int nnz = 0;
		double* rhs = (double*) calloc(ncomp, sizeof(double));
		char* sense = (char*) calloc(ncomp, sizeof(char));
		int* matbeg = (int*) calloc(ncomp, sizeof(int));
		double* matval = (double*) calloc(ncomp * inst->ncols, sizeof(double));
		int* matind = (int*) calloc(ncomp * inst->ncols, sizeof(int));
		cx_compute_cuts(components, ncomp, inst, &nnz, rhs, sense, matbeg, matind, matval);

		int* purgeable = (int*) calloc(ncomp, sizeof(int));
		int* local = (int*) calloc(ncomp, sizeof(int));

		for(int i=0; i<ncomp; i++){
			purgeable[i] = CPX_USECUT_FORCE;			// The cut is added to the relaxation and stays there
			local[i] = 0;								// Means it is globally valid
		}

		// add all the cuts
		int error = CPXcallbackaddusercuts(context, ncomp, nnz, rhs, sense, matbeg, matind, matval, purgeable, local);
		if(error){
			log_error("CPXcallbackaddusercuts error, cod %d", error);
			ret_value = 1;
		}

		utils_safe_free(rhs);
		utils_safe_free(sense);
		utils_safe_free(matbeg);
		utils_safe_free(matval);
		utils_safe_free(matind);
		utils_safe_free(purgeable);
		utils_safe_free(local);

		utils_safe_free(components);

		// add user cuts gave an error, so we break from the loop and free all other resources
		if(ret_value == 1){
			goto cx_free;
		}
	}


	// call the modified greedy and post its solution

	unsigned int seed = inst->threads_seeds[threadid];
	inst->threads_seeds[threadid] = seed+1;

	double prob = ( (double) rand_r(&seed) ) / RAND_MAX;
	if(prob <= 0.1){
		// greedy
		log_info("computing a heuristic NN solution with xstar-weighted costs to post");
		// modify costs
		double* modified_costs = (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));
		for (int i = 0; i < inst->nnodes; i++) {
			// Initialize each element of the matrix to -1 -> infinite cost
			for (int j = i+1; j < inst->nnodes; j++) {
				double cost= inst->costs[i* inst->nnodes + j] * (1 - xstar[cx_xpos(i,j,inst)]);
				modified_costs[i* inst->nnodes + j] = cost;
				modified_costs[j* inst->nnodes + i] = cost;
			}
		}

		// run all nearest neighbor heuristic with xstar-weighted costs to post solution
		tsp_solution solution = tsp_init_solution(inst->nnodes);
		ERROR_CODE error = h_Greedy_iterative_mod_costs(inst, &solution, modified_costs);
		if(!err_ok(error)){
			log_error("error in greedy for posting solution");
			utils_safe_free(modified_costs);
			utils_safe_free(solution.path);
			ret_value = 1;
			goto cx_free;
		}

		// 2opt to the best solution to improve the solution
		error = ref_2opt(inst, &solution);
		if(!err_ok(error)){
			log_error("error in 2opt for posting solution");
			utils_safe_free(modified_costs);
			utils_safe_free(solution.path);
			ret_value = 1;
			goto cx_free;
		}

		// if it is better than incumbement, build a cplex solution and post it
		if(solution.cost < inst->best_solution.cost){
			// build cplex solution
			double *xheu = (double *) calloc(inst->ncols, sizeof(double));
			for ( int i = 0; i < inst->nnodes; i++ ) xheu[cx_xpos(i,solution.path[i],inst)] = 1.0;
			int *ind = (int *) malloc(inst->ncols * sizeof(int));
			for ( int j = 0; j < inst->ncols; j++ ) ind[j] = j;
		
		
			if( CPXcallbackpostheursoln(context, inst->ncols, ind, xheu, solution.cost, CPXCALLBACKSOLUTION_NOCHECK) ){
				log_error("CPXcallbackpostheursoln error");
				error = INTERNAL;
			}else{
				log_debug("posted heuristic solution with modified cost: %f", solution.cost);
			}

			utils_safe_free(xheu);
			utils_safe_free(ind);
		}
		

		utils_safe_free(modified_costs);
		utils_safe_free(solution.path);	
	}
	
	cx_free:
		utils_safe_free(xstar);
		utils_safe_free(new_xstar);
		utils_safe_free(comps);
		utils_safe_free(compscount);
		utils_safe_free(elist);

	return ret_value;
}

int cc_add_violated_sec(double cut_value, int cut_nnodes, int* cut_indexes, void* userhandle){
	
	log_info("Violated cut found, adding new cut");

	violatedcuts_passparams* uh = (violatedcuts_passparams*) userhandle;
	instance* inst = uh->inst;
	CPXCALLBACKCONTEXTptr context = uh->context;
 
	int num_edges = ( cut_nnodes * (cut_nnodes - 1) ) / 2;

	int* index = (int*) calloc(num_edges, sizeof(int));
	double* value = (double*) calloc(num_edges, sizeof(double));

	int nnz=0;
	for(int i=0; i<cut_nnodes; i++){
		for(int j=i+1; j<cut_nnodes; j++){
			// concorde assumes it is undirected
			index[nnz] = cx_xpos(cut_indexes[i], cut_indexes[j], inst);
			value[nnz] = 1.0;
			nnz++;
		}
	}

	const char sense = 'L';
	const double rhs = cut_nnodes - 1;
	const int rmatbeg = 0;
	const int purgeable = CPX_USECUT_PURGE; 		// The cut is added to the relaxation but can be purged later on if CPLEX deems the cut ineffective.
	const int local = 0; 							// Array of flags that specifies for each cut whether it is only locally valid (value = 1) or globally valid (value = 0).

	if(nnz <= 0){
		log_error("nnz should not be 0");
		return 1;
	}

	// add the generated cut to CPLEX
	if(CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &rmatbeg, index, value, &purgeable, &local)){
		log_error("CPXcallbackaddusercuts");
		return 1;
	}

	log_debug("add user cut, edges %d", nnz);
	log_debug("cut value: %.4f", cut_value);

	utils_safe_free(index);
	utils_safe_free(value);

	return 0;
}
