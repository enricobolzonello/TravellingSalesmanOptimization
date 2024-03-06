#include "plot.h"

PLOT plot_open(char* title){
    PLOT plot = popen("gnuplot -persistent", "w");
    fprintf(plot, "set title '%s'\n", title);
    return plot;
}

void plot_point(PLOT plot, point* p){
    fprintf(plot, "%lf %lf \n", p->x, p->y);
}

void plot_tofile(PLOT plot, char* filename){
    mkdir("../plots", 0700);
    fprintf(plot, "set terminal png size 700, 700\n");
    fprintf(plot, "set output '../plots/%s.jpg'\n", filename);
}

void plot_args(PLOT plot, char* args){
    fprintf(plot, "%s\n", args);
}

void plot_free(PLOT plot){
    fprintf(plot, "e\n");
    fflush(plot);
    pclose(plot);
}
