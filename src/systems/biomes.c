/* biomes.c - Biome classification based on elevation, temperature, and moisture
 */
#include "biomes.h"
#include "cell.h"
#include "config.h"
#include "world.h"


void biomes_update(World *w) {
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      WorldCell *cell = &w->cells[y][x];
      // Skip ocean cells (already water)
      if (w->geo.water[y][x]) {
        cell->terrain = TERRAIN_OCEAN;
        continue;
      }
      float elev = w->geo.elevation[y][x];
      float temp = w->clim.temperature[y][x];
      float moist = w->clim.moisture[y][x];

      // Simple Whittaker-like rules
      if (elev > 2000.0f) {
        cell->terrain = TERRAIN_MOUNTAINS;
      } else if (elev > 1000.0f) {
        cell->terrain = TERRAIN_HILLS;
      } else if (moist < 0.2f && temp > 20.0f) {
        cell->terrain = TERRAIN_DESERT;
      } else if (temp < -10.0f) {
        cell->terrain = TERRAIN_SNOW;
      } else if (temp < 5.0f) {
        cell->terrain = TERRAIN_TUNDRA;
      } else if (temp > 15.0f && moist > 0.5f) {
        cell->terrain = TERRAIN_FOREST;
      } else {
        cell->terrain = TERRAIN_PLAINS;
      }
    }
  }
}
