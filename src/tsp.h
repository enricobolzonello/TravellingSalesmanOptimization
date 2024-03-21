#ifndef TSP_H_
#define TSP_H_

/**
 * @file tsp.h
 * @author Enrico Bolzonello (enrico.bolzonello@studenti.unidp.it)
 * @brief TSP-specific utilities
 * @version 0.1
 * @date 2024-03-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "utils/plot.h"
#include<libgen.h>
#include <math.h>

typedef enum {
    ALG_GREEDY = 0,
    ALG_GREEDY_ITER = 1,
    ALG_2OPT_GREEDY = 2,
    ALG_TABU_SEARCH = 3
} algorithms;

typedef struct {
    double timelimit;           // time limit of the algorithm (in seconds)
    int seed;                   // seed for random generation, if not set by the user, defaults to current time
    bool graph_random;          // flag to indicate wheter the graph is randomly generated
    bool graph_input;           // flag to indicate wheter the graph is from the input file
    char* inputfile;            // input file path
    bool tofile;                // if true, plots will be saved in directory /plots
} options;

typedef struct {
    options options_t;

    algorithms alg;            // algorithm chosen 

    int nnodes;                 // number of nodes

    struct utils_clock c;       // clock
    
    bool points_allocated;
    point* points;              // dynamic array of points

    bool costs_computed;        
    double* costs;             // matrix of costs between pairs of points

    double best_solution_cost;
    int* best_solution_path;

    int starting_node;          // save the starting node of the best tour
} instance;

/**
 * @brief Initialize tsp instance with default parameters
 * 
 * @param inst 
 */
void tsp_init(instance* inst);

/**
 * @brief Parser for the command-line arguments
 * 
 * @param argc, number of arguments on command-line
 * @param argv, arguments on command-line
 * @param inst, pointer to an instance
 */
ERROR_CODE tsp_parse_commandline(int argc, char** argv, instance* inst);

/**
 * @brief Generate random points with given seed and number of nodes
 * 
 * @param inst, pointer to an instance
 */
ERROR_CODE tsp_generate_randompoints(instance* inst);

/**
 * @brief Plots instance points 
 * 
 * @param inst, pointer to an instance
 * @return ERROR_CODE
 */
ERROR_CODE tsp_plot_points(instance* inst);

/**
 * @brief Plots instance solution
 * 
 * @param inst 
 * @return ERROR_CODE 
 */
ERROR_CODE tsp_plot_solution(instance* inst);

/**
 * @brief Frees all dynamically allocated resources
 * 
 * @param inst, pointer to an instance
 */
void tsp_free_instance(instance *inst);

/**
 * @brief Util to handle fatal errors, frees all allocated resources
 * 
 * @param inst 
 */
void tsp_handlefatal(instance *inst);

/**
 * @brief Reads a TSPLIB formatted input file
 * 
 * @param inst, pointer to an instance
 */
void tsp_read_input(instance* inst);

/**
 * @brief Precomputes costs and keeps them in matrix costs of the instance
 * 
 * @param inst 
 */
ERROR_CODE tsp_compute_costs(instance* inst);

/**
 * @brief Validates a tsp solution
 * 
 * @param inst 
 * @return true if the solution is valid
 * @return false if the solution is not valid
 */
bool tsp_validate_solution(instance* inst, int* current_solution_path);

/**
 * @brief Updates the best solution iff it is valid and it is better than the current best solution
 * 
 * @param inst 
 */
void tsp_update_best_solution(instance* inst, double current_solution_cost, int* current_solution_path);

double tsp_get_cost(instance* inst, int i, int j);

#endif
