#include "core/economy/land_use.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_land_use_system_t *civ_land_use_create(civ_float_t total_area_km2) {
  civ_land_use_system_t *l = CIV_MALLOC(sizeof(civ_land_use_system_t));
  if (!l) return NULL;
  memset(l, 0, sizeof(*l));

  l->total_area_km2 = (total_area_km2 > 0) ? total_area_km2 : 10000.0;

  /* Initial land distribution */
  civ_float_t base_value[] = { 100.0, 5000.0, 50000.0, 30000.0, 20000.0, 200.0 };
  for (int i = 0; i < CIV_LAND_ZONE_COUNT; i++) {
    l->allocations[i].zone = (civ_land_zone_t)i;
    l->allocations[i].productivity = 0.60;
    l->allocations[i].value_per_km2 = base_value[i];
  }
  /* Most land starts undeveloped */
  l->allocations[CIV_LAND_UNDEVELOPED].area_km2 = l->total_area_km2 * 0.70;
  l->allocations[CIV_LAND_AGRICULTURAL].area_km2 = l->total_area_km2 * 0.20;
  l->allocations[CIV_LAND_RESIDENTIAL].area_km2 = l->total_area_km2 * 0.05;
  l->allocations[CIV_LAND_COMMERCIAL].area_km2 = l->total_area_km2 * 0.02;
  l->allocations[CIV_LAND_INDUSTRIAL].area_km2 = l->total_area_km2 * 0.02;
  l->allocations[CIV_LAND_CONSERVATION].area_km2 = l->total_area_km2 * 0.01;

  l->land_value_index = 1.0;
  return l;
}

void civ_land_use_destroy(civ_land_use_system_t *l) { free(l); }

void civ_land_use_update(civ_land_use_system_t *l, civ_float_t time_delta,
                         int population, civ_float_t gdp_per_capita,
                         civ_float_t urbanization_rate) {
  if (!l) return;
  (void)time_delta;

  l->population_density = (l->total_area_km2 > 0) ? population / l->total_area_km2 : 0.0;

  /* Development pressure: population growth relative to developed land */
  civ_float_t developed = 0.0;
  for (int i = 0; i < CIV_LAND_ZONE_COUNT; i++)
    if (l->allocations[i].zone != CIV_LAND_UNDEVELOPED
        && l->allocations[i].zone != CIV_LAND_CONSERVATION)
      developed += l->allocations[i].area_km2;

  l->development_pressure = (developed > 0)
    ? (civ_float_t)population / developed * 0.01 : 0.5;

  /* Auto-develop undeveloped land as population grows */
  if (l->development_pressure > 0.5 && l->allocations[CIV_LAND_UNDEVELOPED].area_km2 > 0) {
    civ_float_t convert = l->allocations[CIV_LAND_UNDEVELOPED].area_km2 * 0.002 * urbanization_rate;
    if (convert > 0) {
      /* Split conversion between residential, commercial, industrial */
      l->allocations[CIV_LAND_UNDEVELOPED].area_km2 -= convert;
      l->allocations[CIV_LAND_RESIDENTIAL].area_km2 += convert * 0.5;
      l->allocations[CIV_LAND_COMMERCIAL].area_km2 += convert * 0.3;
      l->allocations[CIV_LAND_INDUSTRIAL].area_km2 += convert * 0.2;
    }
  }

  /* Land values rise with GDP and density */
  for (int i = 0; i < CIV_LAND_ZONE_COUNT; i++) {
    if (l->allocations[i].zone == CIV_LAND_UNDEVELOPED) {
      l->allocations[i].value_per_km2 *= (1.0 + gdp_per_capita * 0.0001);
    } else {
      l->allocations[i].value_per_km2 *= (1.0 + gdp_per_capita * 0.0005
                                          + l->development_pressure * 0.02);
    }
  }

  /* Composite land value index */
  civ_float_t total_value = 0.0;
  for (int i = 0; i < CIV_LAND_ZONE_COUNT; i++)
    total_value += l->allocations[i].value_per_km2 * l->allocations[i].area_km2;
  l->land_value_index = total_value / (l->total_area_km2 * 1000.0 + 1.0);
}

void civ_land_use_rezone(civ_land_use_system_t *l, civ_land_zone_t from,
                         civ_land_zone_t to, civ_float_t amount_km2) {
  if (!l || from >= CIV_LAND_ZONE_COUNT || to >= CIV_LAND_ZONE_COUNT) return;
  if (amount_km2 > l->allocations[from].area_km2)
    amount_km2 = l->allocations[from].area_km2;
  l->allocations[from].area_km2 -= amount_km2;
  l->allocations[to].area_km2 += amount_km2;
}

civ_float_t civ_land_use_value_in(const civ_land_use_system_t *l, civ_land_zone_t zone) {
  if (!l || zone >= CIV_LAND_ZONE_COUNT) return 0.0;
  return l->allocations[zone].value_per_km2;
}

civ_float_t civ_land_use_available_for(const civ_land_use_system_t *l, civ_land_zone_t zone) {
  if (!l || zone >= CIV_LAND_ZONE_COUNT) return 0.0;
  civ_float_t used = l->allocations[zone].area_km2;
  civ_float_t max_zone = (zone == CIV_LAND_UNDEVELOPED || zone == CIV_LAND_AGRICULTURAL)
    ? l->total_area_km2 : l->total_area_km2 * 0.3;
  return (max_zone - used > 0) ? max_zone - used : 0.0;
}
