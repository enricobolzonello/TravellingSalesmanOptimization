#include "plot.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || __linux__
    static char* TERMINAL = "x11";
#elif  __APPLE__
    static char* TERMINAL = "qt";
#else
    error "not supported"
#endif

PLOT plot_open(char* title){
    PLOT plot = popen("gnuplot -persistent", "w");
    if(plot == NULL){
        log_fatal("cannot open pipe");
    }
    fprintf(plot, "set term %s font \"Arial\"\n", TERMINAL);
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

void plot_stats(PLOT plot){
    fprintf(plot, "set style data line\n");
    fprintf(plot, "set xrange [0:500]\n");
    fprintf(plot, "set datafile separator \",\" \n");
    fprintf(plot, "f(x) = log(1+x) \n");

    fprintf(plot, "stats 'results/TabuResults.dat' u 1:2 prefix \"B\" nooutput\n");

    fprintf(plot, "plot 'results/TabuResults.dat' using 1:2 title \"  Data\" lw 2,  B_slope * x + B_intercept with lines title \"  Linear fit\", 'results/TabuResults.dat' using (column(1)):(column(2)==B_min_y ? column(2) : 1/0) with points pt 7 lc \"red\" title \"Minimum: \" . gprintf(\"%%.2f\", B_min_y) \n");
}

void plot_free(PLOT plot){
    fflush(plot);
    pclose(plot);
}
