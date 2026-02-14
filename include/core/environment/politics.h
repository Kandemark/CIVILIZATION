#ifndef CIVILIZATION_POLITICS_H
#define CIVILIZATION_POLITICS_H

#include "../../utils/utils.h"
#include "../culture/culture.h"
#include "../governance/custom_governance.h"
#include "world.h"

#define MAX_FACTIONS 16

typedef enum {
  RELATION_WAR,
  RELATION_HOSTILE,
  RELATION_NEUTRAL,
  RELATION_FRIENDLY,
  RELATION_ALLY
} RelationshipType;

typedef struct {
  int id;
  char name[64];
  unsigned char color[3];
  float gold;
  float military_strength;
  civ_custom_governance_t *government;
} Faction;

typedef struct {
  Faction factions[MAX_FACTIONS];
  RelationshipType diplomacy[MAX_FACTIONS][MAX_FACTIONS];
  int ownership[WORLD_HEIGHT][WORLD_WIDTH];
} PoliticsState;

void politics_initialize(PoliticsState *pol, UtilsState *utils);
void politics_update(PoliticsState *pol, const GeographyState *geo,
                     const ClimateState *clim);
void politics_tick(PoliticsState *pol, float time_delta);
void politics_update_diplomacy(PoliticsState *pol);
void politics_set_relationship(PoliticsState *pol, int faction_a, int faction_b,
                               RelationshipType type);
RelationshipType politics_get_relationship(PoliticsState *pol, int faction_a,
                                           int faction_b);

#endif /* CIVILIZATION_POLITICS_H */
