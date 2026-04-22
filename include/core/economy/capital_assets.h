/**
 * @file capital_assets.h
 * @brief Capital assets — factories, equipment, depreciation, investment
 */
#ifndef CIV_ECONOMY_CAPITAL_ASSETS_H
#define CIV_ECONOMY_CAPITAL_ASSETS_H

#include "../../common.h"
#include "../../types.h"

typedef enum { CIV_CAPITAL_MACHINERY, CIV_CAPITAL_VEHICLES, CIV_CAPITAL_BUILDINGS,
               CIV_CAPITAL_TECHNOLOGY, CIV_CAPITAL_INTELLECTUAL, CIV_CAPITAL_TYPE_COUNT } civ_capital_type_t;

typedef struct {
  civ_capital_type_t type;
  civ_float_t        total_value;
  civ_float_t        depreciation_rate;   /* fraction lost per cycle */
  civ_float_t        investment_rate;     /* new capital added per cycle */
  civ_float_t        utilization;         /* 0.0-1.0 */
  civ_float_t        avg_age_years;
} civ_capital_pool_t;

typedef struct {
  civ_capital_pool_t pools[CIV_CAPITAL_TYPE_COUNT];
  civ_float_t        total_capital_stock;
  civ_float_t        capital_per_worker;
  civ_float_t        investment_ratio;    /* fraction of GDP reinvested */
  civ_float_t        depreciation_burden; /* total depreciation cost */
  civ_float_t        productivity_multiplier;
} civ_capital_assets_system_t;

civ_capital_assets_system_t *civ_capital_assets_create(void);
void civ_capital_assets_destroy(civ_capital_assets_system_t *c);
void civ_capital_assets_update(civ_capital_assets_system_t *c, civ_float_t time_delta,
                               civ_float_t gdp, civ_float_t savings_rate,
                               civ_float_t interest_rate, civ_float_t business_confidence);

void civ_capital_assets_invest(civ_capital_assets_system_t *c,
                               civ_capital_type_t type, civ_float_t amount);
civ_float_t civ_capital_assets_return_on_capital(const civ_capital_assets_system_t *c);
civ_float_t civ_capital_assets_productivity_boost(const civ_capital_assets_system_t *c);

#endif
