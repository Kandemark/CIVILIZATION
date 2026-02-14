/**
 * @file political_rivalry.c
 * @brief Implementation of power struggles and opposition management
 */

#include "../../../include/core/politics/political_rivalry.h"
#include "../../../include/common.h"
#include <string.h>

void civ_rivalry_init_rival(civ_political_rival_t *rival, const char *name) {
  if (!rival || !name)
    return;

  memset(rival, 0, sizeof(civ_political_rival_t));
  strncpy(rival->name, name, STRING_SHORT_LEN - 1);
  rival->popularity = 0.4f; /* Start with baseline suspicion */
  rival->influence = 100.0f;
  rival->radicalism = 0.1f;
  rival->is_active = true;
}

civ_result_t
civ_rivalry_perform_opposition_action(civ_political_rival_t *player,
                                      civ_opposition_action_t action) {
  if (!player)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  switch (action) {
  case CIV_OPP_CHARITY:
    player->popularity += 0.05f;
    player->influence -= 20.0f;
    civ_log(CIV_LOG_INFO, "Performed charitable works. Popularity increased.");
    break;
  case CIV_OPP_PROPAGANDA:
    player->popularity += 0.02f;
    player->radicalism += 0.05f;
    civ_log(CIV_LOG_INFO, "Spread dissent. Radicalization is rising.");
    break;
  case CIV_OPP_COUP_ATTEMPT:
    if (player->popularity > 0.7f && player->influence > 500.0f) {
      civ_log(CIV_LOG_WARNING, "A COOPERATIVE HAS BEGUN!");
      return (civ_result_t){CIV_OK, "COUP_SUCCESS"};
    } else {
      civ_log(CIV_LOG_ERROR, "Coup failed due to low support.");
      player->popularity = 0.0f;
      return (civ_result_t){CIV_ERROR_INVALID_STATE, "COUP_FAILURE"};
    }
  default:
    break;
  }

  player->popularity = CLAMP(player->popularity, 0.0f, 1.0f);
  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_rivalry_apply_suppression(civ_suppression_level_t level,
                                           civ_political_rival_t *target) {
  if (!target)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null target"};

  civ_log(CIV_LOG_WARNING, "Leader applied suppression level %d to %s",
          (int)level, target->name);

  switch (level) {
  case CIV_SUP_SURVEILLANCE:
    target->influence *= 0.9f;
    break;
  case CIV_SUP_ARRESTS:
    target->influence *= 0.5f;
    target->is_active = (rand() % 100 > 30);
    break;
  case CIV_SUP_ELIMINATION:
    target->is_active = false;
    civ_log(CIV_LOG_WARNING, "%s has been eliminated from political life.",
            target->name);
    break;
  default:
    break;
  }

  return (civ_result_t){CIV_OK, NULL};
}
