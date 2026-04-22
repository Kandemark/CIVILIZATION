/**
 * @file economic_policy.c
 * @brief Economic policy — subsidies, price controls, regulations, confidence
 */
#include "core/economy/economic_policy.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_economic_policy_system_t *civ_economic_policy_create(void) {
  civ_economic_policy_system_t *p = CIV_MALLOC(sizeof(civ_economic_policy_system_t));
  if (!p) return NULL;
  memset(p, 0, sizeof(*p));

  /* Sensible initial state */
  p->regulation            = CIV_REGULATION_MODERATE;
  p->minimum_wage          = 0.0; /* disabled until set */
  p->consumer_confidence   = 0.65;
  p->business_confidence   = 0.60;
  p->economic_freedom_index = 0.70;

  return p;
}

void civ_economic_policy_destroy(civ_economic_policy_system_t *p) {
  free(p);
}

void civ_economic_policy_update(civ_economic_policy_system_t *p,
                                civ_float_t time_delta,
                                civ_float_t inflation_rate,
                                civ_float_t unemployment_rate,
                                civ_float_t gdp_growth,
                                civ_float_t corruption_level) {
  if (!p) return;
  (void)time_delta;

  /* Sum policy costs */
  p->total_policy_cost = 0.0;
  for (int i = 0; i < CIV_POLICY_TYPE_COUNT; i++) {
    if (p->instruments[i].active) {
      /* Effectiveness degrades with corruption */
      p->instruments[i].effectiveness = 1.0 - corruption_level * 0.8;
      p->total_policy_cost += p->instruments[i].cost * p->instruments[i].magnitude;
    }
  }

  /* Consumer confidence: driven by employment + inflation */
  civ_float_t cc_target = 0.70;
  cc_target -= (unemployment_rate - 0.05) * 1.5;  /* high unemployment hurts */
  cc_target -= (inflation_rate - 0.03) * 2.0;     /* high inflation hurts */
  cc_target += gdp_growth * 2.0;                  /* growth helps */
  if (cc_target < 0.10) cc_target = 0.10;
  if (cc_target > 0.95) cc_target = 0.95;
  p->consumer_confidence += (cc_target - p->consumer_confidence) * 0.15;

  /* Business confidence: growth + regulation freedom */
  civ_float_t bc_target = 0.65;
  bc_target += gdp_growth * 2.5;
  bc_target -= (civ_float_t)p->regulation * 0.06; /* heavy regulation hurts */
  bc_target -= corruption_level * 0.4;
  bc_target -= (inflation_rate - 0.02) * 1.0;
  if (bc_target < 0.05) bc_target = 0.05;
  if (bc_target > 0.95) bc_target = 0.95;
  p->business_confidence += (bc_target - p->business_confidence) * 0.12;

  /* Economic freedom index */
  civ_float_t ef_target = 0.85 - (civ_float_t)p->regulation * 0.15 - corruption_level * 0.3;
  if (ef_target < 0.10) ef_target = 0.10;
  p->economic_freedom_index += (ef_target - p->economic_freedom_index) * 0.1;
}

void civ_economic_policy_set_instrument(civ_economic_policy_system_t *p,
                                        civ_policy_type_t type,
                                        bool active,
                                        civ_float_t magnitude,
                                        const char *target_sector) {
  if (!p || type >= CIV_POLICY_TYPE_COUNT) return;
  p->instruments[type].type      = type;
  p->instruments[type].active    = active;
  p->instruments[type].magnitude = magnitude;
  if (target_sector)
    strncpy(p->instruments[type].target_sector, target_sector, STRING_SHORT_LEN - 1);

  /* Compute cost based on type */
  switch (type) {
    case CIV_POLICY_SUBSIDY:
    case CIV_POLICY_EXPORT_SUBSIDY:
      p->instruments[type].cost = magnitude * 50000.0;
      break;
    case CIV_POLICY_INVESTMENT_TAX_CREDIT:
      p->instruments[type].cost = magnitude * 30000.0;
      break;
    default:
      p->instruments[type].cost = magnitude * 10000.0;
      break;
  }
}

void civ_economic_policy_set_regulation(civ_economic_policy_system_t *p,
                                        civ_regulation_level_t level) {
  if (!p) return;
  p->regulation = level;
}

civ_float_t civ_economic_policy_subsidy_multiplier(const civ_economic_policy_system_t *p,
                                                    const char *sector) {
  if (!p || !sector) return 1.0;
  civ_float_t mult = 1.0;
  for (int i = 0; i < CIV_POLICY_TYPE_COUNT; i++) {
    if (!p->instruments[i].active) continue;
    if (p->instruments[i].type == CIV_POLICY_SUBSIDY ||
        p->instruments[i].type == CIV_POLICY_INVESTMENT_TAX_CREDIT) {
      if (strcmp(p->instruments[i].target_sector, sector) == 0) {
        mult += p->instruments[i].magnitude * p->instruments[i].effectiveness * 0.3;
      }
    }
  }
  return mult;
}

civ_float_t civ_economic_policy_price_modifier(const civ_economic_policy_system_t *p,
                                                const char *commodity) {
  if (!p || !commodity) return 1.0;
  (void)commodity;

  civ_float_t mod = 1.0;

  /* Price cap reduces prices but can cause supply issues */
  if (p->instruments[CIV_POLICY_PRICE_CAP].active)
    mod -= p->instruments[CIV_POLICY_PRICE_CAP].magnitude * 0.15;

  /* Price floor raises prices */
  if (p->instruments[CIV_POLICY_PRICE_FLOOR].active)
    mod += p->instruments[CIV_POLICY_PRICE_FLOOR].magnitude * 0.10;

  /* Import quota raises domestic prices */
  if (p->instruments[CIV_POLICY_IMPORT_QUOTA].active)
    mod += p->instruments[CIV_POLICY_IMPORT_QUOTA].magnitude * 0.08;

  if (mod < 0.5) mod = 0.5;
  if (mod > 2.0) mod = 2.0;
  return mod;
}

civ_float_t civ_economic_policy_growth_modifier(const civ_economic_policy_system_t *p) {
  if (!p) return 1.0;
  civ_float_t mod = 1.0;

  /* Regulation drags growth */
  mod -= (civ_float_t)p->regulation * 0.04;

  /* Investment tax credits boost */
  if (p->instruments[CIV_POLICY_INVESTMENT_TAX_CREDIT].active)
    mod += p->instruments[CIV_POLICY_INVESTMENT_TAX_CREDIT].magnitude
           * p->instruments[CIV_POLICY_INVESTMENT_TAX_CREDIT].effectiveness * 0.08;

  /* Confidence effects */
  mod += (p->business_confidence - 0.5) * 0.15;
  mod += (p->consumer_confidence - 0.5) * 0.10;

  if (mod < 0.5) mod = 0.5;
  if (mod > 1.5) mod = 1.5;
  return mod;
}

civ_float_t civ_economic_policy_inequality_modifier(const civ_economic_policy_system_t *p) {
  if (!p) return 1.0;
  civ_float_t mod = 1.0;

  /* Minimum wage reduces inequality */
  if (p->minimum_wage > 0.0) mod -= 0.05;

  /* Heavy regulation tends to concentrate power in connected firms */
  if (p->regulation >= CIV_REGULATION_HEAVY) mod *= 1.05;

  /* Deregulation increases inequality */
  if (p->regulation <= CIV_REGULATION_LIGHT) mod *= 1.08;

  return mod;
}
