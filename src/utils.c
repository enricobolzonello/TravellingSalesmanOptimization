#include "utils.h"

bool file_exists (const char *filename) {
  struct stat  buffer;   
  return (stat (filename, &buffer) == 0);
}

void parse_commandline(int argc, char** argv, instance* inst){
    if(argc < 2){
        printf("Type %s --help to see the full list of commands\n", argv[0]);
        exit(1);
    }

    inst->options_t.timelimit = -1;
    inst->options_t.seed = time(NULL);
    bool help = false;

    for(int i=0; i<argc; i++){

        if (strcmp("-f", argv[i]) == 0 || strcmp("-file", argv[i]) == 0){
            const char* path = argv[++i];
            if(!file_exists(path)){
                perror("Arguments: file does not exist!");
                exit(1);
            }
            inst->options_t.inputfile = (char*) calloc(strlen(path), sizeof(char));
            strcpy(inst->options_t.inputfile, path);
            continue;
        }

        if (strcmp("-t", argv[i]) == 0 || strcmp("-time", argv[i]) == 0){
            const int t = atoi(argv[i++]);
            if(t<0){
                perror("Arguments: time cannot be negative");
                exit(1);
            }
            inst->options_t.timelimit = t;
            continue;
        }

        if (strcmp("-seed", argv[i]) == 0){
            inst->options_t.seed = atoi(argv[i++]);
            continue;
        }

        // TODO: add flags for algorithm chosen
        if (strcmp("-alg", argv[i]) == 0){
            
            const char* method = argv[++i];

            if (strncmp("GREEDY", method, 6) == 0){

            }else if (strncmp("2OPT-GREEDY", method, 6) == 0){

            }

            continue;
        }

        if (strcmp("-n", argv[i]) == 0){
            const int n = atoi(argv[i++]);
            if(n <= 0){
                perror("Arguments: number of nodes should be greater than 0");
                exit(1);
            }
            inst->nnodes = n;
            continue;
        }

        if(strcmp("-h", argv[i] == 0) || strcmp("-help", argv[i] == 0) || strcmp("--help", argv[i] == 0)){
            help = true;
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
        printf("    --all_algs              prints all possible algorithms");
    }
}