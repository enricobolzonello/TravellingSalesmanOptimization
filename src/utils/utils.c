#include "utils.h"

static char* algs_string[12] = {
    "Greedy", "Greedy\\_Iter", "2opt\\_Greedy", "Tabu\\_Search", "VNS", "Cplex\\_NoSec", "Cplex\\_BendersLoop", "Extra\\_Mileage", "Cplex\\_BendersLoop\\_Patching", "Cplex\\_Branch\\&Cut", "Hard\\_Fixing", "Local\\_Branching"
};

void utils_safe_memory_free (void ** pointer_address)
{
  if (pointer_address != NULL && *pointer_address != NULL)
  {
    free(*pointer_address);

    *pointer_address = NULL;
  }
}

bool utils_file_exists (const char *filename) {
  struct stat  buffer;   
  return (stat (filename, &buffer) == 0);
}

bool utils_invalid_input(int i, int argc, bool* help){
    if (i+1 > argc){
        *help = 1;
        return true;
    }

    return false;
}

void utils_startclock(struct timespec* c){
  if(clock_gettime(CLOCK_MONOTONIC, c) == -1 ){
    log_error("monotonic clock not supported");
  }
}

double utils_timeelapsed(struct timespec* c){
  struct timespec finish;
  if(clock_gettime(CLOCK_MONOTONIC, &finish) == -1){
    log_error("monotonic clock not supported");
    return -1.0;
  }
  return (finish.tv_sec - c->tv_sec) + ( (finish.tv_nsec - c->tv_nsec) / 1000000000.0 );
}

void utils_print_array(int* arr){
  int size = sizeof(*arr) / sizeof(arr[0]);

  printf("Array: ");
  for (int i = 0; i < size; i++) {
      printf("%d ", arr[i]);
  }
  printf("\n");
}

void utils_plotname(char* buffer, int buffersize){
  char plotname[40];
  struct tm timenow;

  time_t now = time(NULL);
  localtime_r(&now, &timenow);

  strftime(plotname, sizeof(plotname), "PLOT_%Y-%m-%d_%H:%M:%S", &timenow);

  strncpy(buffer, plotname, buffersize-1);
  buffer[buffersize-1] = '\0';

}

void utils_format_title(char *fname, int alg) {
  size_t num_algs = sizeof(algs_string) / sizeof(algs_string[0]);

    log_debug("fname: %s", fname);
    // Escape the underscore character (because it's LaTeX)
    size_t length = strlen(fname) + 50;
    size_t new_length = length; // Length of the modified string
    for (size_t i = 0; i < length; i++) {
        if (fname[i] == '_') {
            new_length++; // Increase length to accommodate the additional backslash
        }
    }

    log_debug("old size: %d", length);
    log_debug("new size: %d", new_length);

    // Ensure there's enough space for the modifications
    char *new_fname = (char *)malloc(new_length + 1); // +1 for the null terminator
    if (new_fname == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    // Move characters to their new positions, inserting backslashes before underscores
    int j=0;
    for (size_t i = 0; i < length; i++) {
        if (fname[i] == '_') {
            new_fname[j++] = '\\'; // Insert backslash before underscore
            new_fname[j++] = '_'; // Move underscore to its new position
        } else {
            new_fname[j++] = fname[i]; // Move other characters as they are
        }
    }
    new_fname[new_length] = '\0';

    log_debug("new_fname: %s", new_fname);

    // Strip the extension
    char *dot_position = strrchr(new_fname, '.');
    if (dot_position != NULL) {
        *dot_position = '\0';
    }

    // Get string of algorithm
    const char *alg_s = NULL;
    if (alg >= 0 && alg < (int)num_algs) {
        alg_s = algs_string[alg];
    }

    // Concatenate the algorithm string
    if (alg_s != NULL) {
        strcat(new_fname, alg_s);
    }

    // Copy the new string back to the original pointer (ensure it fits)
    strncpy(fname, new_fname, new_length + 1);

    // Free allocated memory
    utils_safe_free(new_fname);
}

void swap(int* a, int* b){
    int tmp = *a;
	*a = *b;
	*b = tmp;
}

ERROR_CODE tsp_init_solution(int nnodes, tsp_solution* solution){
    ERROR_CODE e = T_OK;
    solution->path = (int*) calloc(nnodes, sizeof(int));
    if(solution->path == NULL){
        log_fatal("solution.path allocation failed");
        e = UNAVAILABLE;
        exit(0);
    }

    solution->cost = __DBL_MAX__;
    solution->ncomp = 0;
    solution->comp = (int*) calloc(nnodes, sizeof(int));
    if(solution->comp == NULL){
        log_fatal("solution.comp allocation failed");
        e = UNAVAILABLE;
    }
    return e;
}
