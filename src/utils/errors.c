#include "errors.h"

static struct{
  int verbosity;
} L;

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

static char* algs_string[12] = {
    "Nearest Neighbour", "All Nearest Neighbour", "Nearest Neighbour + 2OPT", "Tabu Search", "Variable Neighborhood Search", "CPLEX No SECs", "CPLEX Benders Loop", "Extra Mileage", "Cplex BendersLoop + Patching", "CPLEX Branch&Cut", "Hard Fixing", "Local Branching"
};

static char* tenure_policy_string[4] = {
  "Fixed", "Size Dependent", "Random", "Linear"
};

static char* em_init_string[2] = {
  "Max", "Random"
};

static char* bc_policy_string[3] = {
  "Probability", "# of nodes", "Depth"
};

bool err_ok(ERROR_CODE error){
    if(error == T_OK || error == CANCELLED || error == DEADLINE_EXCEEDED){
        return true;
    }

    return false;
}

void err_setverbosity(VERBOSITY verbosity){
    L.verbosity = verbosity;
}

bool err_dolog(void){
    return L.verbosity >= VERBOSE;
}

void err_logging(LOGGING_TYPE level, const char *file, int line, char* message, ...){
    if(!(L.verbosity == QUIET) && !(L.verbosity == NORMAL && (level == LOG_INFO || level == LOG_TRACE || level == LOG_DEBUG)) && !(L.verbosity == VERBOSE && (level == LOG_TRACE || level == LOG_DEBUG))){
        va_list arg;
        va_start(arg, message);

        char buf[16];
        time_t t = time(NULL);
        buf[strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t))] = '\0';

        fprintf(stderr,"%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",buf, level_colors[level], level_strings[level], file, line);
        vfprintf(stderr, message, arg);
        fprintf(stderr, "\n");

        va_end(arg);
    }
}

void err_status(char* message, const char *file, int line) {
  fprintf(stderr, "\033[1A");
	fprintf(stderr, "\033[K");
	err_logging(LOG_INFO, file, line, "%s\t%s", message, "✅");
	fprintf(stderr, "\033[1B"); // Move cursor back down one line to the new line
  fflush(stdout);    // Flush the output buffer to ensure the line is printed immediately
}

void err_printline(void){
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  // horizontal line
    for(int i=0; i<w.ws_col; i++){
      printf("\u2500");
    }
    printf("\n\n");
}

void err_setinfo(int alg, int nnodes, bool random, char* inputfile, double timelimit, int seed, int tabu_policy, int em_init, bool init_mip, int bc_policy, bool callback_relaxation, double lb_improv, int lb_delta, bool lb_kstar){
  if(!(L.verbosity == QUIET)){
    printf(COLOR_BOLD "Travelling Salesman Problem Solver v0.1\n" COLOR_OFF);
    // horizontal line
    err_printline();

    printf("Algorithm:               %s\n", algs_string[alg]);
    printf("Number of nodes:         %d\n", nnodes);


    char random_or_file[40];
    if(random){
      strcpy(random_or_file, "random");
    }else{
      snprintf(random_or_file, sizeof(random_or_file), "%s", inputfile);
    }
    printf("Random/File:             %s\n", random_or_file);

    char timelimit_string[50];
    if(timelimit != -1.0){
      snprintf(timelimit_string, 50, "%.2f", timelimit);
    }else{
      strcpy(timelimit_string, "not set");
    }
    printf("Timelimit:               %s\n", timelimit_string);

    char seed_string[10];
    if(seed != -1.0){
      snprintf(seed_string, 10, "%d", seed);
    }else{
      strcpy(seed_string, "not set");
    }
    printf("Seed:                    %s\n", seed_string);

    switch (alg)
    {
    case 3:
      printf("Tenure Policy:           %s\n", tenure_policy_string[tabu_policy]);
      break;
    case 7:
      printf("EM initialization:       %s\n", em_init_string[em_init]);
      break;
    case 9:
      printf("Use MIP start?           %s\n", init_mip ? "yes" : "no");
      printf("Skip Policy:             %s\n", bc_policy_string[bc_policy]);
      printf("Relaxation callback?     %s\n", callback_relaxation ? "yes" : "no");
      break;
    case 11:
      printf("Impr. to change k        %.2f\n", lb_improv * 100);
      printf("DeltaK                   %d\n", lb_delta);
      printf("Use KStar approach?      %s\n", lb_kstar ? "yes" : "no");
    default:
      break;
    }

    err_printline();

    printf("Press Enter to continue...\n");
    //getchar();
  }
}

void err_printoutput(double cost, double time, int alg){
  if(!(L.verbosity == QUIET)){

    err_printline();

    printf("algorithm: %s\n", algs_string[alg]);

    printf("cost: %.2f\n", cost);
    printf("execution time: %.2f seconds\n", time);

    err_printline();

    printf("Program finished, shutting down...\n");
  }else{
    printf("Cost: %.2f\n", cost);
  }
}
