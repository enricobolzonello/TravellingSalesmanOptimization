#ifndef HEUR_H_
#define HEUR_H_

/**
 * @file heuristics.h
 * @author Enrico Bolzonello (enrico.bolzonello@studenti.unipd.it), Riccardo Vendramin (riccardo.vendramin.1@studenti.unipd.it)
 * @brief 
 * @version 0.1
 * @date 2024-03-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "../tsp.h"
#include "refinment.h"

// https://www.sciencedirect.com/science/article/abs/pii/S0305054897000300?via%3Dihub
#define MAX_FRACTION 0.25
#define MIN_FRACTION 0.125

/**
 * @brief Policies for Tabu Search
 * 
 */
typedef enum{
    POL_FIXED = 0,
    POL_SIZE = 1,
    POL_RANDOM = 2,
    POL_LINEAR = 3
} POLICIES;

/**
 * @brief Tabu search utils
 * 
 */
typedef struct {
    int tenure;                 // current tenure
    int max_tenure;             // maximum possible tenure
    int min_tenure;             // minimum possible tenure

    POLICIES policy;            // policy chosen

    bool increment;             // flag for the linear policy

    int* tabu_list;             // tabu list, nnodes long, each element is the last iteration the element has been encountered
} tabu_search;

//================================================================================
// NEAREST NEIGHBOUR HEURISTIC
//================================================================================

/**
 * @brief Runs greedy algorithm and updates the best solution
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE h_Greedy(instance *inst);

/**
 * @brief Runs greedy iteratively on all nodes and picks the best solution
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE h_Greedy_iterative(instance* inst);

/**
 * @brief Runs greedy iteratively on all nodes and perform 2-opt on each solution until no improvement
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE h_greedy_2opt(instance* inst);

//================================================================================
// TABU SEARCH
//================================================================================

// https://www.scirp.org/journal/paperinformation?paperid=19930
ERROR_CODE tabu_fixed_policy(tabu_search* t, int value);
ERROR_CODE tabu_dependent_policy(tabu_search* t);
ERROR_CODE tabu_random_policy(tabu_search* t);
ERROR_CODE tabu_linear_policy(tabu_search* ts);

ERROR_CODE tabu_init(tabu_search* ts, int nnodes, POLICIES policy);
ERROR_CODE tabu_search_2opt(instance* inst, POLICIES policy);
ERROR_CODE tabu_best_move(instance* inst, int* solution_path, double* solution_cost, tabu_search* ts, int current_iteration);

//================================================================================
// UTILS
//================================================================================

/**
 * @brief Solves with nearest neighbor heuristic starting from a fixed point
 * 
 * @param inst 
 * @param starting_node 
 * @return ERROR_CODE 
 */
ERROR_CODE h_greedyutil(instance* inst, int starting_node, int* solution_path, double* solution_cost);

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

#endif
