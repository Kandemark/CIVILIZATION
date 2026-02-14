/**
 * @file population_vitality.h
 * @brief Population Vitality: Health, Disease, and Education
 */

#ifndef CIVILIZATION_POPULATION_VITALITY_H
#define CIVILIZATION_POPULATION_VITALITY_H

#include "../../common.h"
#include "../../types.h"

/* Disease Unit */
typedef struct {
  char name[STRING_SHORT_LEN];
  civ_float_t contagion_rate;
  civ_float_t lethality;
  civ_float_t severity; /* Affects productivity */
  bool is_active;
} civ_disease_t;

/* Population Health State */
typedef struct {
  civ_float_t general_health;  /* 0.0 to 1.0 */
  civ_float_t education_level; /* 0.0 to 1.0 */

  civ_disease_t active_outbreak;
  bool outbreak_present;

  civ_float_t life_expectancy;
  civ_float_t literacy_rate;
} civ_population_vitality_t;

/* Functions */
void civ_vitality_init(civ_population_vitality_t *vitality);
void civ_vitality_update(civ_population_vitality_t *vitality,
                         civ_float_t time_delta);

civ_result_t civ_vitality_trigger_outbreak(civ_population_vitality_t *vitality,
                                           const char *disease_name,
                                           civ_float_t lethality);
civ_float_t
civ_vitality_calculate_mortality_mod(const civ_population_vitality_t *vitality);

#endif /* CIVILIZATION_POPULATION_VITALITY_H */
