#include <cplex.h> 
#include "cplex_model.h" 
#include "../tsp.h"

#define STAGNATION_THRESHOLD 5

/**
 * @brief runs Hard Fixing algorithm
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE mh_HardFixing(void);

/**
 * @brief runs Local Branching algorithm
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE mh_LocalBranching(void);

//================================================================================
// GENERAL UTILS
//================================================================================

/**
 * @brief util for running mip solver with branch&cut with candidate and relaxation callback. Currently not stable
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param inst Tsp instance
 * @param solution pointer to solution struct to hold the result found
 * @return ERROR_CODE 
 */
ERROR_CODE mh_mipsolver(CPXENVptr env, CPXLPptr lp,  tsp_solution* solution);

/**
 * @brief util for running mip solver with mipopt and patching
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param inst Tsp instance
 * @param solution pointer to solution struct to hold the result found
 * @param time_available time assigned to the mip solver
 * @return ERROR_CODE 
 */
ERROR_CODE mh_mipsolver2(CPXENVptr env, CPXLPptr lp,  tsp_solution* solution, double time_available);

//================================================================================
// HARD FIXING UTILS
//================================================================================

/**
 * @brief util for fixing variables for the Hard Fixing algorithm
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param inst Tsp instance
 * @param solution pointer to solution struct to know what variables to fix
 * @return ERROR_CODE 
 */
ERROR_CODE hf_fixing(CPXENVptr env, CPXLPptr lp, tsp_solution* solution);

/**
 * @brief util for unfixing all previously fixed variables
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param inst Tsp instance
 * @return ERROR_CODE 
 */
ERROR_CODE hf_undofixing(CPXENVptr env, CPXLPptr lp);

//================================================================================
// LOCAL BRANCHING UTILS
//================================================================================

/**
 * @brief util to add the local branching constraint
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param inst Tsp instance
 * @param solution pointer to solution struct to know how to write the constraint
 * @param k degree of freedom 
 * @return ERROR_CODE 
 */
ERROR_CODE lb_add_constraint(CPXENVptr env, CPXLPptr lp,  tsp_solution* solution, int k);

/**
 * @brief util to remove the last constraint added which corresponds to the local branching constraint
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @return ERROR_CODE 
 */
ERROR_CODE lb_remove_constraint(CPXENVptr env, CPXLPptr lp);
