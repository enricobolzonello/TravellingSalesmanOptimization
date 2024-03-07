#include "plot.h"

PLOT plot_open(char* title){
    PLOT plot = popen("gnuplot -persistent", "w");
    fprintf(plot, "set title '%s'\n", title);
    fprintf(plot, "set terminal '%s' size 700, 700\n", "x11");
    return plot;
}

void plot_point(PLOT plot, point* p){
    fprintf(plot, "%lf %lf \n", p->x, p->y);
}


// TODO: doesn't print the plot
void plot_tofile(PLOT plot, char* filename){
    mkdir("plots", 0777);
    fprintf(plot, "set terminal png size 700, 700\n");
    fprintf(plot, "set output \"plots/%s.png\"\n", filename);
}

void plot_args(PLOT plot, char* args){
    fprintf(plot, "%s\n", args);
}

void plot_free(PLOT plot){
    fprintf(plot, "e\n");
    fflush(plot);
    pclose(plot);
}
