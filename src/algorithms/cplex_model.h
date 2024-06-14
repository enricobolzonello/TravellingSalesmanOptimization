#include "../tsp.h"
#include "heuristics.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-function"
#include "../mincut.h"
#pragma GCC diagnostic pop

#include <cplex.h>  

#define EPSILON_BC 0.1
#define THREADS 32

typedef struct{
    CPXCALLBACKCONTEXTptr context;
} violatedcuts_passparams;

/**
 * @brief Solves the TSP finding the optimal solution with CPLEX, it has no subtour elimination constraint so the solution will not be valid
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE cx_Nosec(void);

/**
 * @brief Solves the TSP with the Benders algorithm, takes as parameter the flag patching to signify if we need to apply the Patching heuristic at each solution found
 * 
 * @param patching if True, apply Patching heuristic
 * @return ERROR_CODE 
 */
ERROR_CODE cx_BendersLoop(bool patching);

/**
 * @brief Solves the TSP using branch and cut implemented with CPLEX callback
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE cx_BranchAndCut(void);

//================================================================================
// GENERAL UTILS
//================================================================================

/**
 * @brief initializes Cplex parameters
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @return ERROR_CODE 
 */
ERROR_CODE cx_initialize(CPXENVptr env, CPXLPptr lp);

/**
 * @brief Map from edge (i,j) to position in the CPLEX matrix
 * 
 * @param i Start node
 * @param j End node
 * @param nnodes number of nodes
 * @return int Position in the CPLEX matrix
 */
int cx_xpos(int i, int j, int nnodes);

/**
 * @brief Util for Benders Loop method to add subtour elimination constraints to the model
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param comp An array indicating the component to which each node belongs
 * @param ncomp Number of independent components
 * @return ERROR_CODE 
 */
ERROR_CODE cx_add_sec(CPXENVptr env, CPXLPptr lp, int* comp, int ncomp);

/**
 * @brief Builds the Mixed-Integer Problem in DFJ formulation (without subtour elimination constraint)
 * 
 * @param env Pointer to CPLEX environment
 * @param lp Pointer to CPLEX linear problem
 */
void cx_build_model(CPXENVptr env, CPXLPptr lp);

/**
 * @brief With the optimal solution of the MIP found by CPLEX, build the solution path and its cost
 * 
 * @param xstar Array holding the optimal solution
 * @param comp An array indicating the component to which each node belongs
 * @param ncomp Number of independent components
 */
void cx_build_sol(const double *xstar, tsp_solution* solution);

/**
 * @brief Check CPLEX status codes after mip opt and returns an ERROR_CODE corresponding to its status
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @return ERROR_CODE 
 */
ERROR_CODE cx_handle_cplex_status(CPXENVptr env, CPXLPptr lp);


//================================================================================
// BENDERS UTILS
//================================================================================

/**
 * @brief Patch together the two highest cost subtours in the current solution
 * 
 * @param solution Current Tsp solution
 */
void cx_patching(tsp_solution* solution);

//================================================================================
// BRANCH & CUT UTILS
//================================================================================

/**
 * @brief Given the independent components, computes the cuts to be added to CPLEX
 * 
 * @param comp An array indicating the component to which each node belongs (starts from 1)
 * @param ncomp Total number of independent components
 * @param other Parameter for cplex
 * @return ERROR_CODE 
 */
ERROR_CODE cx_compute_cuts(int* comp, int ncomp, int* nnz, double* rhs, char* sense, int* matbeg, int* matind, double* matval);

/**
 * @brief 
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param ncols number of columns of the model
 * @param xstar array to hold the fractional solution found by cplex
 * @return ERROR_CODE 
 */
ERROR_CODE cx_branchcut_util(CPXENVptr env, CPXLPptr lp, int ncols, double* xstar);

/**
 * @brief Takes a valid TSP solution and adds it to the CPLEX model as a MIP start, to hopefully speed up the computation
 * 
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @param solution Tsp solution to add to the cplex model as a MIP start
 * @return ERROR_CODE 
 */
ERROR_CODE cx_add_mip_starts(CPXENVptr env, CPXLPptr lp, tsp_solution* solution);

//================================================================================
// CALLBACKS
//================================================================================

/**
 * @brief Callback function that will be called by CPLEX, both for candidate and relaxation. The role of this function is to call the appropriate callback function
 * 
 * @param context CPXCALLBACKCONTEXTptr
 * @param contextid Either CPX_CALLBACKCONTEXT_CANDIDATE or CPX_CALLBACKCONTEXT_RELAXATION, will fail otherwise
 * @param userhandle Pointer to data you want to pass (in our case NULL)
 * @return int 0 if it is successful, 1 otherwise
 */
static int CPXPUBLIC callback_branch_and_cut(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * @brief Callback function for the candidate solution
 * 
 * @param context CPXCALLBACKCONTEXTptr
 * @return int 0 if it is successful, 1 otherwise
 */
static int CPXPUBLIC callback_candidate(CPXCALLBACKCONTEXTptr context);

/**
 * @brief Callback function for the relaxation
 * 
 * @param context CPXCALLBACKCONTEXTptr
 * @return int 0 if it is successful, 1 otherwise
 */
static int CPXPUBLIC callback_relaxation(CPXCALLBACKCONTEXTptr context);

/**
 * @brief Callback function called by Concorde, corresponds to int (*doit_fn) in the documentation. 
 * 
 * @param cut_value Value of the cut
 * @param cut_nnodes Number of nodes in the cut
 * @param cut_indexes Array that specify the index of the nodes in the cut
 * @param userhandle pointer to violatedcuts_passparams struct (needs to be void for Concorde)
 * @return int 0 if it is successful, 1 otherwise
 */
int cc_add_violated_sec(double cut_value, int cut_nnodes, int* cut_indexes, void* userhandle);
