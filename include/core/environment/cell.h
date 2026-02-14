#ifndef CIVILIZATION_CELL_H
#define CIVILIZATION_CELL_H

typedef enum {
  TERRAIN_OCEAN,
  TERRAIN_DESERT,
  TERRAIN_PLAINS,
  TERRAIN_FOREST,
  TERRAIN_TUNDRA,
  TERRAIN_SNOW,
  TERRAIN_HILLS,
  TERRAIN_MOUNTAINS
} TerrainType;

typedef struct {
  TerrainType terrain;
  float tectonic_stress;
} WorldCell;

#endif /* CIVILIZATION_CELL_H */
