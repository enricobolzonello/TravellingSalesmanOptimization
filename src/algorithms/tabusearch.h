#ifndef TABU_H_
#define TABU_H_

#include "heuristics.h"

// https://www.sciencedirect.com/science/article/abs/pii/S0305054897000300?via%3Dihub
#define MAX_FRACTION 0.25
#define MIN_FRACTION 0.125

typedef enum{
    POL_FIXED = 0,
    POL_SIZE = 1,
    POL_RANDOM = 2,
    POL_LINEAR = 3
} POLICIES;

typedef struct {
    int tenure;
    int max_tenure;
    int min_tenure;

    POLICIES policy;            // policy chosen

    bool increment;             // flag for the linear policy

    int* tabu_list;
} tabu_search;


// https://www.scirp.org/journal/paperinformation?paperid=19930

ERROR_CODE tabu_fixed_policy(tabu_search* t, int value);
ERROR_CODE tabu_dependent_policy(tabu_search* t);
ERROR_CODE tabu_random_policy(tabu_search* t);
ERROR_CODE tabu_linear_policy(tabu_search* ts);

ERROR_CODE tabu_init(tabu_search* ts, int nnodes, POLICIES policy);
ERROR_CODE tabu_search_2opt(instance* inst, POLICIES policy);
ERROR_CODE tabu_best_move(instance* inst, int* solution_path, double* solution_cost, tabu_search* ts, int current_iteration);

bool is_in_tabu_list(tabu_search* ts, int b, int current_iteration);
void tabu_free(tabu_search* ts);

#endif
