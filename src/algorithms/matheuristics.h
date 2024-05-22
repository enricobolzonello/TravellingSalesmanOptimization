#include <cplex.h> 
#include "cplex_model.h" 
#include "../tsp.h"

ERROR_CODE mh_HardFixing(instance* inst);

ERROR_CODE hf_fixing(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution);

ERROR_CODE hf_undofixing(CPXENVptr env, CPXLPptr lp, instance* inst);

ERROR_CODE hf_mipsolver(CPXENVptr env, CPXLPptr lp, instance* inst, tsp_solution* solution);
