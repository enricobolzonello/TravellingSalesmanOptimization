#ifndef TABU_H_
#define TABU_H_

#include "../tsp.h"


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


// https://www.scirp.org/journal/paperinformation?paperid=19930

ERROR_CODE tabu_fixed_policy(tenure_policy* t, int value);
ERROR_CODE tabu_dependent_policy(tenure_policy* t, int nnodes);
ERROR_CODE tabu_random_policy(tenure_policy* t, int nnodes);

ERROR_CODE tabu_linear_policy(tenure_policy* t, int current_iteration, int nnodes);

#endif
