/**
 * @file currency_system.c
 * @brief Implementation of advanced currency dynamics
 */

#include "../../../include/core/economy/currency_system.h"
#include "../../../include/common.h"
#include <math.h>
#include <string.h>


void civ_currency_init(civ_currency_t *currency, const char *name) {
  if (!currency || !name)
    return;

  memset(currency, 0, sizeof(civ_currency_t));
  strncpy(currency->name, name, STRING_MEDIUM_LEN - 1);
  currency->exchange_rate = 1.0f;
  currency->inflation_index = 1.0f;
  currency->reserve_strength = 0.8f;
  currency->circulation_volume = 1000000.0f;
}

void civ_currency_update_value(civ_currency_t *currency,
                               const civ_national_macro_state_t *macro) {
  if (!currency || !macro)
    return;

  /* Exchange rate is powerful if productivity is high and unemployment is low
   */
  civ_float_t productivity_mod = macro->national_productivity * 0.5f;
  civ_float_t stability_mod = macro->industrial_stability * 0.3f;
  civ_float_t cycle_mod = 1.0f;

  if (macro->current_cycle == CIV_ECON_CYCLE_RECESSION)
    cycle_mod = 0.8f;
  else if (macro->current_cycle == CIV_ECON_CYCLE_DEPRESSION)
    cycle_mod = 0.4f;

  currency->exchange_rate =
      (productivity_mod + stability_mod + (1.0f - macro->unemployment_index)) *
      cycle_mod;
  currency->exchange_rate = CLAMP(currency->exchange_rate, 0.01f, 5.0f);

  /* Inflation rises if reserves are low or printing is too high (implied volume
   * growth) */
  if (currency->reserve_strength < 0.3f) {
    currency->inflation_index += 0.05f;
  }
}

civ_float_t civ_currency_get_exchange_power(const civ_currency_t *a,
                                            const civ_currency_t *b) {
  if (!a || !b)
    return 1.0f;
  return a->exchange_rate / MAX(0.01f, b->exchange_rate);
}
