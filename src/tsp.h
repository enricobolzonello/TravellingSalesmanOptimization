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
#include <libgen.h>
#include <math.h>

#define EPSILON -1.0E-7

/**
 * @brief Policies for Tabu Search
 * 
 */
typedef enum{
    POL_FIXED = 0,
    POL_SIZE = 1,
    POL_RANDOM = 2,
    POL_LINEAR = 3
} ts_policies;

typedef enum{
    EM_MAX = 0,
    EM_RANDOM = 1
} em_init;

typedef enum{
    BC_PROB = 0,
    BC_NODES = 1, 
    BC_DEPTH = 2
} bc_skip;

typedef enum {
    ALG_GREEDY = 0,
    ALG_GREEDY_ITER = 1,
    ALG_2OPT_GREEDY = 2,
    ALG_TABU_SEARCH = 3,
    ALG_VNS = 4,
    ALG_CX_NOSEC = 5,
    ALG_CX_BENDERS = 6,
    ALG_EXTRAMILEAGE = 7,
    ALG_CX_BENDERS_PAT = 8,
    ALG_CX_BRANCH_AND_CUT = 9
} algorithms;

typedef struct {
    double cost;
    int* path;
}tsp_solution;

typedef struct {
    // General options
    double timelimit;           // time limit of the algorithm (in seconds)
    int seed;                   // seed for random generation, if not set by the user, defaults to current time
    bool graph_random;          // flag to indicate wheter the graph is randomly generated
    bool graph_input;           // flag to indicate wheter the graph is from the input file
    char* inputfile;            // input file path
    bool tofile;                // if true, plots will be saved in directory /plots
    int k;

    // Tabu Search options
    ts_policies policy;            // how to update tenure

    // Extra Mileage options
    em_init mileage_init;       // how to initialize extra mileage

    // Branch and Cut options
    bool init_mip;              // set MIP start for CPLEX branch & cut
    bc_skip skip_policy;            // skip policy for branch & cut fractional callback
    bool callback_relaxation;   // if true, it also calls callback for relaxation

} options;

typedef struct {
    int tenure;                 // current tenure
    int max_tenure;             // maximum possible tenure
    int min_tenure;             // minimum possible tenure

    bool increment;             // flag for the linear policy

    int* tabu_list;             // tabu list, nnodes long, each element is the last iteration the element has been encountered
} tabu_search;

typedef struct {
    options options_t;

    algorithms alg;            // algorithm chosen 

    int nnodes;                 // number of nodes

    struct timespec c;       // clock
    
    point* points;              // dynamic array of points
       
    double* costs;             // matrix of costs between pairs of points

    tsp_solution best_solution;

    int starting_node;          // save the starting node of the best tour

    int ncols;
    int* threads_seeds;
} instance;

/**
 * @brief Initialize tsp instance with default parameters
 * 
 * @param inst 
 */
void tsp_init(instance* inst);

/**
 * @brief Initialize solution struct
 * 
 * @param nnodes number of nodes of the instance
 * @return tsp_solution 
 */
tsp_solution tsp_init_solution(int nnodes);

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
ERROR_CODE tsp_update_best_solution(instance* inst, tsp_solution* solution);

/**
 * @brief Get cost of edge i-j, returns -1 if it does not exist
 * 
 * @param inst tsp instance
 * @param i node i
 * @param j node j
 * @return double cost of edge i-j
 */
double tsp_get_cost(instance* inst, int i, int j);

tsp_solution tsp_init_solution(int nnodes);

bool isTour(int path[], int n);

double solutionCost(instance *inst, int path[]);

#endif
