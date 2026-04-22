/**
 * @file agriculture.h
 * @brief Agriculture — food production, livestock, seasons, soil fertility, famine risk
 */
#ifndef CIV_ECONOMY_AGRICULTURE_H
#define CIV_ECONOMY_AGRICULTURE_H

#include "../../common.h"
#include "../../types.h"
#include "../simulation_engine/time_manager.h"

typedef struct {
  civ_float_t arable_land_km2;
  civ_float_t soil_fertility;        /* 0.0-1.0 */
  civ_float_t water_availability;    /* 0.0-1.0 */
  civ_float_t crop_yield;            /* tons per km2 */
  civ_float_t livestock_output;      /* meat/dairy units */
  civ_float_t food_production;       /* total kcal equivalent */
  civ_float_t food_consumption;      /* total kcal needed */
  civ_float_t food_surplus;          /* production - consumption */
  civ_float_t famine_risk;           /* 0.0-1.0 */
  civ_float_t irrigation_level;      /* 0.0-1.0 */
  civ_float_t fertilizer_tech;       /* technology multiplier */
  civ_season_t season;
  civ_float_t season_progress;       /* 0.0-1.0 through current season */
} civ_agriculture_system_t;

civ_agriculture_system_t *civ_agriculture_create(void);
void civ_agriculture_destroy(civ_agriculture_system_t *a);
void civ_agriculture_update(civ_agriculture_system_t *a, civ_float_t time_delta,
                            civ_float_t population, civ_float_t geography_arable_area,
                            civ_float_t tech_level, civ_float_t climate_modifier);

civ_float_t civ_agriculture_food_security(const civ_agriculture_system_t *a);
bool civ_agriculture_is_famine(const civ_agriculture_system_t *a);
civ_float_t civ_agriculture_surplus_export_capacity(const civ_agriculture_system_t *a);

#endif
