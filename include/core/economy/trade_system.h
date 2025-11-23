/**
 * @file trade_system.h
 * @brief Trade Routes and Tariffs
 */

#ifndef CIVILIZATION_TRADE_SYSTEM_H
#define CIVILIZATION_TRADE_SYSTEM_H

#include "../../common.h"
#include "../../types.h"
#include "currency_system.h"

/* Trade Route */
typedef struct {
  char id[STRING_SHORT_LEN];
  char source_nation_id[STRING_SHORT_LEN];
  char target_nation_id[STRING_SHORT_LEN];

  char resource_type[STRING_SHORT_LEN]; /* e.g., "Grain", "Iron" */
  civ_float_t amount;
  civ_float_t value_per_unit;

  civ_float_t tariff_rate; /* 0.0 to 1.0 */

  bool active;
  time_t established_date;
} civ_trade_route_t;

/* Trade Manager */
typedef struct {
  civ_trade_route_t *routes;
  size_t route_count;
  size_t route_capacity;

  civ_currency_manager_t *currency_manager; /* Reference */
} civ_trade_manager_t;

/* Functions */
civ_trade_manager_t *
civ_trade_manager_create(civ_currency_manager_t *currency_mgr);
void civ_trade_manager_destroy(civ_trade_manager_t *manager);

civ_result_t civ_trade_establish_route(civ_trade_manager_t *manager,
                                       const char *source, const char *target,
                                       const char *resource,
                                       civ_float_t amount);
civ_result_t civ_trade_cancel_route(civ_trade_manager_t *manager,
                                    const char *route_id);

void civ_trade_update(civ_trade_manager_t *manager, civ_float_t time_delta);
civ_float_t civ_trade_calculate_revenue(const civ_trade_route_t *route);

#endif /* CIVILIZATION_TRADE_SYSTEM_H */
