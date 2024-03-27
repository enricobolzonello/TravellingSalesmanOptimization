#include "../tsp.h"
#include <cplex.h>  

/**
 * @brief Solves the TSP finding the optimal solution with CPLEX
 * 
 * @param inst Tsp instance
 * @return ERROR_CODE 
 */
ERROR_CODE cx_TSPopt(instance *inst);

//================================================================================
// UTILS
//================================================================================

/**
 * @brief Map from edge (i,j) to position in the CPLEX matrix
 * 
 * @param i Start node
 * @param j End node
 * @param inst Tsp instance
 * @return int Position in the CPLEX matrix
 */
int cx_xpos(int i, int j, instance *inst);

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
void cx_build_sol(const double *xstar, instance *inst, int *comp, int *ncomp);
