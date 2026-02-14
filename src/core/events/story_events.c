/**
 * @file story_events.c
 * @brief Implementation of early-game narrative triggers
 */

#include "../../../include/core/events/story_events.h"
#include "../../../include/common.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void civ_story_spawn_community(civ_player_community_t *community,
                               const char *region_id) {
  if (!community || !region_id)
    return;

  community->population =
      1000 + (rand() % 500); /* Random starting pop around 1000 */
  community->morale = 0.7f;
  strncpy(community->region_id, region_id, STRING_SHORT_LEN - 1);
  community->state = CIV_STORY_COMMUNITY_MEMBER;

  civ_log(CIV_LOG_INFO, "Community spawned in %s with %lld inhabitants.",
          region_id, community->population);
}

bool civ_story_trigger_rally(civ_player_community_t *community) {
  if (!community)
    return false;

  printf("\n--- THE GREAT RALLY ---\n");
  printf("The community gathers under your banner. Their survival depends on "
         "leadership.\n");

  /* Transition to election process */
  community->state = CIV_STORY_ELECTION_PROCESS;
  return true;
}

civ_result_t civ_story_election_outcome(civ_player_community_t *community,
                                        bool won) {
  if (!community)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null community"};

  if (won) {
    community->state = CIV_STORY_LEADER_DESIGNER;
    community->morale += 0.2f;
    printf("You have been elected LEADER!\n");
  } else {
    community->morale -= 0.1f;
    printf("The community chose another. Survival continues as a member.\n");
  }

  return (civ_result_t){CIV_OK, NULL};
}
