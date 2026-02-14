/**
 * @file population_vitality.c
 * @brief Implementation of population health and education
 */

#include "../../../include/core/population/population_vitality.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>


void civ_vitality_init(civ_population_vitality_t *vitality) {
  if (!vitality)
    return;

  memset(vitality, 0, sizeof(civ_population_vitality_t));
  vitality->general_health = 0.8f;
  vitality->education_level = 0.2f;  /* Starts low */
  vitality->life_expectancy = 40.0f; /* Early game base */
  vitality->literacy_rate = 0.05f;
}

void civ_vitality_update(civ_population_vitality_t *vitality,
                         civ_float_t time_delta) {
  if (!vitality)
    return;

  /* Education grows slowly over time */
  vitality->education_level += 0.001f * time_delta;
  vitality->education_level = CLAMP(vitality->education_level, 0.0f, 1.0f);

  /* Outbreak decay or spread */
  if (vitality->outbreak_present) {
    vitality->general_health -=
        vitality->active_outbreak.severity * 0.05f * time_delta;

    /* Outbreak eventually ends */
    if (rand() % 100 < 5) {
      vitality->outbreak_present = false;
      civ_log(CIV_LOG_INFO, "Outbreak of %s has ended.",
              vitality->active_outbreak.name);
    }
  } else {
    /* General health slowly recovers */
    vitality->general_health += 0.01f * time_delta;
  }

  vitality->general_health = CLAMP(vitality->general_health, 0.05f, 1.0f);
}

civ_result_t civ_vitality_trigger_outbreak(civ_population_vitality_t *vitality,
                                           const char *disease_name,
                                           civ_float_t lethality) {
  if (!vitality || !disease_name)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  vitality->outbreak_present = true;
  strncpy(vitality->active_outbreak.name, disease_name, STRING_SHORT_LEN - 1);
  vitality->active_outbreak.lethality = lethality;
  vitality->active_outbreak.severity = lethality * 2.0f;
  vitality->active_outbreak.is_active = true;

  civ_log(CIV_LOG_WARNING, "DISEASE OUTBREAK: %s has appeared!", disease_name);

  return (civ_result_t){CIV_OK, NULL};
}

civ_float_t civ_vitality_calculate_mortality_mod(
    const civ_population_vitality_t *vitality) {
  if (!vitality)
    return 1.0f;

  civ_float_t mod = 1.0f;

  /* Poor health increases mortality */
  if (vitality->general_health < 0.5f) {
    mod += (0.5f - vitality->general_health) * 2.0f;
  }

  /* Active outbreak is catastrophic */
  if (vitality->outbreak_present) {
    mod *= (1.0f + vitality->active_outbreak.lethality);
  }

  return mod;
}
