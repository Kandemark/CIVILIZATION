#include "core/economy/capital_assets.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>

civ_capital_assets_system_t *civ_capital_assets_create(void) {
  civ_capital_assets_system_t *c = CIV_MALLOC(sizeof(civ_capital_assets_system_t));
  if (!c) return NULL;
  memset(c, 0, sizeof(*c));

  civ_float_t init_val[] = { 200000.0, 100000.0, 500000.0, 50000.0, 30000.0 };
  civ_float_t dep_rate[] = { 0.08, 0.12, 0.03, 0.15, 0.05 };
  for (int i = 0; i < CIV_CAPITAL_TYPE_COUNT; i++) {
    c->pools[i].type = (civ_capital_type_t)i;
    c->pools[i].total_value = init_val[i];
    c->pools[i].depreciation_rate = dep_rate[i];
    c->pools[i].utilization = 0.75;
    c->pools[i].avg_age_years = 5.0;
  }
  c->total_capital_stock = 880000.0;
  c->productivity_multiplier = 1.0;
  return c;
}

void civ_capital_assets_destroy(civ_capital_assets_system_t *c) { free(c); }

void civ_capital_assets_update(civ_capital_assets_system_t *c, civ_float_t time_delta,
                               civ_float_t gdp, civ_float_t savings_rate,
                               civ_float_t interest_rate, civ_float_t business_confidence) {
  if (!c) return;
  (void)time_delta;

  /* Investment from savings */
  civ_float_t investable = gdp * savings_rate * business_confidence;
  c->investment_ratio = (gdp > 0) ? investable / gdp : 0.0;

  /* Distribute investment across capital types */
  civ_float_t alloc[] = { 0.30, 0.15, 0.30, 0.15, 0.10 };
  c->total_capital_stock = 0.0;
  c->depreciation_burden = 0.0;

  for (int i = 0; i < CIV_CAPITAL_TYPE_COUNT; i++) {
    civ_capital_pool_t *p = &c->pools[i];

    /* Depreciation */
    civ_float_t dep = p->total_value * p->depreciation_rate;
    c->depreciation_burden += dep;

    /* Interest-rate-sensitive investment */
    civ_float_t rate_sensitivity = (1.0 - interest_rate * 1.5);
    if (rate_sensitivity < 0.1) rate_sensitivity = 0.1;
    p->investment_rate = investable * alloc[i] * rate_sensitivity;

    /* Update pool */
    p->total_value -= dep;
    p->total_value += p->investment_rate;
    if (p->total_value < 0) p->total_value = 0;

    /* Utilization from business confidence */
    p->utilization += (business_confidence - p->utilization) * 0.15;

    /* Age: weighted by new investment vs old */
    if (p->total_value > 0)
      p->avg_age_years += (1.0 - p->investment_rate / (p->total_value + 1.0)) * 0.1;

    c->total_capital_stock += p->total_value;
  }

  /* Capital per worker: aggregate productivity */
  c->capital_per_worker = 50000.0; /* would use actual workforce from labor_market */

  /* Productivity multiplier from capital stock relative to GDP */
  c->productivity_multiplier = 1.0 + (c->total_capital_stock / (gdp + 1.0)) * 0.3;
}

void civ_capital_assets_invest(civ_capital_assets_system_t *c,
                               civ_capital_type_t type, civ_float_t amount) {
  if (!c || type >= CIV_CAPITAL_TYPE_COUNT || amount <= 0) return;
  c->pools[type].total_value += amount;
  c->pools[type].avg_age_years *= 0.9; /* new capital resets age somewhat */
}

civ_float_t civ_capital_assets_return_on_capital(const civ_capital_assets_system_t *c) {
  if (!c || c->total_capital_stock <= 0) return 0.0;
  civ_float_t total_return = 0.0;
  for (int i = 0; i < CIV_CAPITAL_TYPE_COUNT; i++)
    total_return += c->pools[i].total_value * c->pools[i].utilization * 0.15;
  return total_return / c->total_capital_stock;
}

civ_float_t civ_capital_assets_productivity_boost(const civ_capital_assets_system_t *c) {
  return c ? c->productivity_multiplier : 1.0;
}
