#include "tsp.h"

void tsp_init(instance* inst){
    inst->options_t.graph_random = false;
    inst->options_t.graph_input = false;
    inst->options_t.timelimit = -1;
    inst->options_t.seed = 0;
    inst->options_t.tofile = false;
    inst->nnodes = -1;
    inst->costs_computed = false;
    inst->best_solution_cost = __DBL_MAX__;
    err_setverbosity(NORMAL);
    inst->alg = ALG_GREEDY;
    inst->c = utils_startclock();
    inst->starting_node = 0;
}

ERROR_CODE tsp_parse_commandline(int argc, char** argv, instance* inst){
    if(argc < 2){
        printf("Type %s --help to see the full list of commands\n", argv[0]);
        exit(1);
    }

    tsp_init(inst);

    bool help = false;
    bool algs = false;

    for(int i=1; i<argc; i++){

        if (strcmp("-f", argv[i]) == 0 || strcmp("-file", argv[i]) == 0){
            log_info("parsing input file argument");

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* path = argv[++i];

            if(inst->options_t.graph_random){
                log_error("you can't have both random generation and input file");
                log_info("ignoring input file, random graphs will be used");
                continue;
            }

            if(!utils_file_exists(path)){
                log_fatal("file does not exist");
                tsp_handlefatal(inst);
            }

            inst->options_t.inputfile = (char*) calloc(strlen(path), sizeof(char));
            strcpy(inst->options_t.inputfile, path);

            inst->options_t.graph_input = true;

            continue;
        }

        if (strcmp("-t", argv[i]) == 0 || strcmp("-time", argv[i]) == 0){
            log_info("parsing time limit argument");

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const double t = atof(argv[++i]);
            if(t<0){
                log_warn("time cannot be negative");
                log_info("ignoring time limit");
                continue;
            }
            inst->options_t.timelimit = t;
            continue;
        }

        if (strcmp("-seed", argv[i]) == 0){
            log_info("parsing seed argument");

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            inst->options_t.seed = atoi(argv[++i]);
            continue;
        }

        if (strcmp("-alg", argv[i]) == 0){
            log_info("parsing algorithm argument");

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* method = argv[++i];

            if (strcmp("GREEDY", method) == 0){
                inst->alg = ALG_GREEDY;
                log_info("selected greedy algorithm");
            }else if (strcmp("GREEDY_ITERATIVE", method) == 0){
                inst->alg = ALG_GREEDY_ITER;
                log_info("selected iterative greedy algorithm");
            }
            else if (strcmp("2OPT_GREEDY", method) == 0){
                inst->alg = ALG_2OPT_GREEDY;
                log_info("selected 2opt-greedy algorithm");
            }else if (strcmp("TABU_SEARCH", method) == 0){
                inst->alg = ALG_TABU_SEARCH;
                log_info("selected tabu search algorithm");
            }else{
                log_warn("algorithm not recognized, using greedy as default");
            }

            continue;
        }

        if (strcmp("-n", argv[i]) == 0){
            log_info("parsing number of nodes argument");

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
                log_error("you can't have both random generation and input file");
                log_info("ignoring number of nodes, graph from input file will be used");
                continue;
            }

            inst->nnodes = n;

            inst->options_t.graph_random = true;

            char buffer[40];
            utils_plotname(buffer, 40);
            inst->options_t.inputfile = (char*) calloc(strlen(buffer), sizeof(char));
            strcpy(inst->options_t.inputfile, buffer);

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
        printf(COLOR_BOLD "Usage:\n" COLOR_OFF);
        printf("tsp [--help, -help, -h] [-file, -f <path>] [-time, -t <value>] \n");
        printf("    [-seed <value>] [-alg <option>] [-n <value>]\n\n");
        printf(COLOR_BOLD "Options:\n" COLOR_OFF);
        printf("    --help, -help, -h       prints this text\n");
        printf("    -file, -f <path>        input a TSPLIB file format\n");
        printf("    -time, -t <value>       execution time limit in seconds\n");
        printf("    -seed <value>           seed for random generation, if not set defaults to user time\n");
        printf("    -alg <option>           selects the algorithm to solve TSP, run --all_algs to see the options\n");
        printf("    -n <value>              number of nodes\n");
        printf("    --all_algs              prints all possible algorithms\n");
        printf("    --to_file               if present, plots will be saved in directory /plots\n");
        printf("    -q                      quiet verbosity level, prints only output\n");
        printf("    -v                      verbose verbosity level, prints info, warnings, errors or fatal errors\n");
        printf("    -vv                     verbose verbosity level, prints also debug and trace\n");

        return ABORTED;
    }

    if(algs){
        printf(COLOR_BOLD "Available algorithms:\n" COLOR_OFF);
        printf("    - GREEDY\n");
        printf("    - GREEDY_ITERATIVE\n");
        printf("    - 2OPT_GREEDY\n");
        printf("    - TABU_SEARCH");
        
        return ABORTED;
    }

    return OK;
}

ERROR_CODE tsp_generate_randompoints(instance* inst){
    srand(inst->options_t.seed);

    inst->points = (point*) calloc(inst->nnodes, sizeof(point));

    for(int i=0; i<inst->nnodes; i++){
        inst->points[i].x = TSP_RAND();
        inst->points[i].y = TSP_RAND();
    }

    tsp_compute_costs(inst);

    return OK;
}

ERROR_CODE tsp_plot_points(instance* inst){
    int i;
    char* plotfile;
    plotfile = basename(inst->options_t.inputfile);
    utils_format_title(plotfile);
    PLOT plot = plot_open(plotfile);

    if(inst->options_t.tofile){
        plot_tofile(plot, plotfile);
    }

    fprintf(plot, "plot '-' with points pointtype 7\n");

    for(i=0; i<inst->nnodes; i++){
        plot_point(plot, &inst->points[i]);
    }

    plot_free(plot);

    return OK;
}

ERROR_CODE tsp_plot_solution(instance* inst){
    char* plotfile;
    plotfile = basename(inst->options_t.inputfile);
    utils_format_title(plotfile);

    PLOT plot = plot_open(plotfile);
    if(inst->options_t.tofile){
        plot_tofile(plot, plotfile);
    }

    plot_args(plot, "plot '-' using 1:2 w lines");

    for(int i=0; i<inst->nnodes; i++){
        int v = inst->best_solution_path[i];
        plot_edge(plot, inst->points[i], inst->points[v]);
    }

    plot_free(plot);

    return OK;
}

void tsp_handlefatal(instance *inst){
    log_info("fatal error detected, shutting down application");
    tsp_free_instance(inst);
    exit(0);
}

void tsp_free_instance(instance *inst){
    if(inst->options_t.graph_input){
        free(inst->options_t.inputfile);
    }
    if(inst->points_allocated){
        free(inst->points);
    }

    if(inst->costs_computed){
        for(int i = 0; i < inst->nnodes; i++){
            free(inst->costs[i]);
        }
        free(inst->costs);
    }
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
            inst->points_allocated = true;
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

    tsp_compute_costs(inst);
}

ERROR_CODE tsp_compute_costs(instance* inst){
    log_debug("computing costs");

    if(inst->nnodes <= 0) log_fatal("computing costs of empty graph");

    inst->costs = (double **) calloc(inst->nnodes, sizeof(double**));

    for (int i = 0; i < inst->nnodes; i++) {
        inst->costs[i] = calloc(inst->nnodes, sizeof(double));
        // Initialize each element of the matrix to -1 -> infinite cost
        for (int j = 0; j < inst->nnodes; j++) {
            inst->costs[i][j] = -1.0f;
        }
    }

    // computation of costs of edges with euclidean distance
    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = 0; j < inst->nnodes; j++) {

            // check that we have not exceed time limit
            double ex_time = utils_timeelapsed(inst->c);
            if(inst->options_t.timelimit != -1.0){
                if(ex_time > inst->options_t.timelimit){
                    return DEADLINE_EXCEEDED;
                }
            }

            if (j == i){
                continue;
            }
            float distance = sqrtf(pow(inst->points[j].x - inst->points[i].x, 2) + pow(inst->points[j].y - inst->points[i].y, 2));
            inst->costs[i][j] = distance;
            inst->costs[j][i] = distance;
        }
    }

    inst->costs_computed = true;

    return OK;
}

bool tsp_validate_solution(instance* inst, int* current_solution_path) {
    int* node_visit_counter = (int*)calloc(inst->nnodes, sizeof(int));

    // count how many times each node is visited
    for(int i=0; i<inst->nnodes; i++){
        int node = current_solution_path[i];
        node_visit_counter[node] ++;
    }

    // check that each node is visited once
    for(int i=0; i<inst->nnodes; i++){
        if(node_visit_counter[i] != 1){
            free(node_visit_counter);
            return false;
        }
    }

    free(node_visit_counter);
    return true;
}

void tsp_update_best_solution(instance* inst, double current_solution_cost, int* current_solution_path){
    if(tsp_validate_solution(inst, current_solution_path)){
        if(current_solution_cost < inst->best_solution_cost){
            memcpy(inst->best_solution_path, current_solution_path, inst->nnodes * sizeof(int)); // here's the problem
            inst->best_solution_cost = current_solution_cost;
        }
    }else{
        log_debug("You tried to update best_solution with an unvalid solution");
    }   
}
