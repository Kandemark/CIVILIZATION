
/* world.c - Core world management */
#include "world.h"
#include "biomes.h"
#include <stdio.h>
#include <stdlib.h>

World *world_create(void) {
  World *w = (World *)malloc(sizeof(World));
  if (!w) {
    fprintf(stderr, "[ERROR] Failed to allocate World\n");
    return NULL;
  }
  // Initialize utils with a deterministic seed
  utils_init(&w->utils, 20251129u);
  // Zero out cells
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      WorldCell *cell = &w->cells[y][x];
      cell->terrain = TERRAIN_OCEAN;
      cell->elevation = 0.0f;
      cell->temperature = 0.0f;
      cell->moisture = 0.0f;
      cell->political_owner = -1;
      cell->pressure = 1013.0f;
      cell->wind_x = 0.0f;
      cell->wind_y = 0.0f;
      cell->tectonic_stress = 0.0f;
    }
  }
  // Initialize subsystem states
  geography_initialize(&w->geo, &w->utils);
  climate_initialize(&w->clim, &w->utils);
  politics_initialize(&w->pol, &w->utils);
  events_initialize(&w->ev);
  return w;
}

void world_destroy(World *w) {
  if (w) {
    // No dynamic allocations inside subsystems for now
    free(w);
  }
}

void world_initialize(World *w) {
  if (!w)
    return;
  // Run initial erosion and desert formation
  geography_erosion(&w->geo, &w->utils);
  geography_update_deserts(&w->geo);
  // Initial climate based on geography
  climate_update(&w->clim, &w->geo, &w->utils);
  // Rivers flow based on rainfall
  geography_update_rivers(&w->geo, &w->clim);
  // Calculate biomes
  biomes_update(w);
  // Initial political ownership (none)
  politics_update(&w->pol, &w->geo, &w->clim);
}

void world_update(World *w) {
  if (!w)
    return;
  // 1. Update geography (erosion, water flow, desert formation)
  geography_erosion(&w->geo, &w->utils);
  geography_update_deserts(&w->geo);
  // 2. Update climate based on new geography
  climate_update(&w->clim, &w->geo, &w->utils);
  // 2b. Update rivers based on new climate
  geography_update_rivers(&w->geo, &w->clim);
  // 2c. Update biomes
  biomes_update(w);
  // 3. Update events (earthquakes, storms, etc.)
  events_update(&w->ev, w->cells, &w->utils);
  // 4. Update political map (could be expanded later)
  // Check if we need to init/assign land
  politics_update(&w->pol, &w->geo, &w->clim);
  // Evolve governments
  politics_tick(&w->pol, 1.0f);
  // 5. Sync cell data from subsystem states
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      WorldCell *cell = &w->cells[y][x];
      // Terrain from geography
      float elev = w->geo.elevation[y][x];
      cell->elevation = elev;
      cell->terrain =
          (w->geo.water[y][x])
              ? TERRAIN_OCEAN
              : (w->geo.desert[y][x]
                     ? TERRAIN_DESERT
                     : (elev > 2000.0f ? TERRAIN_MOUNTAINS
                                       : (elev > 1000.0f ? TERRAIN_HILLS
                                                         : TERRAIN_PLAINS)));
      // Climate values
      cell->temperature = w->clim.temperature[y][x];
      cell->pressure = w->clim.pressure[y][x];
      cell->wind_x = w->clim.wind_x[y][x];
      cell->wind_y = w->clim.wind_y[y][x];
      // River volume
      cell->river_volume = w->geo.river_volume[y][x];
      // Political owner from politics state
      cell->political_owner = w->pol.ownership[y][x];
    }
  }
}
