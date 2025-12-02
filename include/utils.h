/* utils.h - Helper functions for randomness, noise, interpolation */
#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdlib.h>


/* Simple pseudo‑random number generator state */
typedef struct {
  unsigned int seed;
} UtilsState;

/* Initialize utils with a deterministic seed */
void utils_init(UtilsState *u, unsigned int seed);

/* Return a float in [0,1) */
float utils_rand_float(UtilsState *u);

/* Simple 2‑D value noise (hash based) */
float utils_noise2d(UtilsState *u, int x, int y);

#endif // UTILS_H
