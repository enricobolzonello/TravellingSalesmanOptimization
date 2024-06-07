#include "../tsp.h"
#include "heuristics.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-function"
#include "../mincut.h"
#pragma GCC diagnostic pop

#include <cplex.h>  

#define EPSILON_BC 0.1

typedef struct{
    CPXCALLBACKCONTEXTptr context;
} violatedcuts_passparams;

/**
 * @brief Solves the TSP finding the optimal solution with CPLEX
 * 
 * @param inst Tsp instance
 * @return ERROR_CODE 
 */
ERROR_CODE cx_Nosec(void);

/**
 * @brief 
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE cx_BendersLoop(bool patching);

/**
 * @brief solve inst tsp problem using branch and cut implemented with CPLEX callback
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE cx_BranchAndCut(void);

//================================================================================
// GENERAL UTILS
//================================================================================

/**
 * @brief initializes Cplex parameters
 * 
 * @param inst Tsp instance
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
 * @param inst Tsp instance
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
 * @param inst Tsp instance
 * @return ERROR_CODE 
 */
ERROR_CODE cx_add_sec(CPXENVptr env, CPXLPptr lp, int* comp, int ncomp);

/**
 * @brief Builds the Mixed-Integer Problem in DFJ formulation (without subtour elimination constraint)
 * 
 * @param inst Tsp instance
 * @param env Pointer to CPLEX environment
 * @param lp Pointer to CPLEX linear problem
 */
void cx_build_model(CPXENVptr env, CPXLPptr lp);

/**
 * @brief With the optimal solution of the MIP found by CPLEX, build the solution path and its cost
 * 
 * @param xstar Array holding the optimal solution
 * @param inst Tsp instance
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
 * @param inst Tsp instance
 * @param comp array that indicates at which component the vertex i belongs
 * @param ncomp total number of components
 * @param solution current Tsp solution
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
 * @param inst Tsp instance
 * @param cuts Reference to an array that will hold the cuts
 * @return ERROR_CODE 
 */
ERROR_CODE cx_compute_cuts(int* comp, int ncomp, int* nnz, double* rhs, char* sense, int* matbeg, int* matind, double* matval);

/**
 * @brief 
 * 
 * @param env 
 * @param lp 
 * @param ncols 
 * @param xstar 
 * @return ERROR_CODE 
 */
ERROR_CODE cx_branchcut_util(CPXENVptr env, CPXLPptr lp, int ncols, double* xstar);

/**
 * @brief 
 * 
 * @param env 
 * @param lp 
 * @param solution 
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
 * @param contextid either CPX_CALLBACKCONTEXT_CANDIDATE or CPX_CALLBACKCONTEXT_RELAXATION, will fail otherwise
 * @param userhandle pointer to tsp instance (needs to be void for CPLEX compatibility)
 * @return int 0 if it is successful, 1 otherwise
 */
static int CPXPUBLIC callback_branch_and_cut(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);

/**
 * @brief Callback function for the candidate solution
 * 
 * @param context CPXCALLBACKCONTEXTptr
 * @param inst Tsp instance
 * @return int 0 if it is successful, 1 otherwise
 */
static int CPXPUBLIC callback_candidate(CPXCALLBACKCONTEXTptr context);

/**
 * @brief Callback function for the relaxation
 * 
 * @param context CPXCALLBACKCONTEXTptr
 * @param inst Tsp instance
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
