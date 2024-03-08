#include "plot.h"

PLOT plot_open(char* title){
    PLOT plot = popen("gnuplot -persistent", "w");
    fprintf(plot, "set title '%s'\n", title);
    return plot;
}

void plot_point(PLOT plot, point* p){
    fprintf(plot, "%lf %lf \n", p->x, p->y);
}

void plot_edge(PLOT plot, point u, point v){
    fprintf(plot, "%lf %lf \n", u.x, u.y);
    fprintf(plot, "%lf %lf \n\n", v.x, v.y);
}

void plot_tofile(PLOT plot, char* filename){
    mkdir("plots", 0777);
    fprintf(plot, "set terminal pngcairo\n");
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
