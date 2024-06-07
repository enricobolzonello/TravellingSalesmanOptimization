#ifndef MHEUR_H_
#define MHEUR_H_

#include "heuristics.h"

#define UPPER 10
#define LOWER 2

// https://www.sciencedirect.com/science/article/abs/pii/S0305054897000300?via%3Dihub
#define MAX_FRACTION 0.25
#define MIN_FRACTION 0.125

//================================================================================
// TABU SEARCH
//================================================================================

// https://www.scirp.org/journal/paperinformation?paperid=19930
ERROR_CODE tabu_fixed_policy( tabu_search* t, int value);
ERROR_CODE tabu_dependent_policy(tabu_search* t);
ERROR_CODE tabu_random_policy( tabu_search* t);
ERROR_CODE tabu_linear_policy( tabu_search* ts);

ERROR_CODE mh_TabuSearch(void);

ERROR_CODE tabu_init(tabu_search* ts, int nnodes);
ERROR_CODE tabu_best_move(int* solution_path, double* solution_cost, tabu_search* ts, int current_iteration);

//================================================================================
// VARIABLE NEIGHBORHOOD SEARCH
//================================================================================

ERROR_CODE mh_VNS(void);

ERROR_CODE vns_kick(tsp_solution* solution);


//================================================================================
// UTILS
//================================================================================

/**
 * @brief Util to check if element b in current_iteration is in the tabu list
 * 
 * @param ts tabu_search instance
 * @param b node to check
 * @param current_iteration 
 * @return true if element b is in the tabu list
 * @return false if element b is not in the tabu list
 */
bool is_in_tabu_list(tabu_search* ts, int b, int current_iteration);

/**
 * @brief Util to free all resources for tabu search
 * 
 * @param ts tabu_search instance
 */
void tabu_free(tabu_search* ts);

ERROR_CODE tabu_make_move(int *prev, tsp_solution* solution, int bestCase, int i, int succ_i, int j, int succ_j, int k, int succ_k);

#endif
