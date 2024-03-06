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

void utils_print_error(char* message){
  printf("Error: %s\n", message);
  exit(1);
}
