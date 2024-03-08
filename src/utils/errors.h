#ifndef ERRORS_H_
#define ERRORS_H_

#include<stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// TODO: add verbosity
// https://github.com/googleapis/googleapis/blob/master/google/rpc/code.proto
typedef enum{
    OK = 0,
    CANCELLED = 1,
    UNKNOWN = 2,
    INVALID_ARGUMENT = 3,
    DEADLINE_EXCEEDED = 4,
    NOT_FOUND = 5,
    ALREADY_EXISTS = 6,
    PERMISSION_DENIED = 7,
    UNAUTHENTICATED = 16,
    RESOURCE_EXHAUSTED = 8,
    FAILED_PRECONDITION = 9,
    ABORTED = 10,
    OUT_OF_RANGE = 11,
    UNIMPLEMENTED = 12,
    INTERNAL = 13,
    UNAVAILABLE = 14,
    DATA_LOSS = 15
} ERROR_CODE;

typedef enum{
    LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL
} LOGGING_TYPE;

#define log_trace(...) err_logging(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) err_logging(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  err_logging(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  err_logging(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) err_logging(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) err_logging(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

bool err_ok(ERROR_CODE error);
void err_logging(LOGGING_TYPE level, const char *file, int line, char* message, ...);

#endif
