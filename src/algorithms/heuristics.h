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

//================================================================================
// NEAREST NEIGHBOUR HEURISTIC
//================================================================================

/**
 * @brief Solves the TSP with the Nearest Neighbor heuristic
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE h_Greedy(void);

/**
 * @brief Solves the TSP with the All Nearest Neighbor heuristic
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE h_Greedy_iterative(void);

/**
 * @brief Solves the TSP with the Nearest Neighbor heuristic + 2-OPT
 * 
 * @return ERROR_CODE 
 */
ERROR_CODE h_greedy_2opt(void);

//================================================================================
// EXTRA MILEAGE HEURISTIC
//================================================================================

/**
 * @brief Solves the TSP with the Extra Mileage heuristic
 * 
 * @param inst tsp instance
 * @return ERROR_CODE 
 */
ERROR_CODE h_ExtraMileage(void);

/**
 * @brief Solves the TSP with the All Nearest Neighbor heuristic + 2-OPT. The edge costs are given as an argument so they can be modified as you want.
 * 
 * @param solution Tsp solution struct to hold the best solution
 * @param costs Matrix of costs
 * @return ERROR_CODE 
 */
ERROR_CODE h_Greedy_2opt_mod_costs(tsp_solution* solution, double* costs);

//================================================================================
// UTILS
//================================================================================

/**
 * @brief Solves with Nearest Neighbor heuristic starting from a fixed node
 * 
 * @param starting_node Integer value between 0 and |V|-1 to indicate where the NN heuristic starts
 * @param solution Tsp solution struct to hold the best solution
 * @param costs Matrix of costs
 * @return ERROR_CODE 
 */
ERROR_CODE h_greedyutil(int starting_node, tsp_solution* solution, double* costs);

/**
 * @brief Solves with Extra Mileage heuristic starting from two nodes
 * 
 * @param solution Tsp solution struct to hold the best solution
 * @param nodeA First node
 * @param nodeB Second node
 * @return ERROR_CODE 
 */
ERROR_CODE h_extramileage_util(tsp_solution* solution, int nodeA, int nodeB);

#endif
