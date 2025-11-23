/**
 * @file common.c
 * @brief Implementation of common utilities
 */

#include "../../include/common.h"
#include <stdarg.h>

void civ_log(civ_log_level_t level, const char* format, ...) {
    const char* level_names[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};
    va_list args;
    va_start(args, format);
    
    fprintf(stderr, "[%s] ", level_names[level]);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    va_end(args);
}

