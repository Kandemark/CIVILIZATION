/**
 * @file taxation.c
 * @brief Taxation — brackets, collection, evasion, revenue computation
 */
#include "core/economy/taxation.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CIV_TAX_INITIAL_BRACKET_CAP 8

civ_taxation_system_t *civ_taxation_create(void) {
  civ_taxation_system_t *t = CIV_MALLOC(sizeof(civ_taxation_system_t));
  if (!t) return NULL;
  memset(t, 0, sizeof(*t));

  /* Initialize each tax policy with sensible dynamic defaults */
  for (int i = 0; i < CIV_TAX_TYPE_COUNT; i++) {
    t->policies[i].type       = (civ_tax_type_t)i;
    t->policies[i].active     = false;
    t->policies[i].flat_rate  = 0.0;
    t->policies[i].collection_efficiency = 0.85;
    t->policies[i].evasion_rate = 0.10;
  }

  /* Income tax: progressive brackets */
  t->policies[CIV_TAX_INCOME].active = true;
  t->policies[CIV_TAX_INCOME].bracket_capacity = CIV_TAX_INITIAL_BRACKET_CAP;
  t->policies[CIV_TAX_INCOME].brackets = CIV_MALLOC(sizeof(civ_tax_bracket_t) * CIV_TAX_INITIAL_BRACKET_CAP);

  /* Corporate tax */
  t->policies[CIV_TAX_CORPORATE].active = true;
  t->policies[CIV_TAX_CORPORATE].flat_rate = 0.20;

  /* Sales tax */
  t->policies[CIV_TAX_SALES].active = true;
  t->policies[CIV_TAX_SALES].flat_rate = 0.05;

  return t;
}

void civ_taxation_destroy(civ_taxation_system_t *t) {
  if (!t) return;
  for (int i = 0; i < CIV_TAX_TYPE_COUNT; i++) {
    free(t->policies[i].brackets);
  }
  free(t);
}

void civ_taxation_update(civ_taxation_system_t *t,
                         civ_float_t time_delta,
                         civ_float_t total_gdp,
                         civ_float_t population,
                         civ_float_t governance_efficiency,
                         civ_float_t corruption_level) {
  if (!t) return;
  (void)time_delta;

  /* Collection efficiency decays with corruption, improves with governance */
  civ_float_t target_efficiency = governance_efficiency * (1.0 - corruption_level * 0.7);
  for (int i = 0; i < CIV_TAX_TYPE_COUNT; i++) {
    t->policies[i].collection_efficiency +=
      (target_efficiency - t->policies[i].collection_efficiency) * 0.2;
    /* Evasion rises with corruption */
    t->policies[i].evasion_rate = corruption_level * 0.8;
  }

  /* Revenue computation per tax type */
  civ_float_t revenue = 0.0;
  civ_float_t per_capita_gdp = (population > 0) ? total_gdp / population : 0.0;

  /* Income tax: apply brackets to per-capita GDP, then scale by population */
  if (t->policies[CIV_TAX_INCOME].active && t->policies[CIV_TAX_INCOME].bracket_count > 0) {
    civ_float_t avg_income_tax = civ_taxation_calculate_income_tax(t, per_capita_gdp);
    revenue += avg_income_tax * population * t->policies[CIV_TAX_INCOME].collection_efficiency
               * (1.0 - t->policies[CIV_TAX_INCOME].evasion_rate);
  }

  /* Corporate tax: fraction of GDP */
  if (t->policies[CIV_TAX_CORPORATE].active) {
    civ_float_t corp_profit_share = 0.15; /* roughly 15% of GDP is corporate profit */
    revenue += total_gdp * corp_profit_share * t->policies[CIV_TAX_CORPORATE].flat_rate
               * t->policies[CIV_TAX_CORPORATE].collection_efficiency
               * (1.0 - t->policies[CIV_TAX_CORPORATE].evasion_rate);
  }

  /* Sales tax: applied to consumption (~65% of GDP) */
  if (t->policies[CIV_TAX_SALES].active) {
    civ_float_t consumption = total_gdp * 0.65;
    revenue += consumption * t->policies[CIV_TAX_SALES].flat_rate
               * t->policies[CIV_TAX_SALES].collection_efficiency
               * (1.0 - t->policies[CIV_TAX_SALES].evasion_rate);
  }

  /* Remaining taxes (flat) */
  for (int i = CIV_TAX_PROPERTY; i < CIV_TAX_TYPE_COUNT; i++) {
    if (!t->policies[i].active) continue;
    civ_float_t base = 0.0;
    switch (i) {
      case CIV_TAX_PROPERTY:   base = total_gdp * 0.02; break;
      case CIV_TAX_TARIFF:     base = total_gdp * 0.05; break;
      case CIV_TAX_LUXURY:     base = total_gdp * 0.01; break;
      case CIV_TAX_INHERITANCE: base = total_gdp * 0.005; break;
      default: break;
    }
    revenue += base * t->policies[i].flat_rate
               * t->policies[i].collection_efficiency
               * (1.0 - t->policies[i].evasion_rate);
  }

  t->total_revenue     = revenue;
  t->projected_revenue = revenue; /* would incorporate growth forecasts */

  /* Tax burden: fraction of GDP taken as tax */
  t->tax_burden_index = (total_gdp > 0) ? (revenue / total_gdp) : 0.0;

  /* Gini: simplified model from tax progressivity */
  civ_float_t top_rate = t->policies[CIV_TAX_INCOME].bracket_count > 0
    ? t->policies[CIV_TAX_INCOME].brackets[t->policies[CIV_TAX_INCOME].bracket_count - 1].rate
    : 0.0;
  t->gini_coefficient = 0.45 - (top_rate * 0.25); /* progressive tax reduces inequality */
  if (t->gini_coefficient < 0.20) t->gini_coefficient = 0.20;
  if (t->gini_coefficient > 0.65) t->gini_coefficient = 0.65;
}

void civ_taxation_set_policy(civ_taxation_system_t *t,
                             civ_tax_type_t type,
                             civ_float_t flat_rate) {
  if (!t || type >= CIV_TAX_TYPE_COUNT) return;
  t->policies[type].flat_rate = flat_rate;
  if (flat_rate < 0.0) t->policies[type].flat_rate = 0.0;
  if (flat_rate > 1.0) t->policies[type].flat_rate = 1.0;
}

void civ_taxation_add_bracket(civ_taxation_system_t *t,
                              civ_tax_type_t type,
                              civ_float_t threshold,
                              civ_float_t rate) {
  if (!t || type >= CIV_TAX_TYPE_COUNT) return;
  civ_tax_policy_t *p = &t->policies[type];

  if (p->bracket_count >= p->bracket_capacity) {
    int new_cap = p->bracket_capacity * 2;
    civ_tax_bracket_t *tmp = CIV_REALLOC(p->brackets, sizeof(civ_tax_bracket_t) * new_cap);
    if (!tmp) return;
    p->brackets = tmp;
    p->bracket_capacity = new_cap;
  }

  /* Insert maintaining threshold order */
  int pos = p->bracket_count;
  for (int i = 0; i < p->bracket_count; i++) {
    if (threshold < p->brackets[i].threshold) { pos = i; break; }
  }
  memmove(&p->brackets[pos + 1], &p->brackets[pos],
          (p->bracket_count - pos) * sizeof(civ_tax_bracket_t));
  p->brackets[pos].threshold = threshold;
  p->brackets[pos].rate      = rate;
  p->bracket_count++;
}

void civ_taxation_toggle_tax(civ_taxation_system_t *t, civ_tax_type_t type, bool active) {
  if (!t || type >= CIV_TAX_TYPE_COUNT) return;
  t->policies[type].active = active;
}

civ_float_t civ_taxation_calculate_income_tax(const civ_taxation_system_t *t, civ_float_t income) {
  if (!t) return 0.0;
  const civ_tax_policy_t *p = &t->policies[CIV_TAX_INCOME];
  if (!p->active || p->bracket_count == 0) return 0.0;

  civ_float_t tax = 0.0;
  civ_float_t prev_threshold = 0.0;

  for (int i = 0; i < p->bracket_count; i++) {
    civ_float_t bracket_income = income - prev_threshold;
    if (bracket_income <= 0) break;

    civ_float_t taxable_in_bracket = (income > p->brackets[i].threshold)
      ? (p->brackets[i].threshold - prev_threshold)
      : bracket_income;

    tax += taxable_in_bracket * p->brackets[i].rate;
    prev_threshold = p->brackets[i].threshold;
  }

  /* Top bracket catches remainder */
  if (income > prev_threshold && p->bracket_count > 0) {
    tax += (income - prev_threshold) * p->brackets[p->bracket_count - 1].rate;
  }

  return tax;
}

civ_float_t civ_taxation_effective_tax_rate(const civ_taxation_system_t *t) {
  return t ? t->tax_burden_index : 0.0;
}

civ_float_t civ_taxation_revenue_forecast(const civ_taxation_system_t *t) {
  return t ? t->projected_revenue : 0.0;
}
