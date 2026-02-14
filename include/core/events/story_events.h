/**
 * @file story_events.h
 * @brief Narrative early-game events: Spawning and Leadership
 */

#ifndef CIVILIZATION_STORY_EVENTS_H
#define CIVILIZATION_STORY_EVENTS_H

#include "../../common.h"
#include "../../types.h"
#include "../../ui/onboarding_engine.h"

/* Early Game States */
typedef enum {
  CIV_STORY_COMMUNITY_MEMBER = 0,
  CIV_STORY_ELECTION_PROCESS,
  CIV_STORY_OPPOSITION_ROLE,
  CIV_STORY_LEADER_DESIGNER,
  CIV_STORY_ESTABLISHED_POWER
} civ_story_state_t;

/* Community Context */
typedef struct {
  int64_t population;
  civ_float_t morale;
  civ_float_t political_influence; /* Player's power metric */
  char region_id[STRING_SHORT_LEN];
  char ai_leader_id[STRING_SHORT_LEN]; /* Set if player loses */
  civ_story_state_t state;
} civ_player_community_t;

/* Functions */
void civ_story_spawn_community(civ_player_community_t *community,
                               const char *region_id);
bool civ_story_trigger_rally(civ_player_community_t *community);
civ_result_t civ_story_election_outcome(civ_player_community_t *community,
                                        bool won);

#endif /* CIVILIZATION_STORY_EVENTS_H */
