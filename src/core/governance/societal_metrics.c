/**
 * @file societal_metrics.c
 * @brief Implementation of unified societal health metrics
 */

#include "../../../include/core/governance/societal_metrics.h"
#include "../../../include/common.h"
#include <stdio.h>
#include <string.h>

civ_societal_health_t
civ_societal_calculate_health(const civ_governance_state_t *gov,
                              const civ_cultural_identity_t *culture,
                              const civ_ideology_t *ideology) {

  civ_societal_health_t health;
  memset(&health, 0, sizeof(civ_societal_health_t));

  if (!gov || !culture || !ideology)
    return health;

  health.stability_index = (civ_float_t)gov->stability;
  health.cohesion_index = culture->cohesion;
  health.corruption_index = (civ_float_t)gov->corruption;
  health.radicalization_index = ideology->radicalism;
  health.secularism_index = 0.5f; /* Default middle ground */
  health.vitality_index = 0.8f;   /* Base health value */
  health.economic_cohesion = 0.9f;
  health.international_repute = 0.5f;
  health.gdp_index = 1.0f;
  health.industrial_stability = 0.8f;

  /* Velocity is high if radicalism is high or stability is low */
  health.evolution_velocity =
      (health.radicalization_index + (1.0f - health.stability_index)) * 0.5f;

  /* Determine title pattern from gov */
  strncpy(health.dominant_title, gov->common_title_pattern,
          STRING_SHORT_LEN - 1);
  strncpy(health.national_identity_id, culture->id, STRING_SHORT_LEN - 1);

  return health;
}

void civ_societal_display_metrics(const civ_societal_health_t *health) {
  if (!health)
    return;

  printf("\n--- SOCIETAL HEALTH REPORT ---\n");
  printf("National Identity: %s\n", health->national_identity_id);
  printf("Government Title:  %s\n", health->dominant_title);
  printf("Stability:         %.2f\n", health->stability_index);
  printf("Cultural Cohesion: %.2f\n", health->cohesion_index);
  printf("Corruption Index:  %.2f\n", health->corruption_index);
  printf("Radicalization:    %.2f\n", health->radicalization_index);
  printf("Secularism Index:  %.2f\n", health->secularism_index);
  printf("Vitality Index:    %.2f\n", health->vitality_index);
  printf("Economic Cohesion: %.2f\n", health->economic_cohesion);
  printf("GDP Index:         %.2f\n", health->gdp_index);
  printf("Indust. Stability: %.2f\n", health->industrial_stability);
  printf("Diplomatic Repute: %.2f\n", health->international_repute);
  printf("Evolution Speed:   %.2f\n", health->evolution_velocity);
  printf("------------------------------\n");
}

civ_float_t civ_societal_update_legal_impact(civ_societal_health_t *health,
                                             const char *entity_id, int tier) {
  if (!health || !entity_id)
    return 0.0f;

  /* Banning or declaring terrorist organizations causes immediate
     radicalization spike */
  if (tier >= 3) { /* BANNED=3, TERRORIST=4 */
    health->radicalization_index =
        CLAMP(health->radicalization_index + 0.15f, 0.0f, 1.0f);
    health->stability_index = CLAMP(health->stability_index - 0.1f, 0.0f, 1.5f);
  }

  return health->radicalization_index;
}
