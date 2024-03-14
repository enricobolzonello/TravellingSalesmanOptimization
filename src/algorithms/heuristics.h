#ifndef HEUR_H_
#define HEUR_H_

#include "../tsp.h"

/**
 * @brief Solves with nearest neighbor heuristic starting from a fixed point
 * 
 * @param inst 
 * @param starting_node 
 * @return ERROR_CODE 
 */
ERROR_CODE h_greedyutil(instance* inst, int starting_node, int* solution_path, double* solution_cost);

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
 * @brief Runs the 2opt algorithm
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE h_2opt(instance* inst);

/**
 * @brief Util for reversing a path that goes from start to end
 * 
 * @param inst 
 * @param start node where the path to be reversed starts
 * @param end node where the path to be reversed ends
 * @param prev adjacency list of the paths before the reversing
 */
void h_reverse_path(instance *inst, int start_node, int end_node, int *prev, int* solution_path);

double h_2opt_once(instance* inst, int* solution_path);

/**
 * @brief Runs greedy iteratively on all nodes and perform 2-opt on each solution until no improvement
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE h_greedy_2opt(instance* inst);



#endif
