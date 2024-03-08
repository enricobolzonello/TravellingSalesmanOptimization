#include "errors.h"

bool err_ok(ERROR_CODE error){
    if(error == OK){
        return true;
    }

    return false;
}

// TODO: __FILE__ and __LINE__ considers errors.c things
void err_logging(LOGGING_TYPE level, const char *file, int line, char* message, ...){
    char buf[16];
    time_t t = time(NULL);
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t))] = '\0';
    printf("%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m %s\n",
    buf, level_colors[level], level_strings[level],
    file, line, message);


    /*if(level == LOG_FATAL){
        err_printerror();
    }*/
}
