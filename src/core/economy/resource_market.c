/**
 * @file resource_market.c
 * @brief Implementation of procedural macro-economics
 */

#include "../../../include/core/economy/resource_market.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_commodity_market_t *civ_resource_market_create(void) {
  civ_commodity_market_t *market =
      (civ_commodity_market_t *)CIV_MALLOC(sizeof(civ_commodity_market_t));
  if (!market)
    return NULL;

  memset(market, 0, sizeof(civ_commodity_market_t));
  market->resource_capacity = 32;
  market->resources = (civ_regional_resource_t *)CIV_CALLOC(
      market->resource_capacity, sizeof(civ_regional_resource_t));
  market->global_price_index = 1.0f;

  return market;
}

void civ_resource_market_destroy(civ_commodity_market_t *market) {
  if (!market)
    return;
  CIV_FREE(market->resources);
  CIV_FREE(market);
}

civ_result_t civ_resource_register(civ_commodity_market_t *market,
                                   const char *name,
                                   civ_resource_category_t cat) {
  if (!market || !name)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  if (market->resource_count >= market->resource_capacity) {
    market->resource_capacity *= 2;
    market->resources = (civ_regional_resource_t *)CIV_REALLOC(
        market->resources,
        market->resource_capacity * sizeof(civ_regional_resource_t));
  }

  if (market->resources) {
    civ_regional_resource_t *res = &market->resources[market->resource_count++];
    memset(res, 0, sizeof(civ_regional_resource_t));
    strncpy(res->resource_id, name, STRING_SHORT_LEN - 1);
    res->local_supply = 100.0f;
    res->local_demand = 100.0f;
    res->current_price = 1.0f;

    civ_log(CIV_LOG_INFO, "Registered procedural resource: %s (Category: %d)",
            name, (int)cat);
    return (civ_result_t){CIV_OK, NULL};
  }

  return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
}

void civ_resource_update_price(civ_regional_resource_t *res,
                               civ_float_t global_index) {
  if (!res)
    return;

  /* Price driven by regional scarcity * global market index */
  civ_float_t scarcity = res->local_demand / MAX(1.0f, res->local_supply);
  res->current_price = scarcity * global_index;

  /* Bubble logic (simplified) */
  if (res->current_price > 5.0f) {
    civ_log(CIV_LOG_WARNING, "Economic bubble detected for resource %s",
            res->resource_id);
  }
}
