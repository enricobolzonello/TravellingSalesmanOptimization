#include "tsp.h"

void tsp_init(instance* inst){
    inst->options_t.graph_random = false;
    inst->options_t.graph_input = false;
    inst->options_t.timelimit = -1;
    inst->options_t.seed = -1;
    inst->options_t.tofile = false;
    inst->options_t.k = __INT_MAX__;

    inst->options_t.policy = POL_LINEAR;

    inst->options_t.mileage_init = EM_MAX;

    inst->options_t.init_mip = true;
    inst->options_t.skip_policy = 0;
    inst->options_t.callback_relaxation = true;
    inst->options_t.modified_costs = false;

    inst->options_t.hf_prob = 0.7;

    inst->options_t.lb_improv = 0.02;
    inst->options_t.lb_delta = 10;
    inst->options_t.lb_kstar = false;
    
    inst->nnodes = -1;
    inst->best_solution.cost = __DBL_MAX__;
    inst->starting_node = 0;
    inst->alg = ALG_GREEDY;

    err_setverbosity(NORMAL);

    inst->ncols = -1;
}

tsp_solution tsp_init_solution(int nnodes){
    tsp_solution solution;
    solution.path = calloc(nnodes, sizeof(int));
    solution.cost = __DBL_MAX__;
    return solution;
}

ERROR_CODE tsp_parse_commandline(int argc, char** argv, instance* inst){
    if(argc < 2){
        printf("Type %s --help to see the full list of commands\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    tsp_init(inst);

    bool help = false;
    bool algs = false;

    for(int i=1; i<argc; i++){

        if (strcmp("-f", argv[i]) == 0 || strcmp("-file", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* path = argv[++i];

            if(inst->options_t.graph_random){
                log_error("ignoring input file, random graphs will be used");
                continue;
            }

            if(!utils_file_exists(path)){
                log_fatal("file does not exist");
                tsp_handlefatal(inst);
            }

            inst->options_t.inputfile = strdup(path);

            inst->options_t.graph_input = true;

            continue;
        }

        if (strcmp("-t", argv[i]) == 0 || strcmp("-time", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const double t = atof(argv[++i]);
            if(t<0){
                log_warn("time cannot be negative, ignoring time limit");
                continue;
            }
            inst->options_t.timelimit = t;
            continue;
        }

        if (strcmp("-seed", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            inst->options_t.seed = atoi(argv[++i]);
            continue;
        }

        if (strcmp("-alg", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* method = argv[++i];

            if (strcmp("GREEDY", method) == 0){
                inst->alg = ALG_GREEDY;
                log_info("selected greedy algorithm");
            }else if (strcmp("GREEDY_ITER", method) == 0){
                inst->alg = ALG_GREEDY_ITER;
                log_info("selected iterative greedy algorithm");
            }else if (strcmp("2OPT_GREEDY", method) == 0){
                inst->alg = ALG_2OPT_GREEDY;
                log_info("selected 2opt-greedy algorithm");
            }else if (strcmp("TABU_SEARCH", method) == 0){
                inst->alg = ALG_TABU_SEARCH;
                log_info("selected tabu search algorithm");
            }else if (strcmp("VNS", method) == 0){
                inst->alg = ALG_VNS;
                log_info("selected VNS algorithm");
            }else if (strcmp("CPLEX_NOSEC", method) == 0){
                inst->alg = ALG_CX_NOSEC;
                log_info("selected NOSEC");
            }else if (strcmp("CPLEX_BENDERS", method) == 0){
                inst->alg = ALG_CX_BENDERS;
                log_info("selected BENDERS LOOP");
            }else if (strcmp("EXTRA_MILEAGE", method) == 0){
                inst->alg = ALG_EXTRAMILEAGE;
                log_info("selected EXTRA MILEAGE");
            }else if (strcmp("CPLEX_BENDERS_PAT", method) == 0){
                inst->alg = ALG_CX_BENDERS_PAT;
                log_info("selected BENDERS LOOP with PATCHING");
            }else if (strcmp("CPLEX_BRANCH_CUT", method) == 0){
                inst->alg = ALG_CX_BRANCH_AND_CUT;
                log_info("selected CPLEX BRANCH AND CUT");
            }else if (strcmp("HARD_FIXING", method) == 0){
                inst->alg = ALG_HARD_FIXING;
                log_info("selected Hard Fixing");
            }else if (strcmp("LOCAL_BRANCHING", method) == 0){
                inst->alg = ALG_LOCAL_BRANCHING;
                log_info("selected Hard Fixing");
            }else{
                log_warn("algorithm not recognized, using greedy as default");
            }

            continue;
        }

        if (strcmp("-n", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            int n = atoi(argv[++i]);
            if(n <= 0){
                log_fatal("number of nodes should be greater than 0");
                tsp_handlefatal(inst);
            }

            if(inst->options_t.graph_input){
                log_warn("ignoring number of nodes, graph from input file will be used");
                continue;
            }

            inst->nnodes = n;

            char buffer[40];
            utils_plotname(buffer, 40);
            inst->options_t.inputfile = strdup(buffer);

            inst->options_t.graph_random = true;

            continue;
        }

        if(strcmp("--to_file", argv[i]) == 0){
            log_info("plots will be saved to directory /plots");

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            inst->options_t.tofile = true;
            continue;
        }

        if(strcmp("--init_mip", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            inst->options_t.init_mip = true;
            continue;
        }

        if(strcmp("-skip", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            int value = atoi(argv[++i]);
            if(value < 0 || value > 2){
                log_info("supported options are 0 (thread seeds), 1 (number of nodes), 2 (depth>3)");
                continue;
            }

            inst->options_t.k = value;
            continue;
        }

        if(strcmp("--no_relax", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            inst->options_t.callback_relaxation = false;
            continue;
        }

        if(strcmp("--modify_costs", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            inst->options_t.modified_costs = true;
            continue;
        }

        if (strcmp("-hf_prob", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const double p = atof(argv[++i]);
            if(p <= 0.0 || p > 1.0){
                log_warn("hf_prob must be (0,1]");
                continue;
            }
            inst->options_t.hf_prob = p;
            continue;
        }

        if (strcmp("-lb_delta", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const double p = atoi(argv[++i]);
            if(p < 5){
                log_warn("lb_delta must be >5");
                continue;
            }
            inst->options_t.lb_delta = p;
            continue;
        }

        if (strcmp("-lb_improv", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const double p = atof(argv[++i]);
            if(p <= 0.0 || p > 1.0){
                log_warn("lb_improv must be (0,1]");
                continue;
            }
            inst->options_t.lb_improv = p;
            continue;
        }

        if(strcmp("--lb_kstar", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            inst->options_t.lb_kstar = true;
            continue;
        }

        if(strcmp("-k", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            inst->options_t.k = atoi(argv[++i]);
            continue;
        }

        if(strcmp("-em", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* method = argv[++i];

            if (strcmp("MAX", method) == 0){
                inst->options_t.mileage_init = EM_MAX;
                log_info("selected max initialization for em");
            }else if (strcmp("RANDOM", method) == 0){
                inst->options_t.mileage_init = EM_RANDOM;
                log_info("selected random initialization for em");
            }else{
                log_warn("initialization method not recognized, using MAX as default");
                inst->options_t.mileage_init = EM_MAX;
            }

            continue;
        }

        if(strcmp("-q", argv[i]) == 0){
            err_setverbosity(QUIET);
            continue;
        }

        if(strcmp("-v", argv[i]) == 0){
            err_setverbosity(VERBOSE);
            continue;
        }

        if(strcmp("-vv", argv[i]) == 0){
            err_setverbosity(VERY_VERBOSE);
            continue;
        }

        if(strcmp("-h", argv[i]) == 0 || strcmp("-help", argv[i]) == 0 || strcmp("--help", argv[i]) == 0){
            help = true;
            continue;
        }

        if(strcmp("--all_algs", argv[i]) == 0){
            algs = true;
            continue;
        }

        help = true;
    }

    if(help){
        printf("tsp - Traveling Salesman Solver\n\n");
        printf(COLOR_BOLD "USAGE:\n" COLOR_OFF);
        printf("tsp [--help, -help, -h] [--all_algs] [-file, -f <path>] [-time, -t <value>] [-seed <value>] [-alg <option>] [-n <value>] [--to_file]\n");
        printf("    [-k <value>] [-em <option>] [--init_mip] [-skip <option>] [--no_relax] [-q, (DEFAULT), -v, -vv] \n\n");
        printf(COLOR_BOLD "OPTIONS:\n" COLOR_OFF);
        printf("    --help, -help, -h       prints this text\n");
        printf("    -file, -f <path>        input a TSPLIB file format\n");
        printf("    -time, -t <value>       execution time limit in seconds\n");
        printf("    -seed <value>           seed for random generation, if not set defaults to user time\n");
        printf("    -alg <option>           selects the algorithm to solve TSP, run --all_algs to see the options\n");
        printf("    -n <value>              number of nodes\n");
        printf("    -k <value>              number of iterations of some tabu search and vns, defaults to the maximum value possible\n");
        printf("    -em <option>            initialization for Extra Mileage, options: MAX, RANDOM. Defaults to MAX\n");
        printf("    --all_algs              prints all possible algorithms\n");
        printf("    --to_file               if present, plots will be saved in directory /plots\n");
        printf(COLOR_BOLD "  Branch&Cut\n" COLOR_OFF);
        printf("    --init_mip              use a custom heuristic to be set as MIP start\n");
        printf("    -skip                   skip policy for branch&cut. Either 0 (thread seeds), 1 (number of cplex nodes), 2 (if depth>3)\n");
        printf("    --no_relax              turn off CPLEX relaxation callback function\n");
        printf("    --modify_costs          in the relaxation callback, post to CPLEX an heuristic solution with modified costs\n");
        printf(COLOR_BOLD "  Hard Fixing\n" COLOR_OFF);
        printf("    -hf_prob <value>        probability of setting an edge. Must be in range [0,1)\n");
        printf(COLOR_BOLD "  Local Branching\n" COLOR_OFF);
        printf("    -lb_improv <value>      improvement w.r.t. last iteration objective value needed to increase K. Must be in range (0,1)\n");
        printf("    -lb_delta <value>       corresponds to %lcK, represents the amount by which K is changed\n", 0x0394);
        printf("    --lb_kstar              flag to turn on dynamic K\n");
        printf(COLOR_BOLD "  Verbosity\n" COLOR_OFF);
        printf("    -q                      quiet verbosity level, prints only output\n");
        printf("    DEFAULT                 if no flag is set, prints warnings, erros or fatal errors\n");
        printf("    -v                      verbose verbosity level, prints info, warnings, errors or fatal errors\n");
        printf("    -vv                     verbose verbosity level, prints also debug and trace\n");

        exit(EXIT_SUCCESS);
    }

    if(algs){
        printf(COLOR_BOLD "Available algorithms:\n" COLOR_OFF);
        printf("    - GREEDY\n");
        printf("    - GREEDY_ITER\n");
        printf("    - 2OPT_GREEDY\n");
        printf("    - TABU_SEARCH\n");
        printf("    - VNS\n");
        printf("    - CPLEX_NOSEC\n");
        printf("    - CPLEX_BENDERS\n");
        printf("    - CPLEX_BENDERS_PAT\n");
        printf("    - EXTRA_MILEAGE\n");
        printf("    - CPLEX_BRANCH_CUT\n");
        printf("    - HARD_FIXING\n");
        printf("    - LOCAL_BRANCHING\n");
        
        exit(EXIT_SUCCESS);
    }

    return T_OK;
}

ERROR_CODE tsp_generate_randompoints(instance* inst){
    srand(inst->options_t.seed);

    inst->points = (point*) calloc(inst->nnodes, sizeof(point));

    for(int i=0; i<inst->nnodes; i++){
        inst->points[i].x = TSP_RAND();
        inst->points[i].y = TSP_RAND();
    }

    tsp_compute_costs(inst);

    return T_OK;
}

// TODO: better naming of files
ERROR_CODE tsp_plot_points(instance* inst){
    int i;
    char* plotfile;
    plotfile = basename(inst->options_t.inputfile);
    utils_format_title(plotfile, inst->alg);
    PLOT plot = plot_open(plotfile);

    if(inst->options_t.tofile){
        plot_tofile(plot, plotfile);
    }

    fprintf(plot, "plot '-' with points pointtype 7\n");

    for(i=0; i<inst->nnodes; i++){
        plot_point(plot, &inst->points[i]);
    }

    plot_free(plot);

    return T_OK;
}

ERROR_CODE tsp_plot_solution(instance* inst){
    char* plotfile;
    plotfile = basename(inst->options_t.inputfile);
    utils_format_title(plotfile, inst->alg);

    PLOT plot = plot_open(plotfile);
    if(inst->options_t.tofile){
        plot_tofile(plot, plotfile);
    }

    plot_args(plot, "plot '-' using 1:2 w lines");

    for(int i=0; i<inst->nnodes; i++){
        int v = inst->best_solution.path[i];
        plot_edge(plot, inst->points[i], inst->points[v]);
    }

    plot_free(plot);

    return T_OK;
}

void tsp_read_input(instance* inst){
    FILE *input_file = fopen(inst->options_t.inputfile, "r");
	if ( input_file == NULL ){
        log_fatal(" input file not found!");
        tsp_handlefatal(inst);
    }

    inst->nnodes = -1;

    char line[300];
	char *token2, *token1, *parameter;

    int node_section = 0;

    while ( fgets(line, sizeof(line), input_file) != NULL ) {
        if ( strlen(line) <= 1 ) continue; // skip empty lines
	    parameter = strtok(line, " :");

        if ( strncmp(parameter, "DIMENSION", 9) == 0 ) {
			if ( inst->nnodes >= 0 ) {
                log_fatal("two DIMENSION parameters in the file");
                tsp_handlefatal(inst);
            }
			token1 = strtok(NULL, " :");
			inst->nnodes = atoi(token1);	 
			inst->points = (point *) calloc(inst->nnodes, sizeof(point));
			continue;
		}

        if ( strncmp(parameter, "NODE_COORD_SECTION", 18) == 0 ) 
		{
			if ( inst->nnodes <= 0 ){
                log_fatal("DIMENSION not found");
                tsp_handlefatal(inst);
            } 
			node_section = 1;   
			continue;
		}

        if ( strncmp(parameter, "TYPE", 4) == 0 ) 
		{
			token1 = strtok(NULL, " :");  
			if ( strncmp(token1, "TSP",3) != 0 ){
                log_fatal(" format error:  only TSP file type accepted");
                tsp_handlefatal(inst);
            } 
			continue;
		}

        if ( strncmp(parameter, "EDGE_WEIGHT_TYPE", 16) == 0 ) 
		{
			token1 = strtok(NULL, " :");
			if ( strncmp(token1, "EUC_2D", 6) != 0 ){
                log_fatal(" format error:  only EDGE_WEIGHT_TYPE == EUC_2D managed");
                tsp_handlefatal(inst);
            }
			continue;
		}

        if ( strncmp(parameter, "EOF", 3) == 0 ) {
			break;
		}

        if (node_section) {
			int i = atoi(parameter) - 1; //index 
			token1 = strtok(NULL, " :,");
			token2 = strtok(NULL, " :,");
            point new_point;
            new_point.x = atof(token1);
            new_point.y = atof(token2);
			inst->points[i] = new_point;
			continue;
		}
    }

    ERROR_CODE error = tsp_compute_costs(inst);
    if(!err_ok(error)){
        log_error("code error: %d", error);
    }
}

ERROR_CODE tsp_compute_costs(instance* inst){
    if(inst->nnodes <= 0) {
        log_fatal("computing costs of empty graph");
        tsp_handlefatal(inst);
    }

    inst->costs = (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));

    for (int i = 0; i < inst->nnodes; i++) {
        // Initialize each element of the matrix to -1 -> infinite cost
        for (int j = 0; j < inst->nnodes; j++) {
            inst->costs[i* inst->nnodes + j] = -1.0f;
        }
    }

    // computation of costs of edges with euclidean distance
    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = 0; j < inst->nnodes; j++) {

            // check that we have not exceed time limit
            double ex_time = utils_timeelapsed(&inst->c);
            if(inst->options_t.timelimit != -1.0){
                if(ex_time > inst->options_t.timelimit){
                    return DEADLINE_EXCEEDED;
                }
            }

            if (j == i){
                continue;
            }
            double distance = (double) ((int) (sqrtf(pow(inst->points[j].x - inst->points[i].x, 2) + pow(inst->points[j].y - inst->points[i].y, 2)) + 0.5));
            inst->costs[i* inst->nnodes + j] = distance;
            inst->costs[j* inst->nnodes + i] = distance;
        }
    }

    return T_OK;
}

double tsp_get_cost(instance* inst, int i, int j){
    return inst->costs[i * inst->nnodes + j];
}

bool tsp_validate_solution(instance* inst, int* current_solution_path) {
    int* node_visit_counter = (int*)calloc(inst->nnodes, sizeof(int));

    // count how many times each node is visited
    for(int i=0; i<inst->nnodes; i++){
        int node = current_solution_path[i];
        if(node < 0 || node > inst->nnodes - 1){
            // node index outside range
            utils_safe_free(node_visit_counter);
            return false;
        }
        node_visit_counter[node] ++;
    }

    // check that each node is visited once
    for(int i=0; i<inst->nnodes; i++){
        if(node_visit_counter[i] != 1){
            // at least one node visted zero or more than one time
            utils_safe_free(node_visit_counter);
            return false;
        }
    }

    utils_safe_free(node_visit_counter);
    return isTour(current_solution_path, inst->nnodes);
}

ERROR_CODE tsp_update_best_solution(instance* inst, tsp_solution* current_solution){
    if(tsp_validate_solution(inst, current_solution->path)){
        if(current_solution->cost < inst->best_solution.cost){
            memcpy(inst->best_solution.path, current_solution->path, inst->nnodes * sizeof(int));
            inst->best_solution.cost = current_solution->cost;
            log_info("new best solution: %f", current_solution->cost);
            return T_OK;
        }

        log_debug("discarded cost: %.2f", current_solution->cost);
        return CANCELLED;
    }else{
        log_error("You tried to update best_solution with an unvalid solution");
        return INVALID_ARGUMENT;
    }   
}


bool isTour(int path[], int n) {
    // Check if the path has at least one node
    if (n == 0)
        return false;

    // Array to keep track of visited nodes
    bool visited[n]; // Declare the array without initialization

    // Initialize the visited array
    for (int i = 0; i < n; i++)
        visited[i] = false;

    // Mark the starting node as visited

    // Check if each node is visited exactly once
    int i = 0;
    visited[0] = true;

    bool end_tour = false;
    while (!end_tour) {
        // Check if the node is within the valid range
        if (path[i] < 0 || path[i] >= n)
            return false;

        // Check if the node appears more than once
        if (visited[path[i]])
            end_tour = true;

        // Mark the node as visited
        visited[path[i]] = true;

        i = path[i];
    }

    // Check if all nodes have been visited
    for (int i = 0; i < n; i++) {
        if (!visited[i])
            return false;
    }

    return true;
}

double solutionCost(instance *inst, int path[]){
    double cost = 0;
    for(int i =0; i< inst->nnodes; i++){
        cost += tsp_get_cost(inst, i, path[i]);
    }
    return cost;
}

void tsp_handlefatal(instance *inst){
    log_warn("fatal error detected, shutting down application");
    tsp_free_instance(inst);
    exit(EXIT_FAILURE);
}

void tsp_free_instance(instance *inst){
    utils_safe_free(inst->options_t.inputfile);
    utils_safe_free(inst->points);
    utils_safe_free(inst->costs);
    utils_safe_free(inst->best_solution.path);
}
