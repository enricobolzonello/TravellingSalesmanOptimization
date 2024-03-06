#include "tsp.h"

void tsp_parse_commandline(int argc, char** argv, instance* inst){
    if(argc < 2){
        printf("Type %s --help to see the full list of commands\n", argv[0]);
        exit(1);
    }

    inst->options_t.graph_random = false;
    inst->options_t.graph_input = false;
    inst->options_t.timelimit = -1;
    inst->options_t.seed = 0;
    inst->nnodes = -1;
    bool help = false;
    bool algs = false;

    for(int i=1; i<argc; i++){

        if (strcmp("-f", argv[i]) == 0 || strcmp("-file", argv[i]) == 0){
            if(invalid_input(i, argc, &help)){
                continue;
            }

            if(inst->options_t.graph_random){
                perror("Arguments: you can't have both random generation and input file");
                exit(1);
            }

            const char* path = argv[++i];
            if(!file_exists(path)){
                perror("Arguments: file does not exist!");
                exit(1);
            }
            inst->options_t.inputfile = (char*) calloc(strlen(path), sizeof(char));
            strcpy(inst->options_t.inputfile, path);

            inst->options_t.graph_input = true;

            continue;
        }

        if (strcmp("-t", argv[i]) == 0 || strcmp("-time", argv[i]) == 0){
            if(invalid_input(i, argc, &help)){
                continue;
            }

            const int t = atoi(argv[++i]);
            if(t<0){
                perror("Arguments: time cannot be negative");
                exit(1);
            }
            inst->options_t.timelimit = t;
            continue;
        }

        if (strcmp("-seed", argv[i]) == 0){
            if(invalid_input(i, argc, &help)){
                continue;
            }

            inst->options_t.seed = atoi(argv[++i]);
            continue;
        }

        // TODO: add flags for algorithm chosen
        if (strcmp("-alg", argv[i]) == 0){
            if(invalid_input(i, argc, &help)){
                continue;
            }

            const char* method = argv[++i];

            if (strncmp("GREEDY", method, 6) == 0){
                printf("GREEDY\n");
            }else if (strncmp("2OPT-GREEDY", method, 6) == 0){
                printf("2OPT-GREEDY\n");
            }

            continue;
        }

        if (strcmp("-n", argv[i]) == 0){
            if(invalid_input(i, argc, &help)){
                continue;
            }

            if(inst->options_t.graph_input){
                perror("Arguments: you can't have both random generation and input file");
                exit(1);
            }

            int n = atoi(argv[++i]);
            if(n <= 0){
                perror("Arguments: number of nodes should be greater than 0");
                exit(1);
            }
            inst->nnodes = n;

            inst->options_t.graph_random = true;

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
    }

    if(algs){
        printf(COLOR_BOLD "Available algorithms:\n" COLOR_OFF);
        printf("    - GREEDY\n");
        printf("    - 2OPT-GREEDY\n");
    }
}

void tsp_generate_randompoints(instance* inst){
    srand(inst->options_t.seed);

    inst->points = (point*) calloc(inst->nnodes, sizeof(point));

    for(int i=0; i<inst->nnodes; i++){
        inst->points[i].x = TSP_RAND();
        inst->points[i].y = TSP_RAND();
    }
}

// TODO: doesnt save to file
void tsp_plot_points(instance* inst, char* name, bool to_file){
    int i;
    PLOT plot = plot_open(name);
    // fill it with data
    fprintf(plot, "plot '-' with points pointtype 7\n");

    for(i=0; i<inst->nnodes; i++){
        plot_point(plot, &inst->points[i]);
    }

    if(to_file){
        plot_tofile(plot, name);
    }

    plot_free(plot);
}


void tsp_free_instance(instance *inst){
    free(inst->options_t.inputfile);
    free(inst->points);
}

void tsp_read_input(instance* inst){
    FILE *input_file = fopen(inst->options_t.inputfile, "r");
	if ( input_file == NULL ) print_error(" input file not found!");

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
			if ( inst->nnodes >= 0 ) print_error("two DIMENSION parameters in the file");
			token1 = strtok(NULL, " :");
			inst->nnodes = atoi(token1);	 
			inst->points = (point *) calloc(inst->nnodes, sizeof(point));
			continue;
		}

        if ( strncmp(parameter, "NODE_COORD_SECTION", 18) == 0 ) 
		{
			if ( inst->nnodes <= 0 ) print_error("DIMENSION not found");
			node_section = 1;   
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
}
