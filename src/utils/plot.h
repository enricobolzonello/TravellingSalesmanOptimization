#ifndef PLOT_H_
#define PLOT_H_

/**
 * @file plot.h
 * @author Enrico Bolzonello (enrico.bolzonello@studenti.unidp.it)
 * @brief Utility functions to plot tsp instances
 * @version 0.1
 * @date 2024-03-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "utils.h"

typedef FILE* PLOT;

/**
 * @brief Opens a new gnuplot
 * 
 * @param   title Name of the plot 
 * @return  PLOT Plot instance
 */
PLOT plot_open(char* title);

/**
 * @brief Plots a single point
 * 
 * @param plot  Plot instance
 * @param p Pointer to a point struct
 */
void plot_point(PLOT plot, point* p);

/**
 * @brief Plots an edge between two points
 * 
 * @param plot  Plot instance
 * @param u Point struct 
 * @param v Point struct 
 */
void plot_edge(PLOT plot, point u, point v);

/**
 * @brief Saves the plot to a jpg file in the directory /plots
 * 
 * @param plot Plot instance
 * @param filename Name of the output file
 */
void plot_tofile(PLOT plot, char* filename);

/**
 * @brief Add additional arguments to the plot, like style options
 * 
 * @param plot  Plot instance
 * @param args  String with gnuplot commands
 */
void plot_args(PLOT plot, char* args);

void plot_stats(PLOT plot, char* filename);

/**
 * @brief Free resources
 * 
 * @param plot  Plot instance
 */
void plot_free(PLOT plot);

#endif
