#include "core/economy/black_market.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>

civ_black_market_system_t *civ_black_market_create(void) {
  civ_black_market_system_t *b = CIV_MALLOC(sizeof(civ_black_market_system_t));
  if (!b) return NULL;
  memset(b, 0, sizeof(*b));
  b->enforcement_effectiveness = 0.40;
  b->enforcement_budget = 10000.0;
  b->criminal_violence = 0.05;

  /* Initialize contraband markets */
  const char *names[] = { "Weapons", "Drugs", "Smuggled Goods", "Trafficking", "Counterfeit" };
  for (int i = 0; i < CIV_CONTRABAND_TYPE_COUNT; i++) {
    b->markets[i].type = (civ_contraband_type_t)i;
    b->markets[i].volume = 5000.0 * (i + 1);
    b->markets[i].profit_margin = 0.3 + i * 0.1;
    b->markets[i].active = true;
  }
  return b;
}

void civ_black_market_destroy(civ_black_market_system_t *b) { free(b); }

void civ_black_market_update(civ_black_market_system_t *b, civ_float_t time_delta,
                             civ_float_t gdp, civ_float_t corruption_level,
                             civ_float_t unemployment_rate, civ_float_t regulation_level,
                             civ_float_t enforcement_budget) {
  if (!b) return;
  (void)time_delta;

  b->enforcement_budget = enforcement_budget;
  /* Effectiveness: more budget = more effective, but corruption undermines */
  civ_float_t budget_per_gdp = (gdp > 0) ? enforcement_budget / gdp : 0.0;
  b->enforcement_effectiveness = budget_per_gdp * 50.0 * (1.0 - corruption_level * 0.8);
  if (b->enforcement_effectiveness > 0.95) b->enforcement_effectiveness = 0.95;
  if (b->enforcement_effectiveness < 0.05) b->enforcement_effectiveness = 0.05;

  /* Each contraband market responds to conditions */
  civ_float_t total_illicit = 0.0;
  for (int i = 0; i < CIV_CONTRABAND_TYPE_COUNT; i++) {
    civ_contraband_market_t *m = &b->markets[i];

    /* Enforcement pressure */
    m->enforcement_pressure = b->enforcement_effectiveness;

    /* Volume: grows with unemployment, regulation, corruption; shrinks with enforcement */
    civ_float_t growth = unemployment_rate * 0.3 + regulation_level * 0.15
                        + corruption_level * 0.25 - m->enforcement_pressure * 0.4;
    m->volume *= (1.0 + growth);
    if (m->volume < 100.0) m->volume = 100.0;

    /* Profit margin: higher enforcement = higher risk premium */
    m->profit_margin = 0.2 + m->enforcement_pressure * 0.4 + corruption_level * 0.2;

    total_illicit += m->volume;
  }

  /* Total illicit economy as fraction of GDP */
  b->total_illicit_economy = (gdp > 0) ? total_illicit / gdp : 0.0;

  /* Corruption revenue: officials take a cut */
  b->corruption_revenue = total_illicit * corruption_level * 0.15;

  /* Tax revenue lost: illicit economy * average tax rate */
  b->tax_revenue_lost = total_illicit * 0.20;

  /* Criminal violence: rises with illicit economy and enforcement */
  b->criminal_violence = b->total_illicit_economy * 0.4
                         + b->enforcement_effectiveness * 0.1 * (1.0 - corruption_level);
  if (b->criminal_violence > 1.0) b->criminal_violence = 1.0;
}

civ_float_t civ_black_market_size(const civ_black_market_system_t *b) {
  return b ? b->total_illicit_economy : 0.0;
}

civ_float_t civ_black_market_tax_loss(const civ_black_market_system_t *b) {
  return b ? b->tax_revenue_lost : 0.0;
}

void civ_black_market_crackdown(civ_black_market_system_t *b, civ_float_t investment) {
  if (!b || investment <= 0) return;
  b->enforcement_budget += investment;
  /* Immediate effect: reduces contraband volume */
  for (int i = 0; i < CIV_CONTRABAND_TYPE_COUNT; i++)
    b->markets[i].volume *= 0.85;
}
