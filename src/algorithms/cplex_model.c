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
	error = cx_check_optimal(env, lp, xstar, ncols, inst);
	if(error){
		log_fatal("code %d : error in get_optimal", error);
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
ERROR_CODE cx_BendersLoop(instance* inst){
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
	ERROR_CODE error = cx_initialize(inst, env, lp);
	if(error){
		log_fatal("code %d : error in get_optimal", error);
		tsp_handlefatal(inst);
	}

	tsp_solution solution = tsp_init_solution(inst->nnodes);
	int iteration = 0;
	while(1){
		double ex_time = utils_timeelapsed(inst->c);
        if(inst->options_t.timelimit != -1.0){
            if(ex_time > inst->options_t.timelimit){
               return DEADLINE_EXCEEDED;
            }
        }

		// solve with cplex
		error = CPXmipopt(env,lp);
		if ( error ){
			log_fatal("CPX code %d : CPXmipopt() error", error); 
			tsp_handlefatal(inst);
		}

		int ncols = CPXgetnumcols(env, lp);

		// get the optimal value
		double* xstar = (double *) calloc(ncols, sizeof(double));
		error = cx_check_optimal(env, lp, xstar, ncols, inst);
		if(error){
			log_fatal("code %d : error in get_optimal", error);
			tsp_handlefatal(inst);
		}

		int ncomp = 0;
		int* comp = (int*) calloc(ncols, sizeof(int));
		cx_build_sol(xstar, inst, comp, &ncomp, &solution);

		log_info("number of components: %d", ncomp);

		// only one component it means that we have found an Hamiltonian cycle
		if(ncomp == 1){
			break;
		}

		cx_add_sec(env, lp, comp, ncomp, iteration, inst);

		iteration++;
	}

	// TODO: why doesnt work with tsp_update_solution
	memcpy(inst->best_solution.path, solution.path, inst->nnodes * sizeof(int));
    inst->best_solution.cost = solution.cost;
	log_debug("sol: %f", inst->best_solution.cost);

	free(solution.path);
	
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

	if(inst->options_t.seed != -1){
		CPXsetintparam(env, CPX_PARAM_RANDOMSEED, inst->options_t.seed);		
	}

	if(inst->options_t.timelimit > 0.0){
		CPXsetdblparam(env, CPX_PARAM_TILIM, inst->options_t.timelimit); 
	}

	return OK;
}

ERROR_CODE cx_check_optimal(CPXENVptr env, CPXLPptr lp, double* xstar, int ncols, instance* inst){
	// use the optimal solution found by CPLEX

	// array to hold the optimal solution

	if ( CPXgetx(env, lp, xstar, 0, ncols-1) ){
		log_fatal("CPX : CPXgetx() error");	
		tsp_handlefatal(inst);
	} 

	/*if(err_dolog()){
		// print the model
		for ( int i = 0; i < inst->nnodes; i++ ){
			for ( int j = i+1; j < inst->nnodes; j++ ){
				if ( xstar[cx_xpos(i,j,inst)] > 0.5 ){
					printf("  ... x(%3d,%3d) = 1\n", i+1,j+1);
				} 
			}
		}
	}*/

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

ERROR_CODE cx_add_sec(CPXENVptr env, CPXLPptr lp, int* comp, int ncomp, int iteration, instance* inst){
	if(ncomp == 1){
		return INVALID_ARGUMENT;
	}

	const char sense='L';

	// initialize index and value
	int ncols = CPXgetnumcols(env, lp);
	int* index = (int*) calloc(ncols, sizeof(int));
	double* value = (double*) calloc(ncols, sizeof(double));

	// column name initialization
	char **cname = (char **) calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *) calloc(100, sizeof(char));

	for(int k=1; k<ncomp+1; k++){
		int nnz=0;

		int number_nodes = 0; // |S|

		sprintf(cname[0], "it(%d)-row(%d)", iteration, k);  

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
		CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]);
	}

	return OK;
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
		
		// go to the next component...
	}
}



/*
**** LAZY CONTRAINTS IN THE INPUT MODEL ****

Ex: MZT formulation with directed-arc variables x_ij and x_ji --> xpos_compact(i,j,inst)


...

	int izero = 0;
	int index[3]; 
	double value[3];

	// add lazy constraints  1.0 * u_i - 1.0 * u_j + M * x_ij <= M - 1, for each arc (i,j) not touching node 0	
	double big_M = inst->nnodes - 1.0;
	double rhs = big_M -1.0;
	char sense = 'L';
	int nnz = 3;
	for ( int i = 1; i < inst->nnodes; i++ ) // excluding node 0
	{
		for ( int j = 1; j < inst->nnodes; j++ ) // excluding node 0 
		{
			if ( i == j ) continue;
			sprintf(cname[0], "u-consistency for arc (%d,%d)", i+1, j+1);
			index[0] = upos(i,inst);	
			value[0] = 1.0;	
			index[1] = upos(j,inst);
			value[1] = -1.0;
			index[2] = xpos_compact(i,j,inst);
			value[2] = big_M;
			if ( CPXaddlazyconstraints(env, lp, 1, nnz, &rhs, &sense, &izero, index, value, cname) ) log_fatal("wrong CPXlazyconstraints() for u-consistency");
		}
	}
	
	// add lazy constraints 1.0 * x_ij + 1.0 * x_ji <= 1, for each arc (i,j) with i < j
	rhs = 1.0; 
	char sense = 'L';
	nnz = 2;
	for ( int i = 0; i < inst->nnodes; i++ ) 
	{
		for ( int j = i+1; j < inst->nnodes; j++ ) 
		{
			sprintf(cname[0], "SEC on node pair (%d,%d)", i+1, j+1);
			index[0] = xpos_compact(i,j,inst);
			value[0] = 1.0;
			index[1] = xpos_compact(j,i,inst);
			value[1] = 1.0;
			if ( CPXaddlazyconstraints(env, lp, 1, nnz, &rhs, &sense, &izero, index, value, cname) ) log_fatal("wrong CPXlazyconstraints on 2-node SECs");
		}
	}

...
*** SOME MAIN CPLEX'S PARAMETERS ***


	// increased precision for big-M models
	CPXsetdblparam(env, CPX_PARAM_EPINT, 0.0);		// very important if big-M is present
	CPXsetdblparam(env, CPX_PARAM_EPRHS, 1e-9);   						

	CPXsetintparam(env, CPX_PARAM_MIPDISPLAY, 4);
	if ( VERBOSE >= 60 ) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen
	CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);
	
	CPXsetdblparam(env, CPX_PARAM_TILIM, CPX_INFBOUND+0.0); 
	
	CPXsetintparam(env, CPX_PARAM_NODELIM, 0); 		// abort Cplex after the root node
	CPXsetintparam(env, CPX_PARAM_INTSOLLIM, 1);	// abort Cplex after the first incumbent update

	CPXsetdblparam(env, CPX_PARAM_EPGAP, 1e-4);  	// abort Cplex when gap below 0.01%	 
	

	
*** instance TESTBED for exact codes:

		all TSPlib instances with n <= 500
*/
