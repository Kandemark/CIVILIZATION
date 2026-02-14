/**
 * @file societal_metrics.h
 * @brief Unified Societal Health and Evolution Metrics
 */

#ifndef CIVILIZATION_SOCIETAL_METRICS_H
#define CIVILIZATION_SOCIETAL_METRICS_H

#include "../../common.h"
#include "../../types.h"
#include "../culture/cultural_identity.h"
#include "../culture/ideology_system.h"
#include "../governance_evolution.h"

/* Societal Health State */
typedef struct {
  civ_float_t stability_index;
  civ_float_t cohesion_index;
  civ_float_t corruption_index;
  civ_float_t radicalization_index;

  civ_float_t evolution_velocity;   /* Rate of societal change */
  civ_float_t secularism_index;     /* 0.0 (Theocracy) to 1.0 (State Atheism) */
  civ_float_t vitality_index;       /* Combined population health/education */
  civ_float_t economic_cohesion;    /* Inflation/Market stability */
  civ_float_t international_repute; /* Trust/Diplomatic standing */
  civ_float_t gdp_index;            /* Economic output */
  civ_float_t industrial_stability; /* Systemic resilience */

  char dominant_title[STRING_SHORT_LEN];
  char national_identity_id[STRING_SHORT_LEN];
} civ_societal_health_t;

/* Functions */
civ_societal_health_t
civ_societal_calculate_health(const civ_governance_state_t *gov,
                              const civ_cultural_identity_t *culture,
                              const civ_ideology_t *ideology);

void civ_societal_display_metrics(const civ_societal_health_t *health);

/* Legal Integration */
civ_float_t civ_societal_update_legal_impact(civ_societal_health_t *health,
                                             const char *entity_id, int tier);

#endif /* CIVILIZATION_SOCIETAL_METRICS_H */
