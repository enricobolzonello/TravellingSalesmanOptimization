#include "tsp.h"

instance tsp_inst;
options tsp_env;

void tsp_init(){
    // environment initialization
    tsp_env.graph_random = false;
    tsp_env.graph_input = false;
    tsp_env.timelimit = -1;
    tsp_env.seed = -1;
    tsp_env.tofile = false;
    tsp_env.k = __INT_MAX__;

    tsp_env.policy = POL_LINEAR;

    tsp_env.mileage_init = EM_MAX;

    tsp_env.bl_patching = true;

    tsp_env.init_mip = true;
    tsp_env.skip_policy = 0;
    tsp_env.callback_relaxation = true;
    tsp_env.modified_costs = false;

    tsp_env.hf_prob = 0.7;

    tsp_env.lb_dynk = false;
    tsp_env.lb_initk = 10;
    tsp_env.lb_improv = 0.02;
    tsp_env.lb_delta = 10;
    tsp_env.lb_kstar = false;


    // instance initialization
    tsp_inst.nnodes = -1;
    tsp_inst.best_solution.cost = __DBL_MAX__;
    tsp_inst.starting_node = 0;
    tsp_inst.alg = ALG_GREEDY;
    tsp_inst.cplex_terminate = 0;
    tsp_inst.ncols = -1;

    err_setverbosity(NORMAL);
}

ERROR_CODE tsp_parse_commandline(int argc, char** argv){
    if(argc < 2){
        printf("Type %s --help to see the full list of commands\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    tsp_init();

    bool help = false;
    bool algs = false;

    for(int i=1; i<argc; i++){

        if (strcmp("-f", argv[i]) == 0 || strcmp("-file", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* path = argv[++i];

            if(tsp_env.graph_random){
                log_error("ignoring input file, random graphs will be used");
                continue;
            }

            if(!utils_file_exists(path)){
                log_fatal("file does not exist");
                tsp_handlefatal();
            }

            tsp_env.inputfile = strdup(path);

            tsp_env.graph_input = true;

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
            tsp_env.timelimit = t;
            continue;
        }

        if (strcmp("-seed", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.seed = atoi(argv[++i]);
            continue;
        }

        if (strcmp("-alg", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* method = argv[++i];

            if (strcmp("GREEDY", method) == 0){
                tsp_inst.alg = ALG_GREEDY;
                log_info("selected greedy algorithm");
            }else if (strcmp("GREEDY_ITER", method) == 0){
                tsp_inst.alg = ALG_GREEDY_ITER;
                log_info("selected iterative greedy algorithm");
            }else if (strcmp("2OPT_GREEDY", method) == 0){
                tsp_inst.alg = ALG_2OPT_GREEDY;
                log_info("selected 2opt-greedy algorithm");
            }else if (strcmp("TABU_SEARCH", method) == 0){
                tsp_inst.alg = ALG_TABU_SEARCH;
                log_info("selected tabu search algorithm");
            }else if (strcmp("VNS", method) == 0){
                tsp_inst.alg = ALG_VNS;
                log_info("selected VNS algorithm");
            }else if (strcmp("CPLEX_NOSEC", method) == 0){
                tsp_inst.alg = ALG_CX_NOSEC;
                log_info("selected NOSEC");
            }else if (strcmp("CPLEX_BENDERS", method) == 0){
                tsp_inst.alg = ALG_CX_BENDERS;
                log_info("selected BENDERS LOOP");
            }else if (strcmp("EXTRA_MILEAGE", method) == 0){
                tsp_inst.alg = ALG_EXTRAMILEAGE;
                log_info("selected EXTRA MILEAGE");
            }else if (strcmp("CPLEX_BRANCH_CUT", method) == 0){
                tsp_inst.alg = ALG_CX_BRANCH_AND_CUT;
                log_info("selected CPLEX BRANCH AND CUT");
            }else if (strcmp("HARD_FIXING", method) == 0){
                tsp_inst.alg = ALG_HARD_FIXING;
                log_info("selected Hard Fixing");
            }else if (strcmp("LOCAL_BRANCHING", method) == 0){
                tsp_inst.alg = ALG_LOCAL_BRANCHING;
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
                tsp_handlefatal();
            }

            if(tsp_env.graph_input){
                log_warn("ignoring number of nodes, graph from input file will be used");
                continue;
            }

            tsp_inst.nnodes = n;

            char buffer[40];
            utils_plotname(buffer, 40);
            tsp_env.inputfile = strdup(buffer);

            tsp_env.graph_random = true;

            continue;
        }

        if(strcmp("--to_file", argv[i]) == 0){
            log_info("plots will be saved to directory /plots");

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.tofile = true;
            continue;
        }

        if(strcmp("--no_patching", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.bl_patching = false;
            continue;
        }

        if(strcmp("--init_mip", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.init_mip = true;
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

            tsp_env.k = value;
            continue;
        }

        if(strcmp("--no_relax", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.callback_relaxation = false;
            continue;
        }

        if(strcmp("--modify_costs", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.modified_costs = true;
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
            tsp_env.hf_prob = p;
            continue;
        }

        if (strcmp("--lb_dynk", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.lb_dynk = true;
            continue;
        }

        if (strcmp("-lb_initk", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const double p = atoi(argv[++i]);
            if(p < 10){
                log_warn("lb_delta must be >5");
                continue;
            }
            tsp_env.lb_initk = p;
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
            tsp_env.lb_delta = p;
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
            tsp_env.lb_improv = p;
            continue;
        }

        if(strcmp("--lb_kstar", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.lb_kstar = true;
            continue;
        }

        if(strcmp("-k", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            tsp_env.k = atoi(argv[++i]);
            continue;
        }

        if(strcmp("-em", argv[i]) == 0){

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* method = argv[++i];

            if (strcmp("MAX", method) == 0){
                tsp_env.mileage_init = EM_MAX;
                log_info("selected max initialization for em");
            }else if (strcmp("RANDOM", method) == 0){
                tsp_env.mileage_init = EM_RANDOM;
                log_info("selected random initialization for em");
            }else{
                log_warn("initialization method not recognized, using MAX as default");
                tsp_env.mileage_init = EM_MAX;
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

ERROR_CODE tsp_generate_randompoints(){
    srand(tsp_env.seed);

    tsp_inst.points = (point*) calloc(tsp_inst.nnodes, sizeof(point));

    for(int i=0; i<tsp_inst.nnodes; i++){
        tsp_inst.points[i].x = TSP_RAND();
        tsp_inst.points[i].y = TSP_RAND();
    }

    tsp_compute_costs();

    return T_OK;
}

ERROR_CODE tsp_plot_points(){
    int i;
    char* plotfile;
    plotfile = basename(tsp_env.inputfile);
    utils_format_title(plotfile, tsp_inst.alg);
    PLOT plot = plot_open(plotfile);

    if(tsp_env.tofile){
        plot_tofile(plot, plotfile);
    }

    fprintf(plot, "plot '-' with points pointtype 7\n");

    for(i=0; i<tsp_inst.nnodes; i++){
        plot_point(plot, &tsp_inst.points[i]);
    }

    plot_free(plot);

    return T_OK;
}

ERROR_CODE tsp_plot_solution(){
    char* plotfile;
    plotfile = basename(tsp_env.inputfile);
    utils_format_title(plotfile, tsp_inst.alg);

    PLOT plot = plot_open(plotfile);
    if(tsp_env.tofile){
        plot_tofile(plot, plotfile);
    }

    plot_args(plot, "plot '-' using 1:2 w lines");

    for(int i=0; i<tsp_inst.nnodes; i++){
        int v = tsp_inst.best_solution.path[i];
        plot_edge(plot, tsp_inst.points[i], tsp_inst.points[v]);
    }

    plot_free(plot);

    return T_OK;
}

void tsp_read_input(){
    FILE *input_file = fopen(tsp_env.inputfile, "r");
	if ( input_file == NULL ){
        log_fatal(" input file not found!");
        tsp_handlefatal();
    }

    tsp_inst.nnodes = -1;

    char line[300];
	char *token2, *token1, *parameter;

    int node_section = 0;

    while ( fgets(line, sizeof(line), input_file) != NULL ) {
        if ( strlen(line) <= 1 ) continue; // skip empty lines
	    parameter = strtok(line, " :");

        if ( strncmp(parameter, "DIMENSION", 9) == 0 ) {
			if ( tsp_inst.nnodes >= 0 ) {
                log_fatal("two DIMENSION parameters in the file");
                tsp_handlefatal();
            }
			token1 = strtok(NULL, " :");
			tsp_inst.nnodes = atoi(token1);	 
			tsp_inst.points = (point *) calloc(tsp_inst.nnodes, sizeof(point));
			continue;
		}

        if ( strncmp(parameter, "NODE_COORD_SECTION", 18) == 0 ) 
		{
			if ( tsp_inst.nnodes <= 0 ){
                log_fatal("DIMENSION not found");
                tsp_handlefatal();
            } 
			node_section = 1;   
			continue;
		}

        if ( strncmp(parameter, "TYPE", 4) == 0 ) 
		{
			token1 = strtok(NULL, " :");  
			if ( strncmp(token1, "TSP",3) != 0 ){
                log_fatal(" format error:  only TSP file type accepted");
                tsp_handlefatal();
            } 
			continue;
		}

        if ( strncmp(parameter, "EDGE_WEIGHT_TYPE", 16) == 0 ) 
		{
			token1 = strtok(NULL, " :");
			if ( strncmp(token1, "EUC_2D", 6) != 0 ){
                log_fatal(" format error:  only EDGE_WEIGHT_TYPE == EUC_2D managed");
                tsp_handlefatal();
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
			tsp_inst.points[i] = new_point;
			continue;
		}
    }

    ERROR_CODE error = tsp_compute_costs();
    if(!err_ok(error)){
        log_error("code error: %d", error);
    }
}

ERROR_CODE tsp_compute_costs(){
    if(tsp_inst.nnodes <= 0) {
        log_fatal("computing costs of empty graph");
        tsp_handlefatal();
    }

    tsp_inst.costs = (double *) calloc(tsp_inst.nnodes * tsp_inst.nnodes, sizeof(double));

    for (int i = 0; i < tsp_inst.nnodes; i++) {
        // Initialize each element of the matrix to -1 -> infinite cost
        for (int j = 0; j < tsp_inst.nnodes; j++) {
            tsp_inst.costs[i* tsp_inst.nnodes + j] = -1.0f;
        }
    }

    // computation of costs of edges with euclidean distance
    for (int i = 0; i < tsp_inst.nnodes; i++) {
        for (int j = 0; j < tsp_inst.nnodes; j++) {
            if (j == i){
                continue;
            }
            double distance = (double) ((int) (sqrtf(pow(tsp_inst.points[j].x - tsp_inst.points[i].x, 2) + pow(tsp_inst.points[j].y - tsp_inst.points[i].y, 2)) + 0.5));
            tsp_inst.costs[i* tsp_inst.nnodes + j] = distance;
            tsp_inst.costs[j* tsp_inst.nnodes + i] = distance;
        }
    }

    return T_OK;
}

double tsp_get_cost(int i, int j){
    return tsp_inst.costs[i * tsp_inst.nnodes + j];
}

bool tsp_validate_solution(int nnodes, int* current_solution_path) {
    int* node_visit_counter = (int*)calloc(nnodes, sizeof(int));

    // count how many times each node is visited
    for(int i=0; i<nnodes; i++){
        int node = current_solution_path[i];
        if(node < 0 || node > nnodes - 1){
            // node index outside range
            utils_safe_free(node_visit_counter);
            return false;
        }
        node_visit_counter[node] ++;
    }

    // check that each node is visited once
    for(int i=0; i<nnodes; i++){
        if(node_visit_counter[i] != 1){
            // at least one node visted zero or more than one time
            utils_safe_free(node_visit_counter);
            return false;
        }
    }

    utils_safe_free(node_visit_counter);
    return tsp_is_tour(current_solution_path, nnodes);
}

ERROR_CODE tsp_update_best_solution(tsp_solution* current_solution){
    if(tsp_validate_solution(tsp_inst.nnodes, current_solution->path)){
        if(current_solution->cost < tsp_inst.best_solution.cost){
            memcpy(tsp_inst.best_solution.path, current_solution->path, tsp_inst.nnodes * sizeof(int));
            tsp_inst.best_solution.cost = current_solution->cost;
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


bool tsp_is_tour(int path[], int n) {
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

double tsp_solution_cost(int path[]){
    double cost = 0;
    for(int i =0; i< tsp_inst.nnodes; i++){
        cost += tsp_get_cost(i, path[i]);
    }
    return cost;
}

void tsp_handlefatal(){
    log_warn("fatal error detected, shutting down application");
    tsp_free_instance();
    exit(EXIT_FAILURE);
}

void tsp_free_instance(){
    utils_safe_free(tsp_env.inputfile);
    utils_safe_free(tsp_inst.points);
    utils_safe_free(tsp_inst.costs);
    utils_safe_free(tsp_inst.best_solution.path);
    utils_safe_free(tsp_inst.best_solution.comp);
    utils_safe_free(tsp_inst.threads_seeds);
}
