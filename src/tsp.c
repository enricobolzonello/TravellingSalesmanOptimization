#include "tsp.h"

ERROR_CODE tsp_parse_commandline(int argc, char** argv, instance* inst){
    if(argc < 2){
        printf("Type %s --help to see the full list of commands\n", argv[0]);
        exit(1);
    }

    inst->options_t.graph_random = false;
    inst->options_t.graph_input = false;
    inst->options_t.timelimit = -1;
    inst->options_t.seed = 0;
    inst->nnodes = -1;
    inst->costs_computed = false;
    inst->best_solution_cost = __DBL_MAX__;
    inst->solution_cost = __DBL_MAX__;
    err_setverbosity(NORMAL);
    bool help = false;
    bool algs = false;

    inst->algorithm = calloc(12, sizeof(char));
    strcpy(inst->algorithm, "GREEDY-ONCE");


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

        // TODO: add flags for algorithm chosen
        if (strcmp("-alg", argv[i]) == 0){
            log_info("parsing algorithm argument");

            if(utils_invalid_input(i, argc, &help)){
                log_warn("invalid input");
                continue;
            }

            const char* method = argv[++i];

            free(inst->algorithm);
            if (strncmp("GREEDY-ONCE", method, 11) == 0){
                inst->algorithm = calloc(12, sizeof(char));
                strcpy(inst->algorithm, "GREEDY-ONCE");
                printf("GREEDY\n");
            }else if (strncmp("GREEDY-ALLNODES", method, 15) == 0){
                inst->algorithm = calloc(16, sizeof(char));
                strcpy(inst->algorithm, "GREEDY-ALLNODES");
                printf("GREEDY-ALLNODES\n");
            }
            else if (strncmp("2OPT-GREEDY", method, 11) == 0){
                inst->algorithm = calloc(12, sizeof(char));
                strcpy(inst->algorithm, "2OPT-GREEDY");
                printf("2OPT-GREEDY\n");
            }else{
                utils_print_error("INVALID ALGORITHM use -h or --all_algs for help");
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
        printf("    -q                      quiet verbosity level, prints only output");
        printf("    -v                      verbose verbosity level, prints info, warnings, errors or fatal errors");
        printf("    -vv                      verbose verbosity level, prints also debug and trace");
    }

    if(algs){
        printf(COLOR_BOLD "Available algorithms:\n" COLOR_OFF);
        printf("    - GREEDY-ONCE\n");
        printf("    - GREEDY-ALLNODES\n");
        printf("    - 2OPT-GREEDY\n");
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

ERROR_CODE tsp_plot_points(instance* inst, char* name, bool to_file){
    int i;
    PLOT plot = plot_open(name);

    if(to_file){
        plot_tofile(plot, name);
    }

    fprintf(plot, "plot '-' with points pointtype 7\n");

    for(i=0; i<inst->nnodes; i++){
        plot_point(plot, &inst->points[i]);
    }

    plot_free(plot);

    return OK;
}

ERROR_CODE tsp_plot_solution(instance* inst, char* name,bool to_file){
    int i;
    PLOT plot = plot_open(name);
    if(to_file){
        plot_tofile(plot, name);
    }

    plot_args(plot, "plot '-' using 1:2 w lines");

    int v;
    for(i=0; i<inst->nnodes; i++){
        v = inst->solution_path[i];
        plot_edge(plot, inst->points[i], inst->points[v]);
    }

    plot_free(plot);

    return OK;
}

void tsp_handlefatal(instance *inst){
    log_info("shutting down application");
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
	if ( input_file == NULL ) utils_print_error(" input file not found!");

    inst->nnodes = -1;

    char line[300];
    char* parameter;
    char* token1;
	char* token2;

    int node_section = 0;

    while ( fgets(line, sizeof(line), input_file) != NULL ) {
        if ( strlen(line) <= 1 ) continue; // skip empty lines
	    parameter = strtok(line, " :");

        if ( strncmp(parameter, "DIMENSION", 9) == 0 ) {
			if ( inst->nnodes >= 0 ) utils_print_error("two DIMENSION parameters in the file");
			token1 = strtok(NULL, " :");
			inst->nnodes = atoi(token1);	 
			inst->points = (point *) calloc(inst->nnodes, sizeof(point));
            inst->points_allocated = true;
			continue;
		}

        if ( strncmp(parameter, "NODE_COORD_SECTION", 18) == 0 ) 
		{
			if ( inst->nnodes <= 0 ) utils_print_error("DIMENSION not found");
			node_section = 1;   
			continue;
		}

        if ( strncmp(parameter, "TYPE", 4) == 0 ) 
		{
			token1 = strtok(NULL, " :");  
			if ( strncmp(token1, "TSP",3) != 0 ) utils_print_error(" format error:  only TSP file type accepted");
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

void tsp_compute_costs(instance* inst){
    if(inst->nnodes <= 0) utils_print_error("computing costs of empty graph");

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
        for (int j = i + 1; j < inst->nnodes; j++) {
            float distance = sqrtf(pow(inst->points[j].x - inst->points[i].x, 2) + pow(inst->points[j].y - inst->points[i].y, 2));
            inst->costs[i][j] = distance;
            inst->costs[j][i] = distance;
        }
    }

    inst->costs_computed = false;
}

bool tsp_validate_solution(instance* inst) {
    int* node_visit_counter = (int*)calloc(inst->nnodes, sizeof(int));

    // count how many times each node is visited
    for(int i=0; i<inst->nnodes; i++){
        int node = inst->solution_path[i];
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

void tsp_update_best_solution(instance* inst){
    if(tsp_validate_solution(inst)){
        if(inst->solution_cost < inst->best_solution_cost){
            inst->best_solution_path = inst->solution_path;
            inst->best_solution_cost = inst->solution_cost;
        }
    }else{
        utils_print_error("You tried to update best_solution with an unvalid solution");
    }   
}
