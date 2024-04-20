#include "../tsp.h"
#include <cplex.h>  


typedef struct {
    int nnz;
    double rhs;
    int* index;
    double* value;
} cut;

/**
 * @brief Solves the TSP finding the optimal solution with CPLEX
 * 
 * @param inst Tsp instance
 * @return ERROR_CODE 
 */
ERROR_CODE cx_Nosec(instance *inst);

/**
 * @brief 
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE cx_BendersLoop(instance* inst, bool patching);

/**
 * @brief solve inst tsp problem using branch and cut implemented with CPLEX callback
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE cx_BranchAndCut(instance *inst);

//================================================================================
// UTILS
//================================================================================

/**
 * @brief initializes Cplex parameters
 * 
 * @param inst Tsp instance
 * @param env CPXENVptr
 * @param lp CPXLPptr
 * @return ERROR_CODE 
 */
ERROR_CODE cx_initialize(instance* inst, CPXENVptr env, CPXLPptr lp);

/**
 * @brief Map from edge (i,j) to position in the CPLEX matrix
 * 
 * @param i Start node
 * @param j End node
 * @param inst Tsp instance
 * @return int Position in the CPLEX matrix
 */
int cx_xpos(int i, int j, instance *inst);

ERROR_CODE cx_add_sec(CPXENVptr env, CPXLPptr lp, int* comp, int ncomp, instance* inst);

/**
 * @brief Builds the Mixed-Integer Problem in DFJ formulation (without subtour elimination constraint)
 * 
 * @param inst Tsp instance
 * @param env Pointer to CPLEX environment
 * @param lp Pointer to CPLEX linear problem
 */
void cx_build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

/**
 * @brief With the optimal solution of the MIP found by CPLEX, build the solution path and its cost
 * 
 * @param xstar Array holding the optimal solution
 * @param inst Tsp instance
 * @param comp An array indicating the component to which each node belongs
 * @param ncomp Number of independent components
 */
void cx_build_sol(const double *xstar, instance *inst, int *comp, int *ncomp, tsp_solution* solution);

/**
 * @brief Patch together the two highest cost subtours in the current solution
 * 
 * @param inst Tsp instance
 * @param comp array that indicates at which component the vertex i belongs
 * @param ncomp total number of components
 * @param solution current Tsp solution
 */
void cx_patching(instance *inst, int *comp, int *ncomp, tsp_solution* solution);

static int CPXPUBLIC callback_branch_and_cut(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);
