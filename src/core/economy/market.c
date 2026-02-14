/**
 * @file market.c
 * @brief Implementation of market dynamics
 */

#include "../../../include/core/economy/market.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>


civ_market_dynamics_t *civ_market_dynamics_create(void) {
  civ_market_dynamics_t *market =
      (civ_market_dynamics_t *)CIV_MALLOC(sizeof(civ_market_dynamics_t));
  if (!market) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate market dynamics");
    return NULL;
  }

  civ_market_dynamics_init(market);
  return market;
}

void civ_market_dynamics_destroy(civ_market_dynamics_t *market) {
  if (!market)
    return;

  CIV_FREE(market->reports);
  CIV_FREE(market);
}

void civ_market_dynamics_init(civ_market_dynamics_t *market) {
  if (!market)
    return;

  memset(market, 0, sizeof(civ_market_dynamics_t));

  market->tax_rate = 0.2f;            /* 20% */
  market->government_spending = 0.3f; /* 30% */
  market->base_gdp = 1000000.0f;
  market->base_unemployment = 0.05f; /* 5% */
  market->base_inflation = 0.02f;    /* 2% */
  market->sentiment = CIV_MARKET_SENTIMENT_NEUTRAL;
  market->market_volatility = 0.1f;

  market->report_capacity = 100;
  market->reports = (civ_economic_report_t *)CIV_CALLOC(
      market->report_capacity, sizeof(civ_economic_report_t));
}

civ_economic_report_t
civ_market_dynamics_update(civ_market_dynamics_t *market,
                           civ_float_t time_delta, const void *population_data,
                           const civ_geography_t *geography,
                           civ_float_t tech_level) {
  civ_economic_report_t report = {0};

  if (!market)
    return report;

  /* Assume population data provides workforce size */
  /* For now, use a default population */
  civ_float_t default_population = 10000.0f;
  // In real integration, we'd cast population_data to civ_population_manager_t*

  // Tech multiplier affects all sectors
  civ_float_t tech_multiplier = 1.0f + tech_level * 0.15f;

  // --- Sector Calculations ---

  // 1. Agriculture: Driven by Land Area and arable land
  civ_float_t arable_land = 0.0f;
  if (geography) {
    arable_land = civ_geography_get_agricultural_area(geography);
  }
  // If no geography, minimal subsistence farming
  if (arable_land < 1.0f)
    arable_land = 100.0f;

  market->agriculture.productivity =
      1.0f * tech_multiplier; // Tech boosts efficiency
  market->agriculture.output = arable_land * market->agriculture.productivity *
                               50.0f; // 50 value per km2?

  // 2. Industry: Driven by base factors + urbanization (simulated) + tech
  // Industry needs workforce. Let's assume 20% of pop is industrial workforce
  civ_float_t industrial_workforce = default_population * 0.2f;
  market->industry.productivity =
      1.2f * tech_multiplier *
      tech_multiplier; // Industry scales better with tech
  market->industry.output =
      industrial_workforce * market->industry.productivity * 10.0f;

  // 3. Services: Driven by population size and wealth (correlated to other
  // sectors)
  civ_float_t service_workforce = default_population * 0.4f;
  market->services.productivity = 0.8f * tech_multiplier;
  market->services.output =
      service_workforce * market->services.productivity * 8.0f;

  // Total GDP
  report.agricultural_output = market->agriculture.output;
  report.industrial_output = market->industry.output;
  report.service_output = market->services.output;

  civ_float_t raw_gdp = report.agricultural_output + report.industrial_output +
                        report.service_output;

  // Apply macro factors (Tax, Spending)
  civ_float_t tax_effect = 1.0f - market->tax_rate * 0.5f;
  civ_float_t spending_effect = 1.0f + market->government_spending * 0.3f;

  report.gdp = raw_gdp * tax_effect * spending_effect;
  report.gdp_per_capita = report.gdp / default_population;

  /* Unemployment calculation */
  civ_float_t unemployment_base = market->base_unemployment;
  civ_float_t unemployment_modifier =
      (1.0f - tech_level * 0.1f) + (market->tax_rate * 0.2f);
  report.unemployment_rate =
      CLAMP(unemployment_base * unemployment_modifier, 0.0f, 1.0f);

  /* Inflation calculation */
  civ_float_t inflation_base = market->base_inflation;
  civ_float_t inflation_modifier =
      1.0f + (market->government_spending - 0.3f) * 0.5f;
  report.inflation_rate =
      CLAMP(inflation_base * inflation_modifier, -0.1f, 0.5f);

  /* Growth rate */
  report.growth_rate = (tech_level * 0.02f) -
                       (report.unemployment_rate * 0.01f) +
                       (report.inflation_rate * 0.005f);
  report.growth_rate = CLAMP(report.growth_rate, -0.1f, 0.15f);

  /* Market sentiment */
  if (report.growth_rate > 0.03f) {
    report.sentiment = CIV_MARKET_SENTIMENT_BULLISH;
  } else if (report.growth_rate < -0.01f) {
    report.sentiment = CIV_MARKET_SENTIMENT_BEARISH;
  } else {
    report.sentiment = CIV_MARKET_SENTIMENT_NEUTRAL;
  }

  market->sentiment = report.sentiment;

  /* Store report */
  if (market->report_count < market->report_capacity) {
    market->reports[market->report_count++] = report;
  } else {
    /* Shift reports */
    memmove(market->reports, market->reports + 1,
            (market->report_capacity - 1) * sizeof(civ_economic_report_t));
    market->reports[market->report_capacity - 1] = report;
  }

  return report;
}

civ_economic_report_t
civ_market_dynamics_get_report(const civ_market_dynamics_t *market) {
  civ_economic_report_t report = {0};

  if (!market || market->report_count == 0)
    return report;

  return market->reports[market->report_count - 1];
}

void civ_market_dynamics_set_tax_rate(civ_market_dynamics_t *market,
                                      civ_float_t rate) {
  if (!market)
    return;
  market->tax_rate = CLAMP(rate, 0.0f, 1.0f);
}

void civ_market_dynamics_set_government_spending(civ_market_dynamics_t *market,
                                                 civ_float_t spending) {
  if (!market)
    return;
  market->government_spending = CLAMP(spending, 0.0f, 1.0f);
}
