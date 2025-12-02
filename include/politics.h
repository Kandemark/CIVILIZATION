/* politics.h - Faction data, region borders, political map colors */
#ifndef POLITICS_H
#define POLITICS_H

#include "config.h"
#include "geography.h"
#include "utils.h"


#define MAX_FACTIONS 8

typedef struct {
  int id;
  unsigned char color[3]; // RGB for map rendering
  char name[32];
} Faction;

typedef struct {
  Faction factions[MAX_FACTIONS];
  int ownership[WORLD_HEIGHT][WORLD_WIDTH]; // faction id per cell, -1 = none
} PoliticsState;

/* API */
void politics_initialize(PoliticsState *pol, UtilsState *utils);
void politics_update(PoliticsState *pol, const GeographyState *geo);

#endif // POLITICS_H
