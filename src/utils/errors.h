#ifndef ERRORS_H_
#define ERRORS_H_

/**
 * @file errors.h
 * @author Enrico Bolzonello (enrico.bolzonello@studenti.unipd.it)
 * @brief defines helpers for logging and handling errors
 * @version 0.1
 * @date 2024-03-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define COLOR_BOLD  "\033[1m"
#define COLOR_OFF   "\033[m"

#define NUM_LINES 25
#define MAX_LINE_LENGTH 2048

/**
 * @brief error codes as refered in <a href="https://github.com/googleapis/googleapis/blob/master/google/rpc/code.proto">Google's RPC standard</a> 
 * 
 */
typedef enum{
    T_OK = 0,               // not error
    CANCELLED = 1,          // not error
    UNKNOWN = 2,
    INVALID_ARGUMENT = 3,
    DEADLINE_EXCEEDED = 4,  // not error
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

/**
 * @brief logging types
 * 
 */
typedef enum{
    LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL
} LOGGING_TYPE;

/**
 * @brief verbosity levels
 * -q QUIET, only output
 * (none) NORMAL, only warn, errors and fatal
 * -v VERBOSE, normal verbosity plus info
 * -vv VERY_VERBOSE, verbose plus debug and trace
 * 
 * reference: https://symfony.com/doc/current/console/verbosity.html
 */
typedef enum{
  QUIET = 0,
  NORMAL = 1,
  VERBOSE = 2,
  VERY_VERBOSE = 3
} VERBOSITY;

#define log_trace(...) err_logging(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) err_logging(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  err_logging(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  err_logging(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) err_logging(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) err_logging(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define log_status(message) err_status(message, __FILE__, __LINE__);

/**
 * @brief returns true if the error\_code is OK
 * 
 * @param error ERROR_CODE
 * @return true If it is OK
 * @return false If it is an error
 */
bool err_ok(ERROR_CODE error);

/**
 * @brief set verbosity levels, as defined in \link VERBOSITY \endlink
 * 
 * @param verbosity 
 */
void err_setverbosity(VERBOSITY verbosity);

/**
 * @brief Util to check if you can log based on the set verbosity level
 * 
 * @return true If >= VERBOSE
 * @return false Otherwise
 */
bool err_dolog(void);

/**
 * @brief Helper function to print logs
 * 
 * @param level 
 * @param file 
 * @param line 
 * @param message 
 * @param ... 
 */
void err_logging(LOGGING_TYPE level, const char *file, int line, char* message, ...);

void err_status(char* message, const char *file, int line);

void err_printoutput(double cost, double time, int alg);

void err_setinfo(int alg, int nnodes, bool random, char* inputfile, double timelimit, int seed, int tabu_policy, int em_init, bool init_mip, int bc_policy, bool callback_relaxation,  double lb_improv, int lb_delta, bool lb_kstar);

#endif
