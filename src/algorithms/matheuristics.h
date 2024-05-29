#include <cplex.h> 
#include "cplex_model.h" 
#include "../tsp.h"

#define STAGNATION_THRESHOLD 5

ERROR_CODE mh_HardFixing(instance* inst);

ERROR_CODE mh_LocalBranching(instance* inst);

ERROR_CODE hf_fixing(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution);

ERROR_CODE hf_undofixing(CPXENVptr env, CPXLPptr lp, instance* inst);

ERROR_CODE hf_mipsolver(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution);

ERROR_CODE lb_add_constraint(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution, int k);

ERROR_CODE lb_remove_constraint(CPXENVptr env, CPXLPptr lp);

