#include "errors.h"

bool err_ok(ERROR_CODE error){
    if(error == OK){
        return true;
    }

    return false;
}

void err_setverbosity(VERBOSITY verbosity){
    L.verbosity = verbosity;
}

void err_logging(LOGGING_TYPE level, const char *file, int line, char* message, ...){
    if(!(L.verbosity == QUIET) && !(L.verbosity == NORMAL && (level == LOG_INFO || level == LOG_TRACE || level == LOG_DEBUG)) && !(L.verbosity == VERBOSE && (level == LOG_TRACE || level == LOG_DEBUG))){
        va_list arg;
        va_start(arg, message);

        char buf[16];
        time_t t = time(NULL);
        buf[strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t))] = '\0';
        printf("%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
        buf, level_colors[level], level_strings[level],
        file, line);
        vprintf(message, arg);
        printf("\n");
        va_end(arg);
    }
}
