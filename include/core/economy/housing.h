/**
 * @file housing.h
 * @brief Housing — property markets, urbanization, construction, affordability
 */
#ifndef CIV_ECONOMY_HOUSING_H
#define CIV_ECONOMY_HOUSING_H

#include "../../common.h"
#include "../../types.h"

typedef struct {
  int         total_units;
  int         occupied_units;
  int         vacant_units;
  int         under_construction;
  civ_float_t avg_price;
  civ_float_t avg_rent;
  civ_float_t price_to_income;     /* affordability ratio */
  civ_float_t homeownership_rate;  /* 0.0-1.0 */
  civ_float_t urbanization;        /* 0.0-1.0 fraction urban */
  civ_float_t construction_rate;   /* units per cycle */
  civ_float_t demolition_rate;     /* units per cycle */
  bool        housing_crisis;      /* true when affordability < 0.3 */
} civ_housing_system_t;

civ_housing_system_t *civ_housing_create(void);
void civ_housing_destroy(civ_housing_system_t *h);
void civ_housing_update(civ_housing_system_t *h, civ_float_t time_delta,
                        int population, civ_float_t avg_income,
                        civ_float_t interest_rate, civ_float_t construction_cost_index);

civ_float_t civ_housing_affordability(const civ_housing_system_t *h);
int civ_housing_units_needed(const civ_housing_system_t *h);
civ_float_t civ_housing_construction_demand(const civ_housing_system_t *h);

#endif
