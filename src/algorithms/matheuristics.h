#include <cplex.h> 
#include "cplex_model.h" 
#include "../tsp.h"

#define STAGNATION_THRESHOLD 5
#define SMALL_IMPROV 3

/**
 * @brief Solves the TSP using Hard Fixing
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE mh_HardFixing(void);

/**
 * @brief Solves the TSP using Local Branching
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE mh_LocalBranching(void);

//================================================================================
// GENERAL UTILS
//================================================================================

/**
 * @brief Util for running mip solver with branch&cut with candidate and relaxation callback. Currently not stable
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param solution Pointer to solution struct to hold the result found
 * * @param time_available Time assigned to the mip solver
 * @return ERROR_CODE 
 */
ERROR_CODE mh_mipsolver(CPXENVptr env, CPXLPptr lp,  tsp_solution* solution, double time_available);

/**
 * @brief util for running mip solver with mipopt and patching
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param solution Pointer to solution struct to hold the result found
 * @param time_available Time assigned to the mip solver
 * @return ERROR_CODE 
 */
ERROR_CODE mh_mipsolver2(CPXENVptr env, CPXLPptr lp,  tsp_solution* solution, double time_available);

//================================================================================
// HARD FIXING UTILS
//================================================================================

/**
 * @brief Util for fixing variables for the Hard Fixing algorithm
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param solution Pointer to solution struct to know what variables to fix
 * @return ERROR_CODE 
 */
ERROR_CODE hf_fixing(CPXENVptr env, CPXLPptr lp, tsp_solution* solution);

/**
 * @brief Util for unfixing all previously fixed variables
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @return ERROR_CODE 
 */
ERROR_CODE hf_undofixing(CPXENVptr env, CPXLPptr lp);

//================================================================================
// LOCAL BRANCHING UTILS
//================================================================================

/**
 * @brief Util to add the local branching constraint
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param solution Pointer to solution struct to know how to write the constraint
 * @param k Degree of freedom 
 * @return ERROR_CODE 
 */
ERROR_CODE lb_add_constraint(CPXENVptr env, CPXLPptr lp,  tsp_solution* solution, int k);

/**
 * @brief Util to remove the last constraint added which corresponds to the local branching constraint
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @return ERROR_CODE 
 */
ERROR_CODE lb_remove_constraint(CPXENVptr env, CPXLPptr lp);

/**
 * @brief Computes Kstar, the minimum value of K needed to avoid generating cuts.
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param Kstar Pointer to int variable to hold the computed kstar
 * @return ERROR_CODE 
 */
ERROR_CODE lb_kstar(CPXENVptr env, CPXLPptr lp, int* Kstar);
