/**
 * @file resource_market.h
 * @brief Procedural Resource-Driven Macro-Economic Engine
 */

#ifndef CIVILIZATION_RESOURCE_MARKET_H
#define CIVILIZATION_RESOURCE_MARKET_H

#include "../../common.h"
#include "../../types.h"

/* Resource Category */
typedef enum {
  CIV_RES_CAT_FOOD = 0,
  CIV_RES_CAT_BASIC_MATERIAL,
  CIV_RES_CAT_INDUSTRIAL,
  CIV_RES_CAT_CONSUMER,
  CIV_RES_CAT_LUXURY,
  CIV_RES_CAT_FANTASY /* Emergent/Magic/Rare */
} civ_resource_category_t;

/* Procedural Resource Definition */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  civ_resource_category_t category;
  civ_float_t base_value;
  civ_float_t scarcity_bias; /* 0.0 to 1.0 */
} civ_resource_definition_t;

/* Regional Resource State */
typedef struct {
  char resource_id[STRING_SHORT_LEN];
  civ_float_t local_supply;
  civ_float_t local_demand;
  civ_float_t current_price;
  civ_float_t stockpile;
} civ_regional_resource_t;

/* Commodity Market */
typedef struct {
  civ_regional_resource_t *resources;
  size_t resource_count;
  size_t resource_capacity;

  civ_float_t total_trade_volume;
  civ_float_t global_price_index;
} civ_commodity_market_t;

/* Functions */
civ_commodity_market_t *civ_resource_market_create(void);
void civ_resource_market_destroy(civ_commodity_market_t *market);

civ_result_t civ_resource_register(civ_commodity_market_t *market,
                                   const char *name,
                                   civ_resource_category_t cat);
void civ_resource_update_price(civ_regional_resource_t *res,
                               civ_float_t global_index);

#endif /* CIVILIZATION_RESOURCE_MARKET_H */
