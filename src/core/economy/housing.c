#include "core/economy/housing.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>

civ_housing_system_t *civ_housing_create(void) {
  civ_housing_system_t *h = CIV_MALLOC(sizeof(civ_housing_system_t));
  if (!h) return NULL;
  memset(h, 0, sizeof(*h));
  h->total_units = 10000;
  h->occupied_units = 9200;
  h->vacant_units = h->total_units - h->occupied_units;
  h->avg_price = 250000.0;
  h->avg_rent = 1200.0;
  h->price_to_income = 5.5;
  h->homeownership_rate = 0.65;
  h->urbanization = 0.55;
  h->construction_rate = 50.0;
  return h;
}

void civ_housing_destroy(civ_housing_system_t *h) { free(h); }

void civ_housing_update(civ_housing_system_t *h, civ_float_t time_delta,
                        int population, civ_float_t avg_income,
                        civ_float_t interest_rate, civ_float_t construction_cost_index) {
  if (!h) return;
  (void)time_delta;

  /* Housing units needed: ~2.5 people per unit */
  int units_needed = population / 3;
  int shortage = units_needed - h->total_units;

  /* Construction responds to shortage and interest rates */
  civ_float_t build_demand = (shortage > 0) ? (civ_float_t)shortage * 0.1 : 0.0;
  /* High interest rates suppress construction */
  build_demand *= (1.0 - interest_rate * 2.0);
  /* Construction cost affects rate */
  build_demand /= (construction_cost_index + 0.5);
  if (build_demand < 0) build_demand = 0;

  h->construction_rate = build_demand;
  h->under_construction = (int)build_demand;

  /* Natural demolition (~2% per cycle) */
  h->demolition_rate = h->total_units * 0.02;
  h->total_units += (int)build_demand - (int)h->demolition_rate;
  if (h->total_units < 100) h->total_units = 100;

  /* Occupancy */
  h->occupied_units = (units_needed < h->total_units) ? units_needed : h->total_units;
  h->vacant_units = h->total_units - h->occupied_units;

  /* Price dynamics: supply/demand */
  civ_float_t vacancy_rate = (h->total_units > 0) ? (civ_float_t)h->vacant_units / h->total_units : 0.0;
  civ_float_t price_pressure = (vacancy_rate < 0.05) ? 0.05 : (vacancy_rate > 0.15 ? -0.02 : 0.0);

  h->avg_price *= (1.0 + price_pressure + interest_rate * 0.3);
  h->avg_rent = h->avg_price * 0.005; /* ~0.5% monthly rent-to-price */

  /* Price-to-income */
  h->price_to_income = (avg_income > 0) ? (h->avg_price / avg_income) : 10.0;

  /* Homeownership: falls when unaffordable, rises with low rates */
  civ_float_t affordability = (h->price_to_income < 3.0) ? 1.0
    : (h->price_to_income > 8.0) ? 0.0 : 1.0 - (h->price_to_income - 3.0) / 5.0;
  h->homeownership_rate += (affordability - h->homeownership_rate) * 0.05;

  /* Urbanization: rises with population density pressure */
  h->urbanization += 0.001;
  if (h->urbanization > 0.95) h->urbanization = 0.95;

  /* Housing crisis: affordability < 0.3 AND vacancy < 3% */
  h->housing_crisis = (affordability < 0.3 && vacancy_rate < 0.03);
}

civ_float_t civ_housing_affordability(const civ_housing_system_t *h) {
  if (!h) return 0.0;
  return (h->price_to_income < 3.0) ? 1.0
    : (h->price_to_income > 10.0) ? 0.0 : 1.0 - (h->price_to_income - 3.0) / 7.0;
}

int civ_housing_units_needed(const civ_housing_system_t *h) {
  if (!h) return 0;
  return (h->total_units - h->occupied_units < 0) ? (h->occupied_units - h->total_units) : 0;
}

civ_float_t civ_housing_construction_demand(const civ_housing_system_t *h) {
  return h ? h->construction_rate : 0.0;
}
