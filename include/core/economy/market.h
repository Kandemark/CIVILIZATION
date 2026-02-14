/**
 * @file market.h
 * @brief Market dynamics and economic system
 */

#ifndef CIVILIZATION_MARKET_H
#define CIVILIZATION_MARKET_H

#include "../../common.h"
#include "../../types.h"
#include "../environment/geography.h"

/* Market sentiment enumeration */
typedef enum {
  CIV_MARKET_SENTIMENT_BEARISH = 0,
  CIV_MARKET_SENTIMENT_NEUTRAL = 1,
  CIV_MARKET_SENTIMENT_BULLISH = 2
} civ_market_sentiment_t;

/* Economic Sector */
typedef struct {
  civ_float_t output;       /* Value produced */
  civ_float_t productivity; /* Efficiency multiplier */
  civ_float_t growth_rate;  /* Current growth trend */
} civ_economic_sector_t;

/* Economic report structure */
typedef struct {
  civ_float_t gdp;
  civ_float_t gdp_per_capita;
  civ_float_t unemployment_rate;
  civ_float_t inflation_rate;
  civ_float_t growth_rate;

  /* Sector breakdown */
  civ_float_t agricultural_output;
  civ_float_t industrial_output;
  civ_float_t service_output;

  civ_market_sentiment_t sentiment;
} civ_economic_report_t;

/* Market dynamics structure */
typedef struct {
  civ_float_t tax_rate;
  civ_float_t government_spending;
  civ_float_t base_gdp; /* Legacy/Base factor */
  civ_float_t base_unemployment;
  civ_float_t base_inflation;

  /* Sectors */
  civ_economic_sector_t agriculture;
  civ_economic_sector_t industry;
  civ_economic_sector_t services;

  /* Market state */
  civ_market_sentiment_t sentiment;
  civ_float_t market_volatility;

  /* Historical data */
  civ_economic_report_t *reports;
  size_t report_count;
  size_t report_capacity;
} civ_market_dynamics_t;

/* Function declarations */

/**
 * Create a new market dynamics system
 */
civ_market_dynamics_t *civ_market_dynamics_create(void);

/**
 * Destroy market dynamics system
 */
void civ_market_dynamics_destroy(civ_market_dynamics_t *market);

/**
 * Initialize market dynamics
 */
void civ_market_dynamics_init(civ_market_dynamics_t *market);

/**
 * Update market based on time delta, population data, and geography
 */
civ_economic_report_t
civ_market_dynamics_update(civ_market_dynamics_t *market,
                           civ_float_t time_delta, const void *population_data,
                           const civ_geography_t *geography,
                           civ_float_t tech_level);

/**
 * Get current economic report
 */
civ_economic_report_t
civ_market_dynamics_get_report(const civ_market_dynamics_t *market);

/**
 * Set tax rate
 */
void civ_market_dynamics_set_tax_rate(civ_market_dynamics_t *market,
                                      civ_float_t rate);

/**
 * Set government spending
 */
void civ_market_dynamics_set_government_spending(civ_market_dynamics_t *market,
                                                 civ_float_t spending);

#endif /* CIVILIZATION_MARKET_H */
