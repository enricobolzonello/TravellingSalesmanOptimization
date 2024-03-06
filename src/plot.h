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
 * @param title, name of the plot 
 * @return PLOT, plot instance
 */
PLOT plot_open(char* title);

/**
 * @brief Plots a single point
 * 
 * @param plot 
 * @param p 
 */
void plot_point(PLOT plot, point* p);

/**
 * @brief Saves the plot to a jpg file in the directory /plots
 * 
 * @param plot, plot instance
 * @param filename, name of the output file
 */
void plot_tofile(PLOT plot, char* filename);

/**
 * @brief Add additional arguments to the plot, like style options
 * 
 * @param plot, plot instance
 * @param args, string with gnuplot commands
 */
void plot_args(PLOT plot, char* args);

/**
 * @brief Free resources
 * 
 * @param plot, plot instance
 */
void plot_free(PLOT plot);

#endif
