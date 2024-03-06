#include "tsp.h"

int main(int argc, char* argv[]){
    instance inst;
    tsp_parse_commandline(argc, argv, &inst);
    tsp_generate_randompoints(&inst);
    exit(0);
}
