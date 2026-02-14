/**
 * @file noise.c
 * @brief Implementation of Perlin noise (2D and 3D)
 */

#include "../../include/utils/noise.h"
#include <math.h>

/* Permutation table for Perlin noise */
static const int perm[512] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
    36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120,
    234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
    88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
    134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133,
    230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161,
    1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130,
    116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250,
    124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227,
    47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44,
    154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98,
    108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34,
    242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14,
    239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121,
    50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243,
    141, 128, 195, 78, 66, 215, 61, 156, 180,
    /* Repeat */
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
    36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120,
    234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
    88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
    134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133,
    230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161,
    1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130,
    116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250,
    124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227,
    47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44,
    154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98,
    108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34,
    242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14,
    239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121,
    50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243,
    141, 128, 195, 78, 66, 215, 61, 156, 180};

static civ_float_t fade(civ_float_t t) {
  return t * t * t * (t * (t * 6 - 15) + 10);
}

static civ_float_t lerp(civ_float_t t, civ_float_t a, civ_float_t b) {
  return a + t * (b - a);
}

static civ_float_t grad(int hash, civ_float_t x, civ_float_t y) {
  int h = hash & 15;
  civ_float_t u = h < 8 ? x : y;
  civ_float_t v = h < 4 ? y : h == 12 || h == 14 ? x : 0;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

static civ_float_t grad3(int hash, civ_float_t x, civ_float_t y,
                         civ_float_t z) {
  int h = hash & 15;
  civ_float_t u = h < 8 ? x : y;
  civ_float_t v = h < 4 ? y : h == 12 || h == 14 ? x : z;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

civ_float_t civ_noise_perlin(civ_float_t x, civ_float_t y, uint32_t seed) {
  x += (seed & 0xFFFF) * 0.131f;
  y += ((seed >> 16) & 0xFFFF) * 0.173f;
  int X = (int)floor(x) & 255;
  int Y = (int)floor(y) & 255;
  x -= floor(x);
  y -= floor(y);
  civ_float_t u = fade(x);
  civ_float_t v = fade(y);
  int A = perm[X] + Y;
  int B = perm[X + 1] + Y;
  return lerp(
      v, lerp(u, grad(perm[A], x, y), grad(perm[B], x - 1, y)),
      lerp(u, grad(perm[A + 1], x, y - 1), grad(perm[B + 1], x - 1, y - 1)));
}

civ_float_t civ_noise_perlin3d(civ_float_t x, civ_float_t y, civ_float_t z,
                               uint32_t seed) {
  x += (seed & 0x3FFF) * 0.191f;
  y += ((seed >> 14) & 0x3FFF) * 0.137f;
  z += ((seed >> 28) & 0xF) * 0.121f;
  int X = (int)floor(x) & 255, Y = (int)floor(y) & 255, Z = (int)floor(z) & 255;
  x -= floor(x);
  y -= floor(y);
  z -= floor(z);
  civ_float_t u = fade(x), v = fade(y), w = fade(z);
  int A = perm[X] + Y, AA = perm[A] + Z, AB = perm[A + 1] + Z;
  int B = perm[X + 1] + Y, BA = perm[B] + Z, BB = perm[B + 1] + Z;
  return lerp(
      w,
      lerp(v, lerp(u, grad3(perm[AA], x, y, z), grad3(perm[BA], x - 1, y, z)),
           lerp(u, grad3(perm[AB], x, y - 1, z),
                grad3(perm[BB], x - 1, y - 1, z))),
      lerp(v,
           lerp(u, grad3(perm[AA + 1], x, y, z - 1),
                grad3(perm[BA + 1], x - 1, y, z - 1)),
           lerp(u, grad3(perm[AB + 1], x, y - 1, z - 1),
                grad3(perm[BB + 1], x - 1, y - 1, z - 1))));
}

civ_float_t civ_noise_octave(civ_float_t x, civ_float_t y, int octaves,
                             civ_float_t persistence, civ_float_t scale,
                             uint32_t seed) {
  civ_float_t total = 0, frequency = scale, amplitude = 1, maxValue = 0;
  for (int i = 0; i < octaves; i++) {
    total += civ_noise_perlin(x * frequency, y * frequency, seed) * amplitude;
    maxValue += amplitude;
    amplitude *= persistence;
    frequency *= 2;
  }
  return (total / maxValue) + 0.5f;
}

civ_float_t civ_noise_octave3d(civ_float_t x, civ_float_t y, civ_float_t z,
                               int octaves, civ_float_t persistence,
                               civ_float_t scale, uint32_t seed) {
  civ_float_t total = 0, frequency = scale, amplitude = 1, maxValue = 0;
  for (int i = 0; i < octaves; i++) {
    total +=
        civ_noise_perlin3d(x * frequency, y * frequency, z * frequency, seed) *
        amplitude;
    maxValue += amplitude;
    amplitude *= persistence;
    frequency *= 2;
  }
  return (total / maxValue) + 0.5f;
}
