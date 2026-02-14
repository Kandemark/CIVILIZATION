/**
 * @file utils.h
 * @brief Utility functions and properly seeded RNG
 */

#ifndef CIVILIZATION_UTILS_H
#define CIVILIZATION_UTILS_H

#include "../common.h"

typedef struct {
  unsigned int seed;
} UtilsState;

/**
 * @brief Initialize utility state with seed
 */
void utils_init(UtilsState *u, unsigned int seed);

/**
 * @brief Get a random float in range [0, 1)
 */
float utils_rand_float(UtilsState *u);

/**
 * @brief 2D noise function
 */
float utils_noise2d(UtilsState *u, int x, int y);

#endif /* CIVILIZATION_UTILS_H */
