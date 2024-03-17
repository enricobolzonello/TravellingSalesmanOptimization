#include "utils.h"

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

struct utils_clock utils_startclock(void){
  struct utils_clock c;
  c.starting_time = clock();
  c.started = true;

  return c;
}

double utils_timeelapsed(struct utils_clock c){
  if(!c.started){
    log_debug("clock not started");
    exit(0);
  }

  return (double) (clock() - c.starting_time) / CLOCKS_PER_SEC;
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

void utils_strip_ext(char *fname)
{
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.') {
        --end;
    }

    if (end > fname) {
        *end = '\0';
    }
}
