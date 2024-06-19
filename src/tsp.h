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
    ALG_CX_BRANCH_AND_CUT = 9,
    ALG_HARD_FIXING = 10,
    ALG_LOCAL_BRANCHING = 11
} algorithms;

typedef struct {
    double cost;
    int* path;
    point* points;
    int nnodes;
    double execution_time;
} return_struct;

typedef struct {
    // General options
    double timelimit;           // time limit of the algorithm (in seconds)
    int seed;                   // seed for random generation, if not set by the user, defaults to current time
    bool graph_random;          // flag to indicate wheter the graph is randomly generated
    bool graph_input;           // flag to indicate wheter the graph is from the input file
    char* inputfile;            // input file path
    bool tofile;                // if true, plots will be saved in directory /plots
    int k;                      // number of iterations of VNS and Tabu Search, by default sets to the maximum integer

    // Tabu Search options
    ts_policies policy;         // how to update tenure

    // Extra Mileage options
    em_init mileage_init;       // how to initialize extra mileage

    // Benders Loop options
    bool bl_patching;           // true if it uses patching, false otherwise. Defaults to true

    // Branch and Cut options
    bool init_mip;              // set MIP start for CPLEX branch & cut
    bc_skip skip_policy;        // skip policy for branch & cut fractional callback
    bool callback_relaxation;   // if true, it also calls callback for relaxation
    bool modified_costs;        // if true, post to CPLEX an heuristic solution with modified costs    

    // Hard Fixing options
    double hf_prob;             // probability to set an edge

    // Local Branching options
    bool lb_dynk;               // flag to indicate if use dynamic k
    int lb_initk;               // initial k
    double lb_improv;           // improvement needed to update k
    int lb_delta;               // deltaK
    bool lb_kstar;              // calculate Kstar and pick the average between 0 and Kstar as K starting point

} options;

typedef struct {
    int tenure;                 // current tenure
    int max_tenure;             // maximum possible tenure
    int min_tenure;             // minimum possible tenure

    bool increment;             // flag for the linear policy

    int* tabu_list;             // tabu list, nnodes long, each element is the last iteration the element has been encountered
} tabu_search;

typedef struct {

    algorithms alg;             // algorithm chosen 

    int nnodes;                 // number of nodes

    struct timespec c;          // clock
    
    point* points;              // dynamic array of points
       
    double* costs;              // matrix of costs between pairs of points

    tsp_solution best_solution; // current best solution found

    int starting_node;          // save the starting node of the best tour

    int* threads_seeds;         // seed for each thread

    int ncols;                  // # of columns of the cplex model
    int cplex_terminate;        // flag to signal cplex to stop

} instance;

/**
 * @brief Initialize tsp instance with default parameters
 * 
 */
void tsp_init(void);

/**
 * @brief Parser for the command-line arguments
 * 
 * @param argc, number of arguments on command-line
 * @param argv, arguments on command-line
 */
ERROR_CODE tsp_parse_commandline(int argc, char** argv);

/**
 * @brief Generate random points with given seed and number of nodes
 * 
 */
ERROR_CODE tsp_generate_randompoints(void);

/**
 * @brief Plots instance points 
 * 
 * @return ERROR_CODE
 */
ERROR_CODE tsp_plot_points(void);

/**
 * @brief Plots instance solution
 *  
 * @return ERROR_CODE 
 */
ERROR_CODE tsp_plot_solution(void);

/**
 * @brief Frees all dynamically allocated resources
 * 
 */
void tsp_free_instance(void);

/**
 * @brief Util to handle fatal errors, frees all allocated resources
 * 
 */
void tsp_handlefatal(void);

/**
 * @brief Reads a TSPLIB formatted input file
 * 
 */
void tsp_read_input(void);

/**
 * @brief Precomputes costs and keeps them in matrix costs of the instance
 * 
 */
ERROR_CODE tsp_compute_costs(void);

/**
 * @brief Validates a tsp solution
 * 
 * @return true if the solution is valid
 * @return false if the solution is not valid
 */
bool tsp_validate_solution(int nnodes, int* current_solution_path);

/**
 * @brief Updates the best solution iff it is valid and it is better than the current best solution
 * 
 */
ERROR_CODE tsp_update_best_solution(tsp_solution* solution);

/**
 * @brief Get cost of edge i-j, returns -1 if it does not exist
 * 
 * @param i node i
 * @param j node j
 * @return double cost of edge i-j
 */
double tsp_get_cost(int i, int j);

/**
 * @brief Checks if the path is actually a valid tour
 * 
 * @param path Array to hold the solution path
 * @param n Number of nodes
 * @return true If path is a tour
 * @return false Otherwise
 */
bool tsp_is_tour(int path[], int n);

/**
 * @brief Computes solution cost given a solution path
 * 
 * @param path Array to hold the solution path
 * @return double Solution cost
 */
double tsp_solution_cost(int path[]);

extern instance tsp_inst;
extern options tsp_env;

#endif
