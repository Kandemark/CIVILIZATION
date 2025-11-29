/**
 * @file noise.h
 * @brief Perlin noise generation utilities
 */

#ifndef CIVILIZATION_NOISE_H
#define CIVILIZATION_NOISE_H

#include "../../include/types.h"

/**
 * @brief Generate 2D Perlin noise
 * @param x X coordinate
 * @param y Y coordinate
 * @param seed Random seed
 * @return Noise value between -1.0 and 1.0
 */
civ_float_t civ_noise_perlin(civ_float_t x, civ_float_t y, uint32_t seed);

/**
 * @brief Generate Octave Perlin noise (Fractal Brownian Motion)
 * @param x X coordinate
 * @param y Y coordinate
 * @param octaves Number of layers of noise
 * @param persistence How much amplitude decreases per octave (usually 0.5)
 * @param scale Zoom level (lower = zoomed out)
 * @param seed Random seed
 * @return Noise value normalized roughly between 0.0 and 1.0
 */
civ_float_t civ_noise_octave(civ_float_t x, civ_float_t y, int octaves,
                             civ_float_t persistence, civ_float_t scale,
                             uint32_t seed);

#endif
