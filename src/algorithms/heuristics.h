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
 * @param inst tsp instance
 * @return ERROR_CODE 
 */
ERROR_CODE h_greedy_2opt(instance* inst);

//================================================================================
// EXTRA MILEAGE HEURISTIC
//================================================================================

/**
 * @brief Runs Extra Mileage heuristic
 * 
 * @param inst tsp instance
 * @return ERROR_CODE 
 */
ERROR_CODE h_ExtraMileage(instance* inst);

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
 * @brief 
 * 
 * @param inst 
 * @param solution 
 * @param nodeA 
 * @param nodeB 
 * @return ERROR_CODE 
 */
ERROR_CODE h_extramileage_util(instance* inst, tsp_solution* solution, int nodeA, int nodeB);

#endif
