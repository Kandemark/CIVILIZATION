/**
 * @file currency_system.c
 * @brief Implementation of Currency System
 */

#include "../../../include/core/economy/currency_system.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


civ_currency_manager_t *civ_currency_manager_create(void) {
  civ_currency_manager_t *manager = CIV_MALLOC(sizeof(civ_currency_manager_t));
  if (manager) {
    manager->currencies = NULL;
    manager->currency_count = 0;
    manager->currency_capacity = 0;
    manager->global_volatility_index = 0.05f; // 5% base volatility
  }
  return manager;
}

void civ_currency_manager_destroy(civ_currency_manager_t *manager) {
  if (manager) {
    CIV_FREE(manager->currencies);
    CIV_FREE(manager);
  }
}

civ_currency_t *civ_currency_create(const char *name, const char *symbol,
                                    const char *nation_id) {
  civ_currency_t *currency = CIV_MALLOC(sizeof(civ_currency_t));
  if (currency) {
    snprintf(currency->id, STRING_SHORT_LEN, "curr_%s", nation_id);
    strncpy(currency->name, name, STRING_MEDIUM_LEN - 1);
    strncpy(currency->symbol, symbol, 7);
    strncpy(currency->issuing_nation_id, nation_id, STRING_SHORT_LEN - 1);

    currency->exchange_rate = 1.0f;
    currency->inflation_rate = 0.02f; // 2% target
    currency->volatility = 0.02f;
    currency->circulation = 1000000;
    currency->active = true;
  }
  return currency;
}

civ_result_t civ_currency_manager_add(civ_currency_manager_t *manager,
                                      civ_currency_t *currency) {
  if (!manager || !currency)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (manager->currency_count >= manager->currency_capacity) {
    size_t new_cap =
        manager->currency_capacity == 0 ? 4 : manager->currency_capacity * 2;
    civ_currency_t *new_arr =
        CIV_REALLOC(manager->currencies, new_cap * sizeof(civ_currency_t));
    if (!new_arr)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->currencies = new_arr;
    manager->currency_capacity = new_cap;
  }

  manager->currencies[manager->currency_count++] = *currency;
  // Note: We copy the struct value. If 'currency' was malloced, caller must
  // free it (but NOT its contents if deep copy needed, here it's flat).
  // Actually, civ_currency_create mallocs. So caller should free the pointer
  // after adding.

  return (civ_result_t){CIV_OK, "Currency added"};
}

civ_currency_t *civ_currency_manager_find(civ_currency_manager_t *manager,
                                          const char *id) {
  if (!manager || !id)
    return NULL;
  for (size_t i = 0; i < manager->currency_count; i++) {
    if (strcmp(manager->currencies[i].id, id) == 0) {
      return &manager->currencies[i];
    }
  }
  return NULL;
}

civ_currency_t *
civ_currency_manager_find_by_nation(civ_currency_manager_t *manager,
                                    const char *nation_id) {
  if (!manager || !nation_id)
    return NULL;
  for (size_t i = 0; i < manager->currency_count; i++) {
    if (strcmp(manager->currencies[i].issuing_nation_id, nation_id) == 0) {
      return &manager->currencies[i];
    }
  }
  return NULL;
}

void civ_currency_update_rates(civ_currency_manager_t *manager,
                               civ_float_t time_delta) {
  if (!manager)
    return;

  for (size_t i = 0; i < manager->currency_count; i++) {
    civ_currency_t *c = &manager->currencies[i];
    if (!c->active)
      continue;

    // Random fluctuation based on volatility
    civ_float_t fluctuation =
        ((rand() % 200) - 100) / 10000.0f; // -0.01 to 0.01
    c->exchange_rate +=
        c->exchange_rate * (fluctuation * c->volatility * 10.0f);

    // Inflation effect
    c->circulation +=
        (int64_t)(c->circulation * c->inflation_rate * time_delta);
    c->exchange_rate *= (1.0f - (c->inflation_rate * time_delta *
                                 0.1f)); // Inflation devalues currency

    // Clamp
    if (c->exchange_rate < 0.0001f)
      c->exchange_rate = 0.0001f;
  }
}

civ_float_t civ_currency_convert(const civ_currency_t *from,
                                 const civ_currency_t *to, civ_float_t amount) {
  if (!from || !to)
    return amount; // Assume 1:1 if null
  // Convert to standard (Gold), then to target
  // Amount * FromRate = GoldValue
  // GoldValue / ToRate = TargetAmount
  return (amount * from->exchange_rate) / to->exchange_rate;
}
