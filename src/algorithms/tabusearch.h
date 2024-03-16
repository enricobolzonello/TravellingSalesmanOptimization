#ifndef TABU_H_
#define TABU_H_

#include "../tsp.h"
#include "heuristics.h"


#define ITERATIONS 100

// https://www.sciencedirect.com/science/article/abs/pii/S0305054897000300?via%3Dihub
#define MAX_FRACTION 0.25
#define MIN_FRACTION 0.125

typedef struct {
    int tenure;

    bool fixed_policy;          // fixed tenure
    bool dependent_policy;      // dependent on the size of the instance
    bool random_policy;         // random choice between max and min tenure

    bool changing_policy;       // if it changes during iterations
    bool increment;             // flag for the linear policy
} tenure_policy;

typedef struct {
    tenure_policy tenure_policy;

    int* tabu_list;

    int number_iterations;
} tabu_search;


// https://www.scirp.org/journal/paperinformation?paperid=19930

ERROR_CODE tabu_fixed_policy(tenure_policy* t, int value);
ERROR_CODE tabu_dependent_policy(tenure_policy* t, int nnodes);
ERROR_CODE tabu_random_policy(tenure_policy* t, int nnodes);
ERROR_CODE tabu_linear_policy(tenure_policy* t, int current_iteration, int nnodes);

ERROR_CODE init_tabu_search(tabu_search* ts, int nnodes);

ERROR_CODE tabu_search_2opt(instance* inst);

ERROR_CODE tabu_best_move(instance* inst, int* solution_path, double* solution_cost, tabu_search* ts, int current_iteration);

bool is_in_tabu_list(tabu_search* ts, int b, int current_iteration);

#endif
