#include "cplex_model.h"

ERROR_CODE cx_Nosec(instance *inst){  
	// open CPLEX model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	if ( error ){
		log_fatal("CPX code %d : CPXopenCPLEX() error", error);
		tsp_handlefatal(inst);
	} 
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1"); 
	if ( error ) {
		log_fatal("CPX code %d : CPXcreateprob() error", error);
		tsp_handlefatal(inst);	
	}

	// initialize CPLEX model
	cx_initialize(inst, env, lp);

	// solve with cplex
	error = CPXmipopt(env,lp);
	if ( error ) 
	{
		log_fatal("CPX code %d : CPXmipopt() error", error); 
		tsp_handlefatal(inst);
	}

	int ncols = CPXgetnumcols(env, lp);

	// get the optimal value
	double* xstar = (double *) calloc(ncols, sizeof(double));

	// check that cplex solved it right
	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ){
		log_fatal("CPX : CPXgetx() error");	
		tsp_handlefatal(inst);
	} 

	int ncomp = 0;
	int* comp = (int*) calloc(ncols, sizeof(int));

	// with the optimal found by CPLEX, build the corresponding solution
	tsp_solution solution = tsp_init_solution(inst->nnodes);
	cx_build_sol(xstar, inst, comp, &ncomp, &solution);

	// update best solution (not with tsp_update_solution since it is not a cycle)
	memcpy(inst->best_solution.path, solution.path, inst->nnodes * sizeof(int));
    inst->best_solution.cost = solution.cost;
    log_debug("new best solution: %f", solution.cost);

	log_info("number of independent components: %d", ncomp);

	free(xstar);
	
	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 

	return OK; 
}

// TODO: better error handling
ERROR_CODE cx_BendersLoop(instance* inst, bool patching){
	// open CPLEX model
	int e;
	CPXENVptr env = CPXopenCPLEX(&e);
	if(e){
		log_fatal("CPX code %d : CPXopenCPLEX() error", e);
		tsp_handlefatal(inst);
	} 
	CPXLPptr lp = CPXcreateprob(env, &e, "TSP model version 1"); 
	if(e){
		log_fatal("CPX code %d : CPXcreateprob() error", e);
		tsp_handlefatal(inst);	
	}

	// initialize CPLEX model
	ERROR_CODE error = OK;

	error = cx_initialize(inst, env, lp);
	if(!err_ok(error)){
		log_fatal("code %d : error in initialize", error);
		tsp_handlefatal(inst);
	}

	tsp_solution solution = tsp_init_solution(inst->nnodes);
	int iteration = 0;
	int n_sec = 0;
	while(1){
		log_info("iteration %d", iteration);
		double ex_time = utils_timeelapsed(inst->c);
        if(inst->options_t.timelimit != -1.0){
			CPXsetdblparam(env, CPX_PARAM_TILIM, inst->options_t.timelimit - ex_time); 

            if(ex_time > inst->options_t.timelimit){
				log_warn("exceeded time, saving best solution found until now");
				error = DEADLINE_EXCEEDED;
				break;
            }
        }

		// solve with cplex
		e = CPXmipopt(env,lp);
		if ( e ){
			log_fatal("CPX code %d : CPXmipopt() error", e); 
			free(solution.path);
			tsp_handlefatal(inst);
		}

		int ncols = CPXgetnumcols(env, lp);

		// get the optimal value
		double* xstar = (double *) calloc(ncols, sizeof(double));

		// check that cplex solved it right
		if ( CPXgetx(env, lp, xstar, 0, ncols-1) ){
			log_fatal("CPX : CPXgetx() error");	
			free(solution.path);
			tsp_handlefatal(inst);
		} 

		int ncomp = 0;
		int* comp = (int*) calloc(ncols, sizeof(int));
		cx_build_sol(xstar, inst, comp, &ncomp, &solution);

		log_info("number of components: %d", ncomp);
		log_info("current solution cost: %f", solution.cost);
		log_info("is solution a tour? %d", isTour(solution.path, inst->nnodes));
		log_info("cost re-computed: %f", solutionCost(inst, solution.path));

		n_sec += ncomp;

		// only one component it means that we have found an Hamiltonian cycle
		if(ncomp == 1){
			break;
		}

		error = cx_add_sec(env, lp, comp, ncomp, inst);
		if(!err_ok(error)){
			log_fatal("code %d : error in add_sec", error);
			free(solution.path);
			tsp_handlefatal(inst);
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
		free(comp);
	}

	log_info("is solution a tour? %d", isTour(solution.path, inst->nnodes));
	// save the best solution
	tsp_update_best_solution(inst, &solution);

	free(solution.path);
	
	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 

	return error;
}


ERROR_CODE cx_BranchAndCut(instance *inst){  
	// open CPLEX model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	if ( error ){
		log_fatal("CPX code %d : CPXopenCPLEX() error", error);
		tsp_handlefatal(inst);
	} 
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1"); 
	if ( error ) {
		log_fatal("CPX code %d : CPXcreateprob() error", error);
		tsp_handlefatal(inst);	
	}

	// initialize CPLEX model
	cx_initialize(inst, env, lp);

	int ncols = CPXgetnumcols(env, lp);
	double* xstar = (double *) calloc(ncols, sizeof(double));

	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
	if ( CPXcallbacksetfunc(env, lp, contextid, callback_branch_and_cut, inst) ) {
		log_fatal("CPXcallbacksetfunc() error");
		tsp_handlefatal(inst);
	}

	// solve with cplex
	error = CPXmipopt(env,lp);
	if ( error ) 
	{
		log_fatal("CPX code %d : CPXmipopt() error", error); 
		tsp_handlefatal(inst);
	}

	// check that cplex solved it right
	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ){
		log_fatal("CPX : CPXgetx() error");	
		tsp_handlefatal(inst);
	} 

	int ncomp = 0;
	int* comp = (int*) calloc(ncols, sizeof(int));

	// with the optimal found by CPLEX, build the corresponding solution
	tsp_solution solution = tsp_init_solution(inst->nnodes);
	cx_build_sol(xstar, inst, comp, &ncomp, &solution);

	// update best solution
	tsp_update_best_solution(inst, &solution);

	log_info("number of independent components: %d", ncomp);
	log_info("is solution a tour? %d", isTour(solution.path, inst->nnodes));

	free(xstar);
	
	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 

	return OK; 
}

//================================================================================
// CPLEX UTILS
//================================================================================	

ERROR_CODE cx_initialize(instance* inst, CPXENVptr env, CPXLPptr lp){

	cx_build_model(inst, env, lp);
	
	// Cplex's parameter setting
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	
	// save CPLEX output to a log file
	if(err_dolog()){
		//CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen 
		log_info("save to log file");
		mkdir("logs", 0777);
    	char log_path[1024];
    	sprintf(log_path, "logs/%s.log", inst->options_t.inputfile);
    	CPXsetlogfilename(env, log_path, "w");
	}

	if(inst->options_t.timelimit > 0.0){
		CPXsetdblparam(env, CPX_PARAM_TILIM, inst->options_t.timelimit); 
	}

	return OK;
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

	// free resources
	free(index);
	free(value);

	return OK;
}

ERROR_CODE cx_get_sec(int* comp, int ncomp, instance* inst, cut *cuts){
	log_info("starting sec computing\n");

	if (ncomp <= 1 || comp == NULL || inst == NULL || cuts == NULL) {
        return INVALID_ARGUMENT;
    }

	// initialize index and value
	int ncols = inst->ncols;
	int* index = (int*) calloc(ncols, sizeof(int));
	double* value = (double*) calloc(ncols, sizeof(double));
	log_debug("finish init\n");

	// non più di n**2
	for(int k=1; k<=ncomp; k++){
		cut new_cut;
		cx_init_cut(&new_cut, ncols);

		new_cut.nnz=0;

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

				new_cut.index[new_cut.nnz] = cx_xpos(i,j,inst);
				new_cut.value[new_cut.nnz] = 1.0;

				new_cut.nnz++;
			}
		}
		//printf("cut computed\n");

    	new_cut.rhs = number_nodes - 1.0;
		cuts[k-1] = new_cut;

		// Free resources for current cut
        free(new_cut.index);
        free(new_cut.value);
	}

	log_debug("finish computing cuts");
	log_debug("freeing...");
	// free resources
	free(index);
	free(value);
	log_debug("done. returning.");

	return OK;
}

void cx_init_cut(cut* new_cut, int ncols){
	//printf("init cut...");
	new_cut->index = (int*) calloc(ncols, sizeof(int));
	new_cut->value = (double*) calloc(ncols, sizeof(double));
	//printf("done\n");
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

	free(value);
	free(index);

	free(cname[0]);
	free(cname);

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
		return callback_candidate(context, contextid, inst);
	case CPX_CALLBACKCONTEXT_RELAXATION:
		return callback_relaxation(context, contextid, inst);
	default:
		log_error("Callback error");
		return 1;
	}
}

static int CPXPUBLIC callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, instance* inst){
	int ncols = inst->ncols;
	double* xstar = (double *) calloc(ncols, sizeof(double));
	double objval = CPX_INFBOUND; 
	if ( CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval) ) log_error("CPXcallbackgetcandidatepoint error");

	int ncomp = 0;
	int* comp = (int*) calloc(ncols, sizeof(int));

	tsp_solution solution = tsp_init_solution(inst->nnodes);
	cx_build_sol(xstar, inst, comp, &ncomp, &solution);
	
	if(ncomp > 1){

		const char sense='L';
		int izero = 0;
		int* index = (int*) calloc(ncols, sizeof(int));
		double* value = (double*) calloc(ncols, sizeof(double));

		// VERSIONE PIU PULITA MA BUGGATA
		/*
		cut* cuts = (cut*) calloc(ncomp, sizeof(cut));
		cx_get_sec(comp, ncomp, inst, cuts);

		for(int k=0; k<ncomp; k++){
			printf("cuts[%d]\n", k);
			printf("nnz: %d\n", cuts[k].nnz);
			printf("rhs: %f\n", cuts[k].rhs);
		}

		for(int k=0; k<ncomp; k++){
			if(cuts[k].nnz > 0){
				if ( CPXcallbackrejectcandidate(context, 1, cuts[k].nnz, &(cuts[k].rhs), &sense, &izero, cuts[k].index, cuts[k].value )) {
					log_fatal("CPXcallbackrejectcandidate() error");
					tsp_handlefatal(inst);
				}
			}
		}

		free(cuts);
		*/

		// VERSIONE BASIC FUNZIONANTE
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

			//printf("nnz: %d, rhs: %f", nnz, rhs);


			if ( CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value )) {
				log_fatal("CPXcallbackrejectcandidate() error");
				tsp_handlefatal(inst);
			}
		}
		
		// free resources
		free(index);
		free(value);
		
	}

	free(xstar);
	free(comp);

	return 0;
}

static int CPXPUBLIC callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, instance* inst){
	int ncols = inst->ncols;
	double* xstar = (double *) calloc(ncols, sizeof(double));
	double objval = CPX_INFBOUND; 

	if ( CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols-1, &objval) ){
		log_error("CPXcallbackgetrelaxationpoint error");
	} 

	int ncomp = 0;
	int* comps = NULL;
	int* compscount = NULL;

	// transform into elist format for Concorde
	// elist[2*i] contains one node of the i-th edge, and elist[2*i+1] contains the other node
	int* elist = (int*) calloc(2 * ncols, sizeof(int));
	int num_edges = 0;
	int k=0;

	for(int i=0; i<ncols; i++){
		for(int j=i+1; j<ncols; j++){
			elist[k++] = i;
			elist[k++] = j;

			num_edges++;
		}
	}

	// Detect the connected components of the graph. Receives:
	// ncomp - number of connected components
	// compscount - array to receive the number of connected components
	// comps - array to receive the edges pertaining to each component
	if(CCcut_connect_components(inst->nnodes, num_edges, elist, xstar, &ncomp, &compscount, &comps)){
		log_error("CCcut_connect_components");
	}

	if(ncomp == 1){
		// connected graph, but it may not be a tsp solution

		violatedcuts_passparams userhandle = {.context = context, .inst = inst};

		// find the cut that violate the 2.0-EPSILON threshold
		if(CCcut_violated_cuts(inst->nnodes, num_edges, elist, xstar, 2.0-EPSILON, cx_add_violated_sec, &userhandle)){
			log_error("CCcut_violated_cuts");
		}
	}else{

		// transform components array from concorde format to ours
		// ours: index is the node, value is the components (starting from 1)
		int* components = (int*) calloc(inst->nnodes, sizeof(int));

		int start = 0;
		for(int sub=0; sub<ncomp; sub++){
			for(int i = start; i < start + compscount[sub]; i++){
				components[comps[i]] = sub+1;
			}
		}

		// add all the subtour elimination constraints
		cut* cuts = (cut*) calloc(ncomp, sizeof(cut));
		cx_get_sec(components, ncomp, inst, cuts);

		const char* sense = 'L';
		const int rmatbeg = 0;
		const int purgeable = CPX_USECUT_FILTER;
		const int local = 0;

		for(int i=0; i<ncomp; i++){
			if(CPXcallbackaddusercuts(context, 1, cuts[i].nnz, &(cuts[i].rhs), sense, &rmatbeg, &(cuts[i].index), &(cuts[i].value), &purgeable, &local)){
				log_error("CPXcallbackaddusercuts");
			}
		}

		free(cuts);
		free(components);
	}

	free(xstar);
	free(comps);
	free(compscount);
	free(elist);
}

int cx_add_violated_sec(double cut_value, int cut_nnodes, int* cut_indexes, void* userhandle){
	violatedcuts_passparams* uh = (violatedcuts_passparams*) userhandle;
	instance* inst = uh->inst;
	CPXCALLBACKCONTEXTptr context = uh->context;

	int num_edges = ( cut_nnodes * (cut_nnodes - 1) ) / 2;

	int* index = (int*) calloc(num_edges, sizeof(int));
	double* value = (double*) calloc(num_edges, sizeof(double));

	int nnz=0;
	for(int i=0; i<cut_nnodes; i++){
		for(int j=0; j<cut_nnodes; j++){
			// concorde assumes it is undirected
			if(cut_indexes[i] < cut_indexes[j]){
				index[nnz] = cx_xpos(cut_indexes[i], cut_indexes[j], inst);
				value[nnz] = 1.0;
				nnz++;
			}
		}
	}

	const char* sense = 'L';
	const int rhs = cut_nnodes - 1;
	const int rmatbeg = 0;
	const int purgeable = CPX_USECUT_PURGE; // The cut is added to the relaxation but can be purged later on if CPLEX deems the cut ineffective.
	const int local = 0; // Array of flags that specifies for each cut whether it is only locally valid (value = 1) or globally valid (value = 0).

	if(CPXcallbackaddusercuts(context, 1, num_edges, rhs, sense, &rmatbeg, &index, &value, &purgeable, &local)){
		log_error("CPXcallbackaddusercuts");
	}

	free(index);
	free(value);

	return 0;
}
