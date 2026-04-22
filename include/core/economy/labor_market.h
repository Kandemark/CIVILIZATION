/**
 * @file labor_market.h
 * @brief Labor market — employment, wages, skills, workforce participation
 */
#ifndef CIV_ECONOMY_LABOR_MARKET_H
#define CIV_ECONOMY_LABOR_MARKET_H

#include "../../common.h"
#include "../../types.h"

typedef enum { CIV_LABOR_UNSKILLED, CIV_LABOR_SKILLED, CIV_LABOR_PROFESSIONAL,
               CIV_LABOR_EXECUTIVE, CIV_LABOR_TIER_COUNT } civ_labor_tier_t;

typedef struct {
  civ_labor_tier_t tier;
  int              workforce;          /* people in this tier */
  int              employed;
  civ_float_t      avg_wage;
  civ_float_t      wage_growth;
  civ_float_t      unemployment_rate;  /* within this tier */
} civ_labor_segment_t;

typedef struct {
  civ_labor_segment_t segments[CIV_LABOR_TIER_COUNT];
  int                 total_workforce;
  int                 total_employed;
  int                 total_unemployed;
  civ_float_t         overall_unemployment;
  civ_float_t         labor_force_participation; /* 0.0-1.0 */
  civ_float_t         avg_wage_national;
  civ_float_t         wage_inequality;           /* gini-like 0.0-1.0 */
  civ_float_t         minimum_wage;
  civ_float_t         productivity_per_worker;
} civ_labor_market_t;

civ_labor_market_t *civ_labor_market_create(void);
void civ_labor_market_destroy(civ_labor_market_t *l);
void civ_labor_market_update(civ_labor_market_t *l, civ_float_t time_delta,
                             int total_population, civ_float_t gdp,
                             civ_float_t tech_level, civ_float_t education_level,
                             civ_float_t business_confidence);

civ_float_t civ_labor_wage_for_tier(const civ_labor_market_t *l, civ_labor_tier_t tier);
int civ_labor_available_workers(const civ_labor_market_t *l, civ_labor_tier_t tier);
civ_float_t civ_labor_wage_pressure(const civ_labor_market_t *l);

#endif
