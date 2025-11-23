/**
 * @file currency_system.h
 * @brief Currency and Exchange Rate System
 */

#ifndef CIVILIZATION_CURRENCY_SYSTEM_H
#define CIVILIZATION_CURRENCY_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Currency structure */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  char symbol[8];
  char issuing_nation_id[STRING_SHORT_LEN];

  civ_float_t
      exchange_rate; /* Relative to a global standard (e.g., Gold = 1.0) */
  civ_float_t inflation_rate; /* Annual inflation percentage */
  civ_float_t volatility;     /* How much the rate fluctuates */

  int64_t circulation; /* Total money supply */

  bool active;
} civ_currency_t;

/* Currency Manager */
typedef struct {
  civ_currency_t *currencies;
  size_t currency_count;
  size_t currency_capacity;

  civ_float_t global_volatility_index;
} civ_currency_manager_t;

/* Functions */
civ_currency_manager_t *civ_currency_manager_create(void);
void civ_currency_manager_destroy(civ_currency_manager_t *manager);

civ_currency_t *civ_currency_create(const char *name, const char *symbol,
                                    const char *nation_id);
civ_result_t civ_currency_manager_add(civ_currency_manager_t *manager,
                                      civ_currency_t *currency);
civ_currency_t *civ_currency_manager_find(civ_currency_manager_t *manager,
                                          const char *id);
civ_currency_t *
civ_currency_manager_find_by_nation(civ_currency_manager_t *manager,
                                    const char *nation_id);

void civ_currency_update_rates(civ_currency_manager_t *manager,
                               civ_float_t time_delta);
civ_float_t civ_currency_convert(const civ_currency_t *from,
                                 const civ_currency_t *to, civ_float_t amount);

#endif /* CIVILIZATION_CURRENCY_SYSTEM_H */
