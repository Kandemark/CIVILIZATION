/**
 * @file legal_status.h
 * @brief State Supremacy and Organizational Legal Framework
 */

#ifndef CIVILIZATION_LEGAL_STATUS_H
#define CIVILIZATION_LEGAL_STATUS_H

#include "../../common.h"
#include "../../types.h"

/* Legal Tier */
typedef enum {
  CIV_LEGAL_NATIONAL = 0, /* State-sponsored/Official */
  CIV_LEGAL_TOLERATED,    /* Allowed but not supported */
  CIV_LEGAL_DISCOURAGED,  /* Legal but restricted */
  CIV_LEGAL_BANNED,       /* Illegal to belong/practice */
  CIV_LEGAL_TERRORIST     /* Enemy of the state, active persecution */
} civ_legal_tier_t;

/* Organization Status */
typedef struct {
  char entity_id[STRING_SHORT_LEN]; /* ID of Religion, Ideology, or Party */
  civ_legal_tier_t tier;
  civ_float_t persecution_intensity;
  char edict_history[4][STRING_MEDIUM_LEN];
  size_t edict_count;
} civ_entity_legal_status_t;

/* Functions */
civ_result_t civ_legal_set_status(civ_entity_legal_status_t *status,
                                  civ_legal_tier_t tier, const char *edict_msg);
civ_float_t
civ_legal_calculate_unrest_impact(const civ_entity_legal_status_t *status,
                                  civ_float_t follower_percentage);

#endif /* CIVILIZATION_LEGAL_STATUS_H */
