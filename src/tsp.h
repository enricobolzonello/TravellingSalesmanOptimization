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
#include "utils/utils.h"
#include "utils/plot.h"
#include "utils/errors.h"

typedef struct {
    double timelimit;              // time limit of the algorithm (in seconds)
    int seed;                   // seed for random generation, if not set by the user, defaults to current time
    bool graph_random;          // flag to indicate wheter the graph is randomly generated
    bool graph_input;           // flag to indicate wheter the graph is from the input file
    char* inputfile;            // input file path
} options;

typedef struct {
    options options_t;

    int nnodes;                 // number of nodes to be generated randomly
    point* points;              // dynamic array of points
} instance;

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
 * @brief Outputs a plot 
 * 
 * @param inst, pointer to an instance
 * @param name, title of the plot
 * @param to_file, if you want to save a jpg file
 */
ERROR_CODE tsp_plot_points(instance* inst, char* name, bool to_file);

/**
 * @brief Frees all dynamically allocated resources
 * 
 * @param inst, pointer to an instance
 */
void tsp_free_instance(instance *inst);

void tsp_handlefatal(instance *inst);

/**
 * @brief Reads a TSPLIB formatted input file
 * 
 * @param inst, pointer to an instance
 */
void tsp_read_input(instance* inst);

#endif
