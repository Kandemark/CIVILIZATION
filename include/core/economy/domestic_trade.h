/**
 * @file domestic_trade.h
 * @brief Domestic trade — internal goods movement, regional price dynamics, market integration
 */
#ifndef CIV_ECONOMY_DOMESTIC_TRADE_H
#define CIV_ECONOMY_DOMESTIC_TRADE_H

#include "../../common.h"
#include "../../types.h"

typedef struct {
  char        region_from[STRING_SHORT_LEN];
  char        region_to[STRING_SHORT_LEN];
  char        good[STRING_SHORT_LEN];
  civ_float_t flow_volume;
  civ_float_t transport_cost;
  civ_float_t price_differential;
  bool        active;
} civ_domestic_route_t;

typedef struct {
  civ_domestic_route_t *routes;
  int                    route_count;
  int                    route_capacity;
  civ_float_t            market_integration;   /* 0.0-1.0, how connected regions are */
  civ_float_t            internal_tariff_equivalent; /* friction from poor roads etc */
  civ_float_t            total_domestic_volume;
} civ_domestic_trade_system_t;

civ_domestic_trade_system_t *civ_domestic_trade_create(void);
void civ_domestic_trade_destroy(civ_domestic_trade_system_t *d);
void civ_domestic_trade_update(civ_domestic_trade_system_t *d, civ_float_t time_delta,
                               civ_float_t infrastructure_quality, civ_float_t population,
                               civ_float_t geography_size);

civ_float_t civ_domestic_trade_price_in_region(const civ_domestic_trade_system_t *d,
                                                const char *good, const char *region);
civ_float_t civ_domestic_trade_efficiency(const civ_domestic_trade_system_t *d);

#endif
