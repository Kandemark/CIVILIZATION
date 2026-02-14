/**
 * @file currency_system.h
 * @brief Advanced Emergent Currency and National Macro-Economics
 */

#ifndef CIVILIZATION_CURRENCY_SYSTEM_H
#define CIVILIZATION_CURRENCY_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Economic Cycle State */
typedef enum {
  CIV_ECON_CYCLE_GROWTH = 0,
  CIV_ECON_CYCLE_PEAK,
  CIV_ECON_CYCLE_RECESSION,
  CIV_ECON_CYCLE_DEPRESSION,
  CIV_ECON_CYCLE_RECOVERY
} civ_economic_cycle_t;

/* National Macro-State (The "Why" of the Economy) */
typedef struct {
  civ_float_t national_productivity; /* GDP output */
  civ_float_t industrial_stability;  /* Resilience to shocks */
  civ_float_t unemployment_index;    /* 0.0 to 1.0 */
  civ_economic_cycle_t current_cycle;
} civ_national_macro_state_t;

/* Currency Unit */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];

  civ_float_t exchange_rate;    /* Relative to global standard */
  civ_float_t inflation_index;  /* Consumer price index */
  civ_float_t reserve_strength; /* Backing assets confidence */

  civ_float_t circulation_volume;
} civ_currency_t;

/* Currency Manager */
typedef struct {
  civ_currency_t *currencies;
  size_t currency_count;
  size_t currency_capacity;
} civ_currency_manager_t;

/* Functions */
void civ_currency_init(civ_currency_t *currency, const char *name);
void civ_currency_update_value(civ_currency_t *currency,
                               const civ_national_macro_state_t *macro);

civ_float_t civ_currency_get_exchange_power(const civ_currency_t *a,
                                            const civ_currency_t *b);

#endif /* CIVILIZATION_CURRENCY_SYSTEM_H */
