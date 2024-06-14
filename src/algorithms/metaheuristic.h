#ifndef MHEUR_H_
#define MHEUR_H_

#include "heuristics.h"

// maximum and minimum number of kicks in VNS
#define UPPER 10
#define LOWER 2

// maximum and minimum fraction of nodes to calculate tabu list size 
// based on https://www.sciencedirect.com/science/article/abs/pii/S0305054897000300?via%3Dihub
#define MAX_FRACTION 0.25
#define MIN_FRACTION 0.125


/**
 * @brief Solves the TSP with Tabu Search
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE mh_TabuSearch(void);

/**
 * @brief Solves the TSP using Variable Neighborhood Search
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE mh_VNS(void);


//================================================================================
// VARIABLE NEIGHBORHOOD SEARCH UTILS
//================================================================================

/**
 * @brief Util to give a kick to the current solution. It is implemented as a 3-OPT kick
 * 
 * @param solution Current tsp solution
 * @return ERROR_CODE 
 */
ERROR_CODE vns_kick(tsp_solution* solution);


//================================================================================
// TABU SEARCH POLICIES
// mostly based on https://www.scirp.org/journal/paperinformation?paperid=19930
//================================================================================

/**
 * @brief Fix the tabu list size as value
 * 
 * @param t Tabu_search struct pointer
 * @param value Tabu list size
 * @return ERROR_CODE 
 */
ERROR_CODE tabu_fixed_policy( tabu_search* t, int value);

/**
 * @brief Computes the tabu list size as the average between a max and min tenure, defined on fraction of nodes
 * 
 * @param t Tabu_search struct pointer
 * @return ERROR_CODE 
 */
ERROR_CODE tabu_dependent_policy(tabu_search* t);

/**
 * @brief Fixes the tabu list size as a random number between the max and min tenure, defined as in tabu_dependent_policy
 * 
 * @param t Tabu_search struct pointer
 * @return ERROR_CODE 
 */
ERROR_CODE tabu_random_policy( tabu_search* t);

/**
 * @brief Starting from the minimum tenure, at each iteration it grows until reaching the maximum. At this point, it will start to descend until the minimum. This process repeats until the end of the algorithm.

 * 
 * @param ts Tabu_search struct pointer
 * @return ERROR_CODE 
 */
ERROR_CODE tabu_linear_policy( tabu_search* ts);


//================================================================================
// TABU SEARCH UTILS
//================================================================================

/**
 * @brief Initializes tabu_search struct
 * 
 * @param ts Tabu_search struct pointer
 * @param nnodes Number of nodes
 * @return ERROR_CODE 
 */
ERROR_CODE tabu_init(tabu_search* ts, int nnodes);

/**
 * @brief Find and executes best move for the tabu search algorithm
 * 
 * @param solution_path Current solution path
 * @param solution_cost Current solution cost
 * @param ts Tabu_search struct pointer
 * @param current_iteration Integer that indicates the current iteration 
 * @return ERROR_CODE 
 */
ERROR_CODE tabu_best_move(int* solution_path, double* solution_cost, tabu_search* ts, int current_iteration);

/**
 * @brief Util to check if element b in current_iteration is in the tabu list
 * 
 * @param ts Tabu_search instance
 * @param b Node to check
 * @param current_iteration Integer value to indicate the current iteration of the loop. Used to check whether the node has been used recently
 * @return true If element b is in the tabu list
 * @return false If element b is not in the tabu list
 */
bool is_in_tabu_list(tabu_search* ts, int b, int current_iteration);

/**
 * @brief Util to free all resources for tabu search
 * 
 * @param ts Tabu_search instance
 */
void tabu_free(tabu_search* ts);

/**
 * @brief Executes the 3-OPT move 
 * 
 * @param prev 
 * @param solution 
 * @param bestCase 
 * @param i 
 * @param succ_i 
 * @param j 
 * @param succ_j 
 * @param k 
 * @param succ_k 
 * @return ERROR_CODE 
 */
ERROR_CODE tabu_make_move(int *prev, tsp_solution* solution, int bestCase, int i, int succ_i, int j, int succ_j, int k, int succ_k);

#endif
