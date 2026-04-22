/**
 * @file black_market.h
 * @brief Black market — smuggling, grey markets, corruption economy, illicit trade
 */
#ifndef CIV_ECONOMY_BLACK_MARKET_H
#define CIV_ECONOMY_BLACK_MARKET_H

#include "../../common.h"
#include "../../types.h"

typedef enum { CIV_CONTRABAND_WEAPONS, CIV_CONTRABAND_DRUGS, CIV_CONTRABAND_GOODS,
               CIV_CONTRABAND_PEOPLE, CIV_CONTRABAND_COUNTERFEIT, CIV_CONTRABAND_TYPE_COUNT } civ_contraband_type_t;

typedef struct {
  civ_contraband_type_t type;
  civ_float_t           volume;           /* economic value flowing */
  civ_float_t           profit_margin;    /* compared to legal equivalent */
  civ_float_t           enforcement_pressure; /* 0.0-1.0 */
  bool                  active;
} civ_contraband_market_t;

typedef struct {
  civ_contraband_market_t markets[CIV_CONTRABAND_TYPE_COUNT];
  civ_float_t             total_illicit_economy; /* fraction of GDP */
  civ_float_t             corruption_revenue;     /* bribes, kickbacks */
  civ_float_t             enforcement_budget;
  civ_float_t             enforcement_effectiveness; /* 0.0-1.0 */
  civ_float_t             tax_revenue_lost;      /* evaded taxes */
  civ_float_t             criminal_violence;     /* 0.0-1.0 */
} civ_black_market_system_t;

civ_black_market_system_t *civ_black_market_create(void);
void civ_black_market_destroy(civ_black_market_system_t *b);
void civ_black_market_update(civ_black_market_system_t *b, civ_float_t time_delta,
                             civ_float_t gdp, civ_float_t corruption_level,
                             civ_float_t unemployment_rate, civ_float_t regulation_level,
                             civ_float_t enforcement_budget);

civ_float_t civ_black_market_size(const civ_black_market_system_t *b);
civ_float_t civ_black_market_tax_loss(const civ_black_market_system_t *b);
void civ_black_market_crackdown(civ_black_market_system_t *b, civ_float_t investment);

#endif
