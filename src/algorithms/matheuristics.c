#include "matheuristics.h"

ERROR_CODE mh_HardFixing()
{
    ERROR_CODE e = T_OK;

    log_info("running Hard Fixing");

    // open CPLEX model
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error)
    {
        log_fatal("CPX code %d : CPXopenCPLEX() error", error);
        e = FAILED_PRECONDITION;
        goto mh_free;
    }
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error)
    {
        log_fatal("CPX code %d : CPXcreateprob() error", error);
        e = FAILED_PRECONDITION;
        goto mh_free;
    }

    // initialize CPLEX model
    e = cx_initialize(env, lp);
    if (!err_ok(e))
    {
        log_error("error in initializing cplex model");
        e = FAILED_PRECONDITION;
        goto mh_free;
    }

    log_info("CPLEX initialized correctly");

    // initialize the current solution as the best solution found by heuristic
    tsp_solution solution;
    tsp_init_solution(tsp_inst.nnodes, &solution);
    memcpy(solution.path, tsp_inst.best_solution.path, tsp_inst.nnodes * sizeof(int));
    solution.cost = tsp_inst.best_solution.cost;

    int i = 0;
    while (1)
    {
        // check if exceeds time
        double ex_time = utils_timeelapsed(&tsp_inst.c);
        if (tsp_env.timelimit != -1.0)
        {
            if (ex_time > tsp_env.timelimit)
            {
                log_warn("deadline exceeded in hard fixing");
                e = DEADLINE_EXCEEDED;
                break;
            }
        }

        log_info("iteration: %d", i);

        // add solution as a MIP start
        e = cx_add_mip_starts(env, lp, &solution);
        if (!err_ok(e))
        {
            log_error("error add mip start");
            goto mh_free;
        }

        // set remaining time limit for cplex
        // 1/10 of the total remaining time
        double time_remain = (tsp_env.timelimit - ex_time) / 10;
        log_debug("time assigned to mip solver : %.4f", time_remain);

        // FIXING
        e = hf_fixing(env, lp, &solution);
        if (!err_ok(e))
        {
            log_error("error in fixing");
            goto mh_free;
        }

        // MIP SOLVER
        e = mh_mipsolver2(env, lp, &solution, time_remain);
        if (!err_ok(e))
        {
            log_error("error in mip solver");
            goto mh_free;
        }

        log_info("MIP SOLVER DONE");

        e = tsp_update_best_solution(&solution);
        if (!err_ok(e))
        {
            log_error("code %d : error in updating best solution of Hard Fixing");
        }

        // FIXING UNDO
        e = hf_undofixing(env, lp);
        if (!err_ok(e))
        {
            log_error("error in undo fixing");
            goto mh_free;
        }

        i++;
    }

mh_free:
    utils_safe_free(solution.path);

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return e;
}

ERROR_CODE mh_LocalBranching()
{
    ERROR_CODE e = T_OK;

    log_info("running Local Branching");

    // open CPLEX model
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error)
    {
        log_fatal("CPX code %d : CPXopenCPLEX() error", error);
        e = FAILED_PRECONDITION;
        goto mh_free;
    }
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error)
    {
        log_fatal("CPX code %d : CPXcreateprob() error", error);
        e = FAILED_PRECONDITION;
        goto mh_free;
    }

    // initialize CPLEX model
    e = cx_initialize(env, lp);
    if (!err_ok(e))
    {
        log_error("error in initializing cplex model");
        e = FAILED_PRECONDITION;
        goto mh_free;
    }

    log_info("CPLEX initialized correctly");

    int K = tsp_env.lb_initk;

    tsp_solution solution;
    tsp_init_solution(tsp_inst.nnodes, &solution);
    // initialize the current solution as the best solution found by heuristic
    memcpy(solution.path, tsp_inst.best_solution.path, tsp_inst.nnodes * sizeof(int));
    solution.cost = tsp_inst.best_solution.cost;

    if (tsp_env.lb_kstar)
    {
        int Kstar;
        if(!err_ok(lb_kstar(env, lp, &Kstar))){
            log_warn("kstar computation failed, dropping back to fixed K");
        }else{
            K = Kstar / 2;
        }
    }

    log_info("initial K: %d", K);

    int st_counter = 0;
    int small_improv = 0;

    // file to hold solution value in each iteration
    FILE *f = fopen("results/LocalBranchingK.dat", "w+");
    int i = 0;

    while (1)
    {
        // check if exceeds time
        double ex_time = utils_timeelapsed(&tsp_inst.c);
        if (tsp_env.timelimit != -1.0)
        {
            if (ex_time > tsp_env.timelimit)
            {
                log_warn("deadline exceeded in local branching");
                e = DEADLINE_EXCEEDED;
                break;
            }
        }

        log_info("iteration %d", i);

        // save value of K
        fprintf(f, "%d,%d\n", i, K);

        // add solution as a MIP start
        e = cx_add_mip_starts(env, lp, &solution);
        if (!err_ok(e))
        {
            log_error("error add mip start");
            goto mh_free;
        }

        // Add LB constraint
        e = lb_add_constraint(env, lp, &solution, K);
        if (!err_ok(e))
        {
            log_error("error in adding local braching constraint");
            goto mh_free;
        }

        // set remaining time limit for cplex
        // 1/10 of the total remaining time
        double time_remain = (tsp_env.timelimit - ex_time) / 3;
        log_info("time assigned to mip solver : %.4f", time_remain);

        // MIP SOLVER
        e = mh_mipsolver2(env, lp, &solution, time_remain);
        if (!err_ok(e))
        {
            log_error("error in mip solver");
            goto mh_free;
        }

        log_info("mip solver done");

        // update k
        // POLICY
        // keep a stagnation counter to count how many iterations do not improve the solution, if it becomes greater than a threshold, increase k
        //
        // if the new solution is better:
        //  if the improvement is not much (for now 2%), we increase k to generate deeper cuts
        //  otherwise we stay at the k we are in

        if (tsp_env.lb_dynk)
        {
            if (solution.cost < tsp_inst.best_solution.cost)
            {

                log_info("solution cost: %.0f", solution.cost);
                log_info("best solution cost: %.0f", tsp_inst.best_solution.cost);
                double improvement = (1.0 - solution.cost / tsp_inst.best_solution.cost);
                log_info("improvement: %.4f", improvement);

                if (improvement < tsp_env.lb_improv)
                {
                    small_improv++;
                    if (small_improv % SMALL_IMPROV == 0)
                    {
                        K = max(K - tsp_env.lb_delta, 10);
                    }
                }
                else
                {
                    K += tsp_env.lb_delta;
                    small_improv = 0;
                }
            }
            else
            {
                st_counter++;

                if (st_counter > STAGNATION_THRESHOLD)
                {
                    st_counter = 0;
                    K += tsp_env.lb_delta;
                }
            }

            log_info("new K: %d", K);
        }

        // save the best solution
        e = tsp_update_best_solution(&solution);
        if (!err_ok(e))
        {
            log_error("code %d : error in updating best solution of Local Branching");
            goto mh_free;
        }

        // Remove LB constraint
        e = lb_remove_constraint(env, lp);
        if (!err_ok(e))
        {
            log_error("error in removing local braching constraint");
            goto mh_free;
        }

        i += 1;
    }

mh_free:
    fclose(f);
    utils_safe_free(solution.path);
    utils_safe_free(solution.comp);

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return e;
}

//================================================================================
// GENERAL UTILS
//================================================================================

ERROR_CODE mh_mipsolver(CPXENVptr env, CPXLPptr lp, tsp_solution *solution, double time_available)
{
    ERROR_CODE e = T_OK;

    // set the time limit
    if (CPXsetdblparam(env, CPXPARAM_TimeLimit, time_available))
    {
        log_error("error in CPXsetdblparam");
        e = INTERNAL;
        goto hf_free;
    }

    double *xstar = (double *)calloc(tsp_inst.ncols, sizeof(double));
    int ncomp = 0;
    int *comp = (int *)calloc(tsp_inst.ncols, sizeof(int));

    e = cx_branchcut_util(env, lp, tsp_inst.ncols, xstar);
    if (!err_ok(e))
    {
        log_error("error in branch&cut util");
        goto hf_free;
    }

    // with the solution found by CPLEX, build the corresponding solution
    cx_build_sol(xstar, solution);

    log_info("cost: %.2f", solution->cost);
    log_info("ncomp: %d", ncomp);

hf_free:
    utils_safe_free(xstar);
    utils_safe_free(comp);
    return e;
}

ERROR_CODE mh_mipsolver2(CPXENVptr env, CPXLPptr lp, tsp_solution *solution, double time_available)
{
    int e = T_OK;
    int cpxerror;

    double *xstar = (double *)malloc(tsp_inst.ncols * sizeof(double));

    // set the time limit
    cpxerror = CPXsetdblparam(env, CPXPARAM_TimeLimit, time_available);
    if (cpxerror)
    {
        log_error("Error in tsp_cplex: CPXsetdblparam error (%d).", cpxerror);
        e = INTERNAL;
        goto mh_free;
    }

    // solve the model using cplex
    cpxerror = CPXmipopt(env, lp);
    if (cpxerror)
    {
        log_error("Error in tsp_cplex: CPXmipopt error (%d).", cpxerror);
        e = INTERNAL;
        goto mh_free;
    }

    // get xstar
    cpxerror = CPXgetx(env, lp, xstar, 0, CPXgetnumcols(env, lp) - 1);
    if (cpxerror)
    {
        log_error("Error in tsp_cplex: CPXgetx error (%d).", cpxerror);
        e = INTERNAL;
        goto mh_free;
    }

    cx_build_sol(xstar, solution);

    log_info("solution found with %d components and cost %.0f", solution->ncomp, solution->cost);

    // apply patching to fix the solution
    while (solution->ncomp > 1)
    {
        cx_patching(solution);
        log_debug("number of components: %d", solution->ncomp);
        log_debug("current solution cost: %f", solution->cost);
        log_debug("is solution a tour? %d", tsp_is_tour(solution->path, tsp_inst.nnodes));
    }

    log_info("applied Patching, updated cost: %.0f", solution->cost);

// return status code from cplex
mh_free:
    utils_safe_free(xstar);
    return e;
}

//================================================================================
// HARD FIXING UTILS
//================================================================================

ERROR_CODE hf_fixing(CPXENVptr env, CPXLPptr lp, tsp_solution *solution)
{
    // choose E^tilde and set lb

    char *message = "executing fixing";
    log_info(message);

    ERROR_CODE e = T_OK;
    int k = 0;

    double one = 1.0;
    const char lb = 'L';
    for (int i = 0; i < tsp_inst.nnodes; i++)
    {
        // unsigned int seed = (unsigned) tsp_env.seed;
        double prob = ((double)rand()) / RAND_MAX;

        if (prob < tsp_env.hf_prob)
        {
            k++;
            log_debug("add edge (%d,%d) to E^tilde", i, solution->path[i]);
            int index = cx_xpos(i, solution->path[i], tsp_inst.nnodes);

            if (CPXchgbds(env, lp, 1, &index, &lb, &one))
            {
                log_error("error in changing the bound");
                e = INTERNAL;
                goto hf_free;
            }
        }
    }

    log_status(message);

    log_info("edges added: %d\n", k);

hf_free:
    return e;
}

ERROR_CODE hf_undofixing(CPXENVptr env, CPXLPptr lp)
{
    ERROR_CODE e = T_OK;

    double zero = 0.0;
    const char lb = 'L';
    for (int i = 0; i < tsp_inst.ncols; i++)
    {
        int pos = i;
        if (CPXchgbds(env, lp, 1, &pos, &lb, &zero))
        {
            log_error("error in changing the bound");
            e = INTERNAL;
            goto hf_free;
        }
    }

hf_free:
    return e;
}

//================================================================================
// LOCAL BRANCHING UTILS
//================================================================================

ERROR_CODE lb_add_constraint(CPXENVptr env, CPXLPptr lp, tsp_solution *solution, int k)
{
    ERROR_CODE e = T_OK;

    int *index = (int *)calloc(tsp_inst.nnodes, sizeof(int));
    double *value = (double *)calloc(tsp_inst.nnodes, sizeof(double));

    double rhs = tsp_inst.nnodes - k;
    char sense = 'G';
    char **cname = (char **)malloc(1 * sizeof(char *)); // (char **) required by cplex...

    size_t nbytes = snprintf(NULL, 0, "%s", "local branching") + 1; /* +1 for the '\0' */
    cname[0] = (char *)malloc(nbytes);
    snprintf(cname[0], nbytes, "%s", "local branching");

    int izero = 0;

    int nnz = 0;
    for (int i = 0; i < tsp_inst.nnodes; i++)
    {
        index[nnz] = cx_xpos(i, solution->path[i], tsp_inst.nnodes);
        value[nnz] = 1.0;
        nnz++;
    }

    // Integrity check
    if (nnz != tsp_inst.nnodes)
    {
        log_error("INTEGRITY CHECK: Error in tsp_convert_path_to_indval: k != nnodes (%d != %d).", nnz, tsp_inst.nnodes);
        e = DATA_LOSS;
        goto mh_free;
    }

    for (int e = 0; e < nnz; e++)
    {
        if (index[e] < 0 || index[e] >= tsp_inst.ncols || value[e] != 1.0)
        {
            log_error("INTEGRITY CHECK: Error in tsp_convert_path_to_indval: filling ind or val (%d - %f).", index[e], value[e]);
            e = INVALID_ARGUMENT;
            goto mh_free;
        }
    }

    if (CPXaddrows(env, lp, 0, 1, tsp_inst.nnodes, &rhs, &sense, &izero, index, value, NULL, &cname[0]))
    {
        log_error("error in CPXaddrows for local branching constraints");
        e = INTERNAL;
        goto mh_free;
    }

mh_free:
    utils_safe_free(cname[0]);
    utils_safe_free(cname);
    utils_safe_free(value);
    utils_safe_free(index);

    return e;
}

ERROR_CODE lb_remove_constraint(CPXENVptr env, CPXLPptr lp)
{
    ERROR_CODE e = T_OK;

    int num_rows = CPXgetnumrows(env, lp);
    if (num_rows == 0)
    {
        log_error("error in num_rows");
        e = INTERNAL;
        goto lb_free;
    }

    log_info("num_rows: %d", num_rows);

    if (CPXdelrows(env, lp, num_rows - 1, num_rows - 1))
    {
        e = INTERNAL;
        log_error("error in CPXdelrows");
        goto lb_free;
    }

lb_free:
    return e;
}

ERROR_CODE lb_kstar(CPXENVptr env, CPXLPptr lp, int *Kstar)
{
    ERROR_CODE e = T_OK;

    int *varindices = (int *)calloc(tsp_inst.ncols, sizeof(int));
    double *values = (double *)calloc(tsp_inst.ncols, sizeof(double));
    int *indices = (int *)calloc(tsp_inst.ncols, sizeof(int));
    char *xctype = (char *)calloc(tsp_inst.ncols, sizeof(char));
    double *xstar = (double *)calloc(tsp_inst.ncols, sizeof(double));

    // ESTENSIONE

    // 0) get the heuristic solution xh (must be done when the model is MIP)
    int nzcnt = tsp_inst.ncols;
    int beg;

    int effortlevel;
    int startspace = tsp_inst.ncols;
    int surplus;

    int error = CPXgetmipstarts(env, lp, &nzcnt, &beg, varindices, values, &effortlevel, tsp_inst.ncols, &surplus, 0, 0);
    if (error)
    {
        log_error("error code: %d, surplus: %d", error, surplus);
        e = INTERNAL;
        goto mh_free;
    }

    // 1) LP relaxation

    // 1.1) change variable types

    for (int i = 0; i < tsp_inst.ncols; i++)
    {
        indices[i] = i;
        xctype[i] = 'C';
    }

    if (CPXchgctype(env, lp, tsp_inst.ncols, indices, xctype))
    {
        log_error("error in chgctype");
        e = INTERNAL;
        goto mh_free;
    }

    // 1.2) change the current problem to a related problem
    if (CPXchgprobtype(env, lp, CPXPROB_LP))
    {
        log_error("error in chgprobtype");
        e = INTERNAL;
        goto mh_free;
    }

    // 2) solve lp relaxation
    if (CPXlpopt(env, lp))
    {
        log_error("error in lpopt");
        e = INTERNAL;
        goto mh_free;
    }

    // 3) get xstar
    // check that cplex solved it right
    if (CPXgetx(env, lp, xstar, 0, tsp_inst.ncols - 1))
    {
        log_fatal("CPX : CPXgetx() error");
        goto mh_free;
    }

    // 4) compute kstar
    double temp = 0.0;

    for (int i = 0; i < nzcnt; i++)
    {
        if (values[i] > 0.5)
        {
            // positions should match
            temp += (1 - xstar[i]);
        }
    }

    // 5) revert to MIP
    for (int i = 0; i < tsp_inst.ncols; i++)
    {
        indices[i] = i;
        xctype[i] = 'B';
    }

    if (CPXchgctype(env, lp, tsp_inst.ncols, indices, xctype))
    {
        log_error("error in chgctype");
        e = INTERNAL;
        goto mh_free;
    }

    if (CPXchgprobtype(env, lp, CPXPROB_MILP))
    {
        log_error("error in chgprobtype");
        e = INTERNAL;
        goto mh_free;
    }

    *Kstar = (int)(temp + 0.5);

    log_info("K-star: %d", Kstar);

mh_free:
    utils_safe_free(varindices);
    utils_safe_free(values);
    utils_safe_free(indices);
    utils_safe_free(xctype);
    utils_safe_free(xstar);

    return e;
}
