/**
 * @file game_events.h
 * @brief Game Event Triggers
 */

#ifndef CIVILIZATION_GAME_EVENTS_H
#define CIVILIZATION_GAME_EVENTS_H

#include "../environment/disaster_system.h"

struct civ_game;
typedef struct civ_game civ_game_t;

void civ_trigger_economic_crisis(civ_game_t *game);
void civ_trigger_natural_disaster(civ_game_t *game, civ_disaster_type_t type);

#endif /* CIVILIZATION_GAME_EVENTS_H */
