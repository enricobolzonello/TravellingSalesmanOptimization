#ifndef REF_H_
#define REF_H_

/**
 * @file refinment.h
 * @author Enrico Bolzonello (enrico.bolzonello@studenti.unipd.it), Riccardo Vendramin (riccardo.vendramin.1@studenti.unipd.it)
 * @brief 
 * @version 0.1
 * @date 2024-03-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "../tsp.h"

/**
 * @brief 2opt refinment algorithm
 * 
 * @param inst tsp instance
 * @param solution solution struct
 * @return ERROR_CODE 
 */
ERROR_CODE ref_2opt(instance* inst, tsp_solution* solution, double* costs, bool update_incumbent);

//================================================================================
// UTILS
//================================================================================

/**
 * @brief Util for one 2opt move
 * 
 * @param inst tsp instance
 * @param solution_path solution struct
 * @return double best delta
 */
double ref_2opt_once(instance* inst, tsp_solution* solution, double* costs);

/**
 * @brief Util to reverse a path from start_node to end_node in solution_path
 * 
 * @param inst tsp instance
 * @param start_node 
 * @param end_node 
 * @param prev array path before the 2opt move
 * @param solution_path path that will be modified
 */
void ref_reverse_path(instance *inst, int a, int succ_a, int b, int succ_b, int *prev, int* solution_path);

#endif
