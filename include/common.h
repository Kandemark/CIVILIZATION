/**
 * @file common.h
 * @brief Common definitions and utilities for the Civilization simulation
 * 
 * This file contains common types, constants, and utility macros used throughout
 * the C implementation of the Civilization game.
 */

#ifndef CIVILIZATION_COMMON_H
#define CIVILIZATION_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Version information */
#define CIV_VERSION_MAJOR 0
#define CIV_VERSION_MINOR 1
#define CIV_VERSION_PATCH 0

/* Memory management macros */
#define CIV_MALLOC(size) malloc(size)
#define CIV_CALLOC(count, size) calloc(count, size)
#define CIV_REALLOC(ptr, size) realloc(ptr, size)
#define CIV_FREE(ptr) do { if (ptr) { free(ptr); ptr = NULL; } } while(0)

/* Array size macro */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Min/Max macros */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(val, min, max) (MAX(min, MIN(max, val)))

/* String utilities */
#define STRING_MAX_LEN 256
#define STRING_MEDIUM_LEN 128
#define STRING_SHORT_LEN 64

/* Error codes */
typedef enum {
    CIV_OK = 0,
    CIV_ERROR_NULL_POINTER = -1,
    CIV_ERROR_OUT_OF_MEMORY = -2,
    CIV_ERROR_INVALID_ARGUMENT = -3,
    CIV_ERROR_NOT_FOUND = -4,
    CIV_ERROR_INVALID_STATE = -5,
    CIV_ERROR_IO = -6
} civ_error_t;

/* Result type for operations */
typedef struct {
    civ_error_t error;
    const char* message;
} civ_result_t;

/* Boolean result */
#define CIV_SUCCESS(result) ((result).error == CIV_OK)
#define CIV_FAILED(result) ((result).error != CIV_OK)

/* Logging levels */
typedef enum {
    CIV_LOG_DEBUG = 0,
    CIV_LOG_INFO = 1,
    CIV_LOG_WARNING = 2,
    CIV_LOG_ERROR = 3,
    CIV_LOG_FATAL = 4
} civ_log_level_t;

/* Logging function */
void civ_log(civ_log_level_t level, const char* format, ...);

/* Assertion macro */
#ifdef DEBUG
#define CIV_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            civ_log(CIV_LOG_FATAL, "Assertion failed: %s at %s:%d", \
                    message, __FILE__, __LINE__); \
            abort(); \
        } \
    } while(0)
#else
#define CIV_ASSERT(condition, message) ((void)0)
#endif

#endif /* CIVILIZATION_COMMON_H */

