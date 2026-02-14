/**
 * @file dynamic_economy.h
 * @brief Economy system with supply/demand, trade, and resource flows
 *
 * Prices are dynamic, production scales with knowledge, no artificial caps
 */

#ifndef CIVILIZATION_DYNAMIC_ECONOMY_H
#define CIVILIZATION_DYNAMIC_ECONOMY_H

#include "../common.h"
#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resource types (can be extended)
 */
typedef enum {
  RESOURCE_FOOD,
  RESOURCE_PRODUCTION,
  RESOURCE_GOLD,
  RESOURCE_LUXURY,
  RESOURCE_STRATEGIC,
  RESOURCE_COUNT
} civ_resource_type_t;

/**
 * @brief Market state for a resource
 */
typedef struct {
  double supply;  // Units available per turn
  double demand;  // Units wanted per turn
  double price;   // Current market price (dynamic)
  double reserve; // Stockpiled amount
} civ_market_t;

/**
 * @brief Economic state tracking
 */
typedef struct {
  civ_market_t markets[RESOURCE_COUNT];
  double gdp;              // Total economic output
  double inflation;        // Price level change rate
  double trade_efficiency; // 0.0 → 1.0+
  int32_t trade_routes;    // Active trade connections
} civ_economy_state_t;

/**
 * @brief Calculate production output based on knowledge and workers
 */
double civ_economy_production(double engineering_level, double worker_count,
                              double tool_quality);

/**
 * @brief Calculate market price based on supply/demand
 * Uses elasticity formula: price = base × (demand/supply)^elasticity
 */
double civ_economy_market_price(const civ_market_t *market, double base_price,
                                double elasticity);

/**
 * @brief Update markets: adjust prices, apply supply/demand
 */
void civ_economy_update_markets(civ_economy_state_t *economy);

/**
 * @brief Calculate trade value between two economies
 */
double civ_economy_trade_value(const civ_economy_state_t *economy_a,
                               const civ_economy_state_t *economy_b,
                               double distance_factor);

/**
 * @brief Add supply to a market (production)
 */
void civ_economy_add_supply(civ_market_t *market, double amount);

/**
 * @brief Consume from a market (creates demand)
 * Returns actual amount consumed (may be less if scarce)
 */
double civ_economy_consume(civ_market_t *market, double desired_amount);

/**
 * @brief Calculate GDP from all production
 */
double civ_economy_calculate_gdp(const civ_economy_state_t *economy);

/**
 * @brief Initialize economy
 */
void civ_economy_init(civ_economy_state_t *economy);

#ifdef __cplusplus
}
#endif

#endif /* CIVILIZATION_DYNAMIC_ECONOMY_H */
