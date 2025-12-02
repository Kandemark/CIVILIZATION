/* climate.h - Temperature, wind, pressure, rainfall simulation */
#ifndef CLIMATE_H
#define CLIMATE_H

#include "config.h"
#include "geography.h"
#include "utils.h"

#define MAX_TEMPERATURE 40.0f // Celsius
#define MIN_TEMPERATURE -30.0f

typedef struct ClimateState {
  float temperature[WORLD_HEIGHT][WORLD_WIDTH]; // Celsius
  float pressure[WORLD_HEIGHT][WORLD_WIDTH];    // hPa
  float wind_x[WORLD_HEIGHT][WORLD_WIDTH];      // wind vector components
  float wind_y[WORLD_HEIGHT][WORLD_WIDTH];
  float rainfall[WORLD_HEIGHT][WORLD_WIDTH]; // mm per turn
} ClimateState;

/* API */
void climate_initialize(ClimateState *clim, UtilsState *utils);
void climate_update(ClimateState *clim, const GeographyState *geo,
                    UtilsState *utils);

#endif // CLIMATE_H
