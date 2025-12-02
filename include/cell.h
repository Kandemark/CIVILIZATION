/* cell.h - WorldCell definition */
#ifndef CELL_H
#define CELL_H

#include "config.h"

typedef enum {
  TERRAIN_OCEAN,
  TERRAIN_PLAINS,
  TERRAIN_HILLS,
  TERRAIN_MOUNTAINS,
  TERRAIN_DESERT,
  TERRAIN_FOREST,
  TERRAIN_TUNDRA,
  TERRAIN_SNOW
} TerrainType;

/* WorldCell holds per‑tile data */
typedef struct {
  TerrainType terrain;
  float elevation;     // meters
  float temperature;   // Celsius
  float moisture;      // 0..1
  int political_owner; // faction id, -1 = none
  float pressure;      // hPa
  float wind_x;        // wind vector components
  float wind_y;
  float tectonic_stress; // for earthquakes
  float river_volume;    // flow volume
} WorldCell;

#endif // CELL_H
