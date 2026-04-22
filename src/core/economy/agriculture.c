#include "core/economy/agriculture.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_agriculture_system_t *civ_agriculture_create(void) {
  civ_agriculture_system_t *a = CIV_MALLOC(sizeof(civ_agriculture_system_t));
  if (!a) return NULL;
  memset(a, 0, sizeof(*a));
  a->arable_land_km2   = 1000.0;
  a->soil_fertility    = 0.65;
  a->water_availability = 0.70;
  a->irrigation_level  = 0.10;
  a->fertilizer_tech   = 1.0;
  a->season            = CIV_SEASON_SPRING;
  return a;
}

void civ_agriculture_destroy(civ_agriculture_system_t *a) { free(a); }

void civ_agriculture_update(civ_agriculture_system_t *a, civ_float_t time_delta,
                            civ_float_t population, civ_float_t geography_arable_area,
                            civ_float_t tech_level, civ_float_t climate_modifier) {
  if (!a) return;

  /* Season cycle */
  a->season_progress += time_delta * 0.01; /* ~100 cycles per year */
  if (a->season_progress >= 1.0) {
    a->season_progress = 0.0;
    a->season = (a->season + 1) % 4;
  }

  /* Arable land from geography */
  if (geography_arable_area > 0) a->arable_land_km2 = geography_arable_area;

  /* Soil fertility: degrades with overuse, recovers with tech */
  civ_float_t intensity = population / (a->arable_land_km2 + 1.0);
  a->soil_fertility += (0.5 - intensity * 0.001 - a->soil_fertility * 0.05
                        + tech_level * 0.02) * time_delta * 0.5;
  if (a->soil_fertility < 0.10) a->soil_fertility = 0.10;
  if (a->soil_fertility > 1.00) a->soil_fertility = 1.00;

  /* Water and irrigation improve with tech */
  a->irrigation_level += (tech_level * 0.2 - a->irrigation_level) * time_delta * 0.3;

  /* Fertilizer tech ramps with overall tech */
  a->fertilizer_tech = 1.0 + tech_level * 0.5;

  /* Seasonal yield multiplier */
  civ_float_t season_mult[] = { 0.8, 1.2, 1.1, 0.3 };
  int s = (int)a->season;

  /* Crop yield: area * fertility * water * irrigation * fertilizer * season * climate */
  a->crop_yield = a->arable_land_km2 * a->soil_fertility * (0.5 + a->water_availability * 0.5)
                  * (1.0 + a->irrigation_level) * a->fertilizer_tech
                  * season_mult[s] * climate_modifier * 2.0;

  /* Livestock: fraction of arable land */
  a->livestock_output = a->arable_land_km2 * 0.3 * a->soil_fertility * season_mult[s] * 0.8;

  /* Total food (kcal equivalent from tons) */
  a->food_production = (a->crop_yield + a->livestock_output) * 800.0;

  /* Consumption: ~2500 kcal per person per day-equivalent */
  a->food_consumption = population * 2500.0 * 365.0;

  a->food_surplus = a->food_production - a->food_consumption;

  /* Famine risk: when production < 70% of needs */
  civ_float_t ratio = (a->food_consumption > 0) ? a->food_production / a->food_consumption : 1.0;
  a->famine_risk = (ratio < 0.7) ? (1.0 - ratio / 0.7) : 0.0;
}

civ_float_t civ_agriculture_food_security(const civ_agriculture_system_t *a) {
  if (!a || a->food_consumption <= 0) return 1.0;
  return (a->food_production / a->food_consumption) > 1.0 ? 1.0
         : a->food_production / a->food_consumption;
}

bool civ_agriculture_is_famine(const civ_agriculture_system_t *a) {
  return a ? (a->famine_risk > 0.5) : false;
}

civ_float_t civ_agriculture_surplus_export_capacity(const civ_agriculture_system_t *a) {
  if (!a || a->food_surplus <= 0) return 0.0;
  return a->food_surplus * 0.5; /* half of surplus available for trade */
}
