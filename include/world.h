/* world.h - Core world data structures and API */
#ifndef WORLD_H
#define WORLD_H

#include "cell.h"
#include "climate.h"
#include "events.h"
#include "geography.h"
#include "politics.h"
#include "utils.h"


typedef struct {
  int x;
  int y;
} Coord;

/* Aggregated world state */
typedef struct {
  WorldCell cells[WORLD_HEIGHT][WORLD_WIDTH];
  GeographyState geo;
  ClimateState clim;
  PoliticsState pol;
  EventsState ev;
  UtilsState utils; // optional helper state
} World;

/* API */
World *world_create(void);
void world_destroy(World *w);
void world_initialize(World *w);
void world_update(World *w); // runs one simulation turn

#endif // WORLD_H
