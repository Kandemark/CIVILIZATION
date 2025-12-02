/* utils.c - Implementation of utility functions */
#include "utils.h"
#include <limits.h>

/* Simple linear congruential generator */
static unsigned int lcg_next(unsigned int *seed) {
  // Constants from Numerical Recipes
  *seed = (1664525u * (*seed) + 1013904223u);
  return *seed;
}

void utils_init(UtilsState *u, unsigned int seed) {
  if (u) {
    u->seed = seed ? seed : 123456789u; // default deterministic seed
  }
}

float utils_rand_float(UtilsState *u) {
  if (!u)
    return 0.0f;
  unsigned int val = lcg_next(&u->seed);
  // Scale to [0,1)
  return (float)val / (float)UINT_MAX;
}

float utils_noise2d(UtilsState *u, int x, int y) {
  // Simple hash based value noise
  unsigned int h = (unsigned int)(x * 374761393u + y * 668265263u);
  h = (h ^ (h >> 13)) * 1274126177u;
  h = h ^ (h >> 16);
  // Mix with RNG state for more variation
  h ^= u->seed;
  return (float)(h & 0xFFFFu) / 65536.0f; // [0,1)
}
