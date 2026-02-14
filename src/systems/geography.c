/* geography.c - Implementation of terrain, elevation, water, erosion, desert
 * formation */
#include "../../include/core/environment/geography.h"
#include "../../include/core/environment/climate.h"
#include "../../include/utils/utils.h"
#include <math.h>
#include <stdlib.h>

/* Simple fractal noise based elevation */
static float generate_elevation(int x, int y, UtilsState *utils) {
  // Combine several octaves of value noise
  float freq = 0.05f;
  float amp = 1.0f;
  float elevation = 0.0f;
  for (int octave = 0; octave < 4; ++octave) {
    float nx = (float)x * freq;
    float ny = (float)y * freq;
    float noise = utils_noise2d(utils, (int)nx, (int)ny);
    elevation += noise * amp;
    freq *= 2.0f;
    amp *= 0.5f;
  }
  // Scale to realistic range
  return elevation * MAX_ELEVATION; // 0 .. MAX_ELEVATION
}

void geography_initialize(GeographyState *geo, UtilsState *utils) {
  if (!geo || !utils)
    return;
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      float elev = generate_elevation(x, y, utils);
      geo->elevation[y][x] = elev;
      // Water if below sea level
      geo->water[y][x] = (elev <= SEA_LEVEL) ? 1 : 0;
      geo->desert[y][x] = 0; // will be set later
      geo->river_volume[y][x] = 0.0f;
    }
  }
}

/* Very simple erosion: lower high peaks a bit based on neighboring heights */
void geography_erosion(GeographyState *geo, UtilsState *utils) {
  if (!geo)
    return;
  // Create a copy to read original values
  float copy[WORLD_HEIGHT][WORLD_WIDTH];
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      copy[y][x] = geo->elevation[y][x];
    }
  }
  for (int y = 1; y < WORLD_HEIGHT - 1; ++y) {
    for (int x = 1; x < WORLD_WIDTH - 1; ++x) {
      float sum = 0.0f;
      int count = 0;
      // average of the 8 neighbours
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          if (dx == 0 && dy == 0)
            continue;
          sum += copy[y + dy][x + dx];
          ++count;
        }
      }
      float avg = sum / (float)count;
      // Move current height a fraction towards the average
      float diff = avg - copy[y][x];
      geo->elevation[y][x] += diff * 0.1f; // erosion factor
      // Re‑evaluate water after erosion
      geo->water[y][x] = (geo->elevation[y][x] <= SEA_LEVEL) ? 1 : 0;
    }
  }
}

/* Desert formation based on a simple moisture threshold. */
void geography_update_deserts(GeographyState *geo) {
  if (!geo)
    return;
  const float DESERT_ELEVATION_THRESHOLD = 500.0f; // meters
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      if (!geo->water[y][x] &&
          geo->elevation[y][x] < DESERT_ELEVATION_THRESHOLD) {
        // Very low moisture heuristic
        geo->desert[y][x] = 1;
      } else {
        geo->desert[y][x] = 0;
      }
    }
  }
}

/* River generation:
   1. Identify potential sources (high rainfall, high elevation).
   2. Flow downhill to lowest neighbor.
   3. Accumulate volume.
   4. If volume > threshold, mark as river.
*/
void geography_update_rivers(GeographyState *geo, const ClimateState *clim) {
  if (!geo || !clim)
    return;

  // Reset river volume
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      geo->river_volume[y][x] = 0.0f;
    }
  }

  // Let's use a "droplet" approach for simplicity and performance in this demo.
  // Spawn droplets at random locations weighted by rainfall.
  // Trace them downhill.

  // Spawn N droplets.
  int num_droplets = WORLD_WIDTH * WORLD_HEIGHT * 2;
  for (int i = 0; i < num_droplets; ++i) {
    int rx = rand() % WORLD_WIDTH;
    int ry = rand() % WORLD_HEIGHT;

    // Only start if there is some rain here
    if (clim->rainfall[ry][rx] < 0.1f)
      continue;

    int cx = rx;
    int cy = ry;

    while (1) {
      // Add to volume
      geo->river_volume[cy][cx] += 1.0f;

      // Stop if in water
      if (geo->water[cy][cx])
        break;

      // Find lowest neighbor
      float min_elev = geo->elevation[cy][cx];
      int nx = -1, ny = -1;

      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          if (dx == 0 && dy == 0)
            continue;
          int tx = cx + dx;
          int ty = cy + dy;
          if (tx >= 0 && tx < WORLD_WIDTH && ty >= 0 && ty < WORLD_HEIGHT) {
            if (geo->elevation[ty][tx] < min_elev) {
              min_elev = geo->elevation[ty][tx];
              nx = tx;
              ny = ty;
            }
          }
        }
      }

      // If local minimum (pit), stop
      if (nx == -1)
        break;

      // Move to neighbor
      cx = nx;
      cy = ny;
    }
  }
}
