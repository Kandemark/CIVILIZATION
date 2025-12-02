/* geography.h - Terrain, elevation, water, erosion, desert formation */
#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H

#include "config.h"
#include "utils.h"

#define MAX_ELEVATION 3000.0f // meters
#define SEA_LEVEL 0.0f

// Forward declaration to avoid circular include
struct ClimateState;

typedef struct GeographyState {
  float elevation[WORLD_HEIGHT][WORLD_WIDTH];    // meters
  int water[WORLD_HEIGHT][WORLD_WIDTH];          // 0 = land, 1 = water
  int desert[WORLD_HEIGHT][WORLD_WIDTH];         // boolean flag for desert
  float river_volume[WORLD_HEIGHT][WORLD_WIDTH]; // flow volume, >0 means river
} GeographyState;

/* API */
void geography_initialize(GeographyState *geo, UtilsState *utils);
void geography_erosion(GeographyState *geo, UtilsState *utils);
void geography_update_deserts(GeographyState *geo);
void geography_update_rivers(GeographyState *geo,
                             const struct ClimateState *clim);

#endif // GEOGRAPHY_H
