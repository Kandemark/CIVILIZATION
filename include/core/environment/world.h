#ifndef CIVILIZATION_WORLD_H
#define CIVILIZATION_WORLD_H

#include "cell.h"
#include "climate.h"


typedef struct {
  GeographyState geo;
  ClimateState clim;
  WorldCell cells[WORLD_HEIGHT][WORLD_WIDTH];
} World;

#endif /* CIVILIZATION_WORLD_H */
