#include "utils.h"

static char* algs_string[10] = {
    "Greedy", "Greedy\\_Iter", "2opt\\_Greedy", "Tabu\\_Search", "VNS", "Cplex\\_NoSec", "Cplex\\_BendersLoop", "Extra\\_Mileage", "Cplex\\_BendersLoop\\_Patching", "Cplex\\_Branch\\&Cut"
};

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
  struct tm *timenow;

  time_t now = time(NULL);
  timenow = gmtime(&now);

  strftime(plotname, sizeof(plotname), "PLOT_%Y-%m-%d_%H:%M:%S", timenow);

  strncpy(buffer, plotname, buffersize-1);
  buffer[buffersize-1] = '\0';

}

void utils_format_title(char *fname, int alg)
{
    // escape the underscore character (because it's LaTeX)
    size_t length = strlen(fname);
    size_t new_length = length; // Length of the modified string
    for (size_t i = 0; i < length; i++) {
        if (fname[i] == '_') {
            new_length++; // Increase length to accommodate the additional backslash
        }
    }

    // Move characters to their new positions, inserting backslashes before underscores
    for (size_t i = length - 1, j = new_length - 1; i < new_length && i < j; i--, j--) {
        if (fname[i] == '_') {
            fname[j--] = '_'; // Move underscore to its new position
            fname[j] = '\\'; // Insert backslash before underscore
        } else {
            fname[j] = fname[i]; // Move other characters as they are
        }
    }

    // strip the extension
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.') {
        --end;
    }

    if (end > fname) {
        *end = '\0';
    }

    // get string of algorithm
    const char* alg_s = NULL;
    if (alg >= 0 && alg < sizeof(algs_string) / sizeof(char*)) {
        alg_s = algs_string[alg];
    }

    // Copy characters from s2 to the end of s1
    while (*alg_s) {
        *end = *alg_s;
        end++;
        alg_s++;
    }
    
    // Add the null terminator at the end
    *end = '\0';
}

void swap(int* a, int* b){
    int tmp = *a;
	*a = *b;
	*b = tmp;
}
