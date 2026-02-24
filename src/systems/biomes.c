/* biomes.c - Biome classification based on elevation, temperature, and moisture
 */
#include "../../include/core/environment/biomes.h"
#include "../../include/core/environment/cell.h"
#include "../../include/core/environment/world.h"
#include "../../include/utils/config.h"


/* Whittaker Diagram Lookup Table (Simplified)
 * Rows: Temperature (Hot to Cold indices)
 * Cols: Moisture (Dry to Wet indices)
 */
static const TerrainType BIOME_TABLE[5][5] = {
    // Dry           // Semiarid      // Moderate       // Wet // Very Wet
    {TERRAIN_DESERT, TERRAIN_DESERT, TERRAIN_FOREST, TERRAIN_FOREST,
     TERRAIN_FOREST}, // Hot (>30C)
    {TERRAIN_DESERT, TERRAIN_PLAINS, TERRAIN_FOREST, TERRAIN_FOREST,
     TERRAIN_FOREST}, // Warm (20-30C)
    {TERRAIN_DESERT, TERRAIN_PLAINS, TERRAIN_PLAINS, TERRAIN_FOREST,
     TERRAIN_FOREST}, // Temperate (10-20C)
    {TERRAIN_TUNDRA, TERRAIN_TUNDRA, TERRAIN_TUNDRA, TERRAIN_TUNDRA,
     TERRAIN_TUNDRA}, // Cold (0-10C)
    {TERRAIN_SNOW, TERRAIN_SNOW, TERRAIN_SNOW, TERRAIN_SNOW,
     TERRAIN_SNOW} // Freezing (<0C)
};

void biomes_update(World *w) {
  if (!w)
    return;

  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      WorldCell *cell = &w->cells[y][x];

      // Keep ocean as ocean
      if (w->geo.water[y][x]) {
        cell->terrain = TERRAIN_OCEAN;
        continue;
      }

      // Elevation overrides
      float elev = w->geo.elevation[y][x];
      if (elev > 2000.0f) {
        cell->terrain = TERRAIN_MOUNTAINS;
        continue;
      } else if (elev > 1000.0f) {
        cell->terrain = TERRAIN_HILLS;
        // Hills can have biomes, but for now strict type
        // In improved version, we might want HILL_FOREST etc.
        // By definition, this overrides biome classification.
        continue;
      }

      // 2D Lookup logic
      float temp = w->clim.temperature[y][x];
      // Normalize rainfall to 0..1 for lookup (let's assume max rain is
      // 2000mm?) Wait, original moisture was 0..1. Rainfall is mm/turn. Need a
      // heuristic. Say Max Rain = 10.0 per turn? Or just use relative.
      float rain = w->clim.rainfall[y][x];
      float moist = rain * 0.1f; // Quick norm
      if (moist > 1.0f)
        moist = 1.0f;

      // Map Temperature to index 0-4 (Hot to Cold)
      // Assume Temp range -10 to 40?
      // Table expectation:
      // Index 0: > 25
      // Index 1: 15 - 25
      // Index 2: 5 - 15
      // Index 3: -5 - 5
      // Index 4: < -5
      int temp_idx = 0;
      if (temp > 25.0f)
        temp_idx = 0;
      else if (temp > 15.0f)
        temp_idx = 1;
      else if (temp > 5.0f)
        temp_idx = 2;
      else if (temp > -5.0f)
        temp_idx = 3;
      else
        temp_idx = 4;

      // Map Moisture to index 0-4 (Dry to Wet) (0.0 to 1.0)
      int moist_idx = (int)(moist * 5.0f);
      if (moist_idx < 0)
        moist_idx = 0;
      if (moist_idx > 4)
        moist_idx = 4;

      cell->terrain = BIOME_TABLE[temp_idx][moist_idx];
    }
  }
}
