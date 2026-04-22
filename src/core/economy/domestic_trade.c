#include "core/economy/domestic_trade.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CIV_DOM_TRADE_INITIAL_ROUTE_CAP 16

civ_domestic_trade_system_t *civ_domestic_trade_create(void) {
  civ_domestic_trade_system_t *d = CIV_MALLOC(sizeof(civ_domestic_trade_system_t));
  if (!d) return NULL;
  memset(d, 0, sizeof(*d));
  d->route_capacity = CIV_DOM_TRADE_INITIAL_ROUTE_CAP;
  d->routes = CIV_MALLOC(sizeof(civ_domestic_route_t) * d->route_capacity);
  d->market_integration = 0.50;
  d->internal_tariff_equivalent = 0.12;
  return d;
}

void civ_domestic_trade_destroy(civ_domestic_trade_system_t *d) {
  if (!d) return;
  free(d->routes);
  free(d);
}

void civ_domestic_trade_update(civ_domestic_trade_system_t *d, civ_float_t time_delta,
                               civ_float_t infrastructure_quality, civ_float_t population,
                               civ_float_t geography_size) {
  if (!d) return;
  (void)time_delta;

  /* Market integration improves with infrastructure */
  d->market_integration += (infrastructure_quality - d->market_integration) * 0.08;

  /* Internal friction from poor infrastructure and large geography */
  d->internal_tariff_equivalent = (1.0 - infrastructure_quality) * 0.3
                                  + (geography_size / 50000.0) * 0.05;
  if (d->internal_tariff_equivalent > 0.50) d->internal_tariff_equivalent = 0.50;

  /* Trade volume scales with population * integration */
  d->total_domestic_volume = population * d->market_integration * 0.5;

  /* Update route flows based on price differentials and transport cost */
  for (int i = 0; i < d->route_count; i++) {
    civ_domestic_route_t *r = &d->routes[i];
    if (!r->active) continue;

    r->transport_cost = d->internal_tariff_equivalent * 0.5;
    r->price_differential *= (1.0 - d->market_integration * 0.1); /* arbitrage narrows gaps */

    /* Flow proportional to price gap minus transport cost */
    civ_float_t net_margin = r->price_differential - r->transport_cost;
    r->flow_volume = (net_margin > 0) ? net_margin * 100.0 : 0.0;
  }
}

civ_float_t civ_domestic_trade_price_in_region(const civ_domestic_trade_system_t *d,
                                                const char *good, const char *region) {
  (void)good; (void)region;
  if (!d) return 1.0;
  /* Base price modified by market integration */
  return 1.0 + (1.0 - d->market_integration) * 0.3;
}

civ_float_t civ_domestic_trade_efficiency(const civ_domestic_trade_system_t *d) {
  return d ? d->market_integration : 0.0;
}
