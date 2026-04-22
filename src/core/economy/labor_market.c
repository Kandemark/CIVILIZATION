#include "core/economy/labor_market.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_labor_market_t *civ_labor_market_create(void) {
  civ_labor_market_t *l = CIV_MALLOC(sizeof(civ_labor_market_t));
  if (!l) return NULL;
  memset(l, 0, sizeof(*l));
  l->labor_force_participation = 0.62;
  l->avg_wage_national = 30000.0;
  l->wage_inequality = 0.38;
  l->productivity_per_worker = 50000.0;

  /* Initialize tiers with sensible distributions */
  civ_float_t tier_share[] = { 0.30, 0.40, 0.22, 0.08 };
  civ_float_t tier_wage[]   = { 15000.0, 28000.0, 55000.0, 120000.0 };
  for (int i = 0; i < CIV_LABOR_TIER_COUNT; i++) {
    l->segments[i].tier = (civ_labor_tier_t)i;
    l->segments[i].avg_wage = tier_wage[i];
    l->segments[i].unemployment_rate = 0.06 - i * 0.015;
  }
  return l;
}

void civ_labor_market_destroy(civ_labor_market_t *l) { free(l); }

void civ_labor_market_update(civ_labor_market_t *l, civ_float_t time_delta,
                             int total_population, civ_float_t gdp,
                             civ_float_t tech_level, civ_float_t education_level,
                             civ_float_t business_confidence) {
  if (!l) return;
  (void)time_delta;

  l->total_workforce = (int)(total_population * l->labor_force_participation);

  /* Distribute workforce across tiers based on education */
  civ_float_t tier_dist[CIV_LABOR_TIER_COUNT];
  tier_dist[CIV_LABOR_UNSKILLED]   = 0.30 * (1.0 - education_level * 0.5);
  tier_dist[CIV_LABOR_SKILLED]     = 0.40;
  tier_dist[CIV_LABOR_PROFESSIONAL] = 0.22 * (1.0 + education_level * 0.5);
  tier_dist[CIV_LABOR_EXECUTIVE]   = 0.08 * (1.0 + education_level * 0.3);

  /* Normalize */
  civ_float_t sum = 0.0;
  for (int i = 0; i < CIV_LABOR_TIER_COUNT; i++) sum += tier_dist[i];
  if (sum > 0) for (int i = 0; i < CIV_LABOR_TIER_COUNT; i++) tier_dist[i] /= sum;

  l->total_employed = 0;
  l->total_unemployed = 0;

  civ_float_t gdp_per_worker = (l->total_workforce > 0) ? gdp / l->total_workforce : 50000.0;

  for (int i = 0; i < CIV_LABOR_TIER_COUNT; i++) {
    civ_labor_segment_t *s = &l->segments[i];
    s->workforce = (int)(l->total_workforce * tier_dist[i]);

    /* Unemployment within tier: base + confidence + tech displacement */
    civ_float_t base_unemp = 0.05 - i * 0.015;
    civ_float_t tech_displacement = tech_level * 0.04; /* automation affects lower tiers more */
    if (i <= CIV_LABOR_SKILLED) tech_displacement *= (1.5 - i * 0.5);
    s->unemployment_rate = base_unemp + (1.0 - business_confidence) * 0.05 + tech_displacement;
    if (s->unemployment_rate < 0.01) s->unemployment_rate = 0.01;
    if (s->unemployment_rate > 0.35) s->unemployment_rate = 0.35;

    s->employed = (int)(s->workforce * (1.0 - s->unemployment_rate));
    l->total_employed += s->employed;
    l->total_unemployed += (s->workforce - s->employed);

    /* Wage growth: productivity growth + tier demand */
    civ_float_t labor_demand = (business_confidence - 0.5) * 0.1;
    s->wage_growth = (gdp_per_worker / (s->avg_wage + 1.0) - 1.0) * 0.1 + labor_demand;
    s->avg_wage *= (1.0 + s->wage_growth);
    if (s->avg_wage < 5000.0) s->avg_wage = 5000.0;
  }

  l->overall_unemployment = (l->total_workforce > 0)
    ? (civ_float_t)l->total_unemployed / l->total_workforce : 0.0;

  /* National average wage */
  l->avg_wage_national = 0.0;
  for (int i = 0; i < CIV_LABOR_TIER_COUNT; i++)
    l->avg_wage_national += l->segments[i].avg_wage * l->segments[i].workforce;
  l->avg_wage_national /= (l->total_workforce + 1);

  /* Productivity */
  l->productivity_per_worker = gdp_per_worker;
}

civ_float_t civ_labor_wage_for_tier(const civ_labor_market_t *l, civ_labor_tier_t tier) {
  if (!l || tier >= CIV_LABOR_TIER_COUNT) return 0.0;
  return l->segments[tier].avg_wage;
}

int civ_labor_available_workers(const civ_labor_market_t *l, civ_labor_tier_t tier) {
  if (!l || tier >= CIV_LABOR_TIER_COUNT) return 0;
  return l->segments[tier].workforce - l->segments[tier].employed;
}

civ_float_t civ_labor_wage_pressure(const civ_labor_market_t *l) {
  if (!l) return 0.0;
  /* Low unemployment → upward wage pressure */
  return (1.0 - l->overall_unemployment) * 2.0 - 1.0;
}
