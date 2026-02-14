#ifndef CIVILIZATION_CLIMATE_H
#define CIVILIZATION_CLIMATE_H

#include "../../common.h"
#include "../../utils/utils.h"

#define WORLD_WIDTH 256
#define WORLD_HEIGHT 256
#define MAX_TEMPERATURE 40.0f
#define MAX_ELEVATION 4000.0f
#define SEA_LEVEL 0.0f

typedef struct {
  float elevation[WORLD_HEIGHT][WORLD_WIDTH];
  bool water[WORLD_HEIGHT][WORLD_WIDTH];
  bool desert[WORLD_HEIGHT][WORLD_WIDTH];
  float river_volume[WORLD_HEIGHT][WORLD_WIDTH];
} GeographyState;

typedef struct {
  float temperature[WORLD_HEIGHT][WORLD_WIDTH];
  float pressure[WORLD_HEIGHT][WORLD_WIDTH];
  float wind_x[WORLD_HEIGHT][WORLD_WIDTH];
  float wind_y[WORLD_HEIGHT][WORLD_WIDTH];
  float rainfall[WORLD_HEIGHT][WORLD_WIDTH];
} ClimateState;

void climate_initialize(ClimateState *clim, UtilsState *utils);
void climate_update(ClimateState *clim, const GeographyState *geo,
                    UtilsState *utils);

#endif /* CIVILIZATION_CLIMATE_H */
