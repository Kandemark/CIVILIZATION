/**
 * @file noise.h
 * @brief Perlin noise generation utilities
 */

#ifndef CIVILIZATION_NOISE_H
#define CIVILIZATION_NOISE_H

#include "../../include/types.h"

/**
 * @brief Generate 2D Perlin noise
 */
civ_float_t civ_noise_perlin(civ_float_t x, civ_float_t y, uint32_t seed);

/**
 * @brief Generate 3D Perlin noise for seamless mapping
 */
civ_float_t civ_noise_perlin3d(civ_float_t x, civ_float_t y, civ_float_t z,
                               uint32_t seed);

/**
 * @brief Generate Octave Perlin noise (Fractal Brownian Motion)
 */
civ_float_t civ_noise_octave(civ_float_t x, civ_float_t y, int octaves,
                             civ_float_t persistence, civ_float_t scale,
                             uint32_t seed);

/**
 * @brief Generate 3D Octave Perlin noise
 */
civ_float_t civ_noise_octave3d(civ_float_t x, civ_float_t y, civ_float_t z,
                               int octaves, civ_float_t persistence,
                               civ_float_t scale, uint32_t seed);

#endif
