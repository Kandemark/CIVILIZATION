#include "core/economy/energy.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CIV_ENERGY_INITIAL_PLANT_CAP 8

civ_energy_system_t *civ_energy_create(void) {
  civ_energy_system_t *e = CIV_MALLOC(sizeof(civ_energy_system_t));
  if (!e) return NULL;
  memset(e, 0, sizeof(*e));
  e->plant_capacity = CIV_ENERGY_INITIAL_PLANT_CAP;
  e->plants = CIV_MALLOC(sizeof(civ_power_plant_t) * e->plant_capacity);
  e->grid_reliability = 0.85;
  e->energy_price = 50.0; /* per MWh */
  e->energy_independence = 0.90;
  return e;
}

void civ_energy_destroy(civ_energy_system_t *e) {
  if (!e) return;
  free(e->plants);
  free(e);
}

void civ_energy_update(civ_energy_system_t *e, civ_float_t time_delta,
                       civ_float_t population, civ_float_t industrial_output,
                       civ_float_t tech_level, civ_float_t fuel_availability) {
  if (!e) return;
  (void)time_delta;

  /* Demand: per-capita baseload + industrial */
  e->total_demand_mw = population * 0.001 + industrial_output * 0.01;

  /* Supply */
  e->total_capacity_mw = 0.0;
  e->renewable_fraction = 0.0;
  civ_float_t renewable_capacity = 0.0;

  for (int i = 0; i < e->plant_count; i++) {
    civ_power_plant_t *p = &e->plants[i];

    /* Efficiency improves with tech */
    p->efficiency += (0.4 + tech_level * 0.4 - p->efficiency) * 0.05;

    /* Non-renewable output limited by fuel */
    civ_float_t fuel_mult = p->is_renewable ? 1.0 : fuel_availability;

    p->current_output_mw = p->capacity_mw * p->efficiency * fuel_mult;
    e->total_capacity_mw += p->capacity_mw;

    if (p->is_renewable) renewable_capacity += p->capacity_mw;
  }

  e->renewable_fraction = (e->total_capacity_mw > 0)
    ? renewable_capacity / e->total_capacity_mw : 0.0;

  /* Blackout risk: demand > supply * reliability */
  civ_float_t effective_supply = e->total_capacity_mw * e->grid_reliability;
  e->blackout = (e->total_demand_mw > effective_supply);

  /* Energy price: supply/demand balance */
  civ_float_t balance = (e->total_demand_mw > 0)
    ? effective_supply / e->total_demand_mw : 2.0;
  civ_float_t target_price = 100.0 / (balance + 0.2);
  /* Renewables cheaper */
  target_price *= (1.0 - e->renewable_fraction * 0.3);
  /* Tech reduces cost */
  target_price /= (1.0 + tech_level * 0.3);
  e->energy_price += (target_price - e->energy_price) * 0.1;

  /* Grid reliability: degrades without investment, tech improves */
  e->grid_reliability += (tech_level * 0.1 - (1.0 - e->grid_reliability) * 0.02) * 0.5;
  if (e->grid_reliability < 0.20) e->grid_reliability = 0.20;
  if (e->grid_reliability > 0.99) e->grid_reliability = 0.99;

  /* Energy independence */
  e->energy_independence = (e->total_capacity_mw > 0)
    ? 1.0 - (1.0 - fuel_availability) * 0.5 : 0.0;
}

civ_power_plant_t *civ_energy_add_plant(civ_energy_system_t *e, civ_energy_source_t source,
                                        civ_float_t capacity, civ_float_t efficiency) {
  if (!e) return NULL;
  if (e->plant_count >= e->plant_capacity) {
    int nc = e->plant_capacity * 2;
    civ_power_plant_t *tmp = CIV_REALLOC(e->plants, sizeof(civ_power_plant_t) * nc);
    if (!tmp) return NULL;
    e->plants = tmp;
    e->plant_capacity = nc;
  }
  civ_power_plant_t *p = &e->plants[e->plant_count];
  memset(p, 0, sizeof(*p));
  p->source = source;
  p->capacity_mw = capacity;
  p->efficiency = efficiency;
  p->is_renewable = (source >= CIV_ENERGY_SOLAR);
  p->cost_per_mwh = p->is_renewable ? 40.0 : 60.0;
  p->carbon_intensity = p->is_renewable ? 0.01 : 0.8;
  e->plant_count++;
  return p;
}

civ_float_t civ_energy_surplus(const civ_energy_system_t *e) {
  if (!e) return 0.0;
  return e->total_capacity_mw - e->total_demand_mw;
}

civ_float_t civ_energy_cost_burden(const civ_energy_system_t *e, civ_float_t gdp) {
  if (!e || gdp <= 0) return 0.0;
  return (e->energy_price * e->total_demand_mw) / gdp;
}
