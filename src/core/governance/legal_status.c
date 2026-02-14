/**
 * @file legal_status.c
 * @brief Implementation of state organizational legal framework
 */

#include "../../../include/core/governance/legal_status.h"
#include "../../../include/common.h"
#include <string.h>

civ_result_t civ_legal_set_status(civ_entity_legal_status_t *status,
                                  civ_legal_tier_t tier,
                                  const char *edict_msg) {
  if (!status)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null status"};

  status->tier = tier;

  /* Persecution increases if banned or terrorist */
  if (tier == CIV_LEGAL_BANNED)
    status->persecution_intensity = 0.5f;
  else if (tier == CIV_LEGAL_TERRORIST)
    status->persecution_intensity = 1.0f;
  else
    status->persecution_intensity = 0.0f;

  if (edict_msg && status->edict_count < 4) {
    strncpy(status->edict_history[status->edict_count++], edict_msg,
            STRING_MEDIUM_LEN - 1);
  }

  return (civ_result_t){CIV_OK, NULL};
}

civ_float_t
civ_legal_calculate_unrest_impact(const civ_entity_legal_status_t *status,
                                  civ_float_t follower_percentage) {
  if (!status)
    return 0.0f;

  /* High impact if many followers are persecuted/banned */
  if (status->tier >= CIV_LEGAL_DISCOURAGED) {
    return follower_percentage * status->persecution_intensity;
  }

  return 0.0f;
}
