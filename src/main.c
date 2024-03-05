#include "utils.h"

int main(int argc, char* argv[]){
    instance inst;
    parse_commandline(argc, argv, &inst);
    exit(0);
}
