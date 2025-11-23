/**
 * @file trade_system.c
 * @brief Implementation of Trade System
 */

#include "../../../include/core/economy/trade_system.h"
#include <stdio.h>
#include <string.h>

civ_trade_manager_t *
civ_trade_manager_create(civ_currency_manager_t *currency_mgr) {
  civ_trade_manager_t *manager = CIV_MALLOC(sizeof(civ_trade_manager_t));
  if (manager) {
    manager->routes = NULL;
    manager->route_count = 0;
    manager->route_capacity = 0;
    manager->currency_manager = currency_mgr;
  }
  return manager;
}

void civ_trade_manager_destroy(civ_trade_manager_t *manager) {
  if (manager) {
    CIV_FREE(manager->routes);
    CIV_FREE(manager);
  }
}

civ_result_t civ_trade_establish_route(civ_trade_manager_t *manager,
                                       const char *source, const char *target,
                                       const char *resource,
                                       civ_float_t amount) {
  if (!manager || !source || !target)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (manager->route_count >= manager->route_capacity) {
    size_t new_cap =
        manager->route_capacity == 0 ? 8 : manager->route_capacity * 2;
    civ_trade_route_t *new_arr =
        CIV_REALLOC(manager->routes, new_cap * sizeof(civ_trade_route_t));
    if (!new_arr)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->routes = new_arr;
    manager->route_capacity = new_cap;
  }

  civ_trade_route_t *route = &manager->routes[manager->route_count++];
  snprintf(route->id, STRING_SHORT_LEN, "trade_%ld", (long)time(NULL));
  strncpy(route->source_nation_id, source, STRING_SHORT_LEN - 1);
  strncpy(route->target_nation_id, target, STRING_SHORT_LEN - 1);
  strncpy(route->resource_type, resource, STRING_SHORT_LEN - 1);

  route->amount = amount;
  route->value_per_unit = 10.0f; // Base value
  route->tariff_rate = 0.05f;    // 5% default tariff
  route->active = true;
  route->established_date = time(NULL);

  return (civ_result_t){CIV_OK, "Trade route established"};
}

civ_result_t civ_trade_cancel_route(civ_trade_manager_t *manager,
                                    const char *route_id) {
  if (!manager || !route_id)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  for (size_t i = 0; i < manager->route_count; i++) {
    if (strcmp(manager->routes[i].id, route_id) == 0) {
      manager->routes[i].active = false;
      return (civ_result_t){CIV_OK, "Trade route cancelled"};
    }
  }
  return (civ_result_t){CIV_ERROR_NOT_FOUND, "Route not found"};
}

void civ_trade_update(civ_trade_manager_t *manager, civ_float_t time_delta) {
  if (!manager)
    return;

  // Update logic: Fluctuations in value, random disruptions
  for (size_t i = 0; i < manager->route_count; i++) {
    if (!manager->routes[i].active)
      continue;

    // Random value fluctuation
    civ_float_t fluctuation = ((rand() % 100) - 50) / 1000.0f; // -0.05 to 0.05
    manager->routes[i].value_per_unit *= (1.0f + fluctuation);
    if (manager->routes[i].value_per_unit < 1.0f)
      manager->routes[i].value_per_unit = 1.0f;
  }
}

civ_float_t civ_trade_calculate_revenue(const civ_trade_route_t *route) {
  if (!route || !route->active)
    return 0.0f;
  return route->amount * route->value_per_unit * (1.0f - route->tariff_rate);
}
