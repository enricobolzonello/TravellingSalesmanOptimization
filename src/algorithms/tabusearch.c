#include "tabusearch.h"

ERROR_CODE tabu_fixed_policy(tenure_policy* t, int value){
    if(t->dependent_policy || t->fixed_policy || t->random_policy){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    t->fixed_policy = true;

    t->tenure = value;

    return OK;
}

ERROR_CODE tabu_dependent_policy(tenure_policy* t, int nnodes){
    if(t->fixed_policy || t->random_policy){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    t->dependent_policy = true;
    t->tenure = (int)ceil((MAX_FRACTION + MIN_FRACTION) * nnodes);

    return OK;
}

ERROR_CODE tabu_random_policy(tenure_policy* t, int nnodes){
    if(t->dependent_policy || t->fixed_policy){
        log_warn("policy has already been set");
        return ALREADY_EXISTS;
    }

    t->random_policy = true;

    int max_tenure = MAX_FRACTION * nnodes;
    int min_tenure = MIN_FRACTION * nnodes;

    t->tenure = (int)(rand() / RAND_MAX) * (max_tenure - min_tenure) + min_tenure;

    return OK;
}

ERROR_CODE tabu_linear_policy(tenure_policy* t, int current_iteration, int nnodes){
    t->changing_policy = true;
    
    int max_tenure = MAX_FRACTION * nnodes;
    int min_tenure = MIN_FRACTION * nnodes;

    if(max_tenure == t->tenure || min_tenure == t->tenure){
        t->increment = !t->increment;
    }

    if(t->increment){
        t->tenure++;
    }else{
        t->tenure--;
    }

    return OK;
}
