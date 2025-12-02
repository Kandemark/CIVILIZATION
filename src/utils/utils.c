/* utils.c - Implementation of utility functions */
#include "utils.h"

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

/* OPTIMIZED: Inline for performance in hot loops */
static inline float utils_rand_float_impl(UtilsState *restrict u) {
  if (!u)
    return 0.0f;
  const unsigned int val = lcg_next(&u->seed);
  // OPTIMIZATION: Use multiplication instead of division
  // Scale to [0,1) - compiler can optimize this better
  return (float)val * (1.0f / (float)UINT_MAX);
}

float utils_rand_float(UtilsState *u) { return utils_rand_float_impl(u); }

float utils_noise2d(UtilsState *u, int x, int y) {
  // OPTIMIZED: Simple hash based value noise with better mixing
  // Use prime numbers for better distribution
  unsigned int h = (unsigned int)(x * 374761393u + y * 668265263u);
  h = (h ^ (h >> 13)) * 1274126177u;
  h = h ^ (h >> 16);
  // Mix with RNG state for more variation
  h ^= u->seed;
  // OPTIMIZATION: Use bit masking and multiplication instead of division
  return (float)(h & 0xFFFFu) * (1.0f / 65536.0f); // [0,1)
}
