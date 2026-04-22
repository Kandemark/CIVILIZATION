/**
 * @file budget.h
 * @brief Government budget — revenue, expenditure, debt, deficit management
 *
 * Dynamic fiscal simulation. Spending allocations shift based on player
 * priorities and crisis conditions. Debt becomes unsustainable if ignored.
 */
#ifndef CIV_ECONOMY_BUDGET_H
#define CIV_ECONOMY_BUDGET_H

#include "../../common.h"
#include "../../types.h"

/* Spending categories */
typedef enum {
  CIV_BUDGET_MILITARY = 0,
  CIV_BUDGET_INFRASTRUCTURE,
  CIV_BUDGET_EDUCATION,
  CIV_BUDGET_HEALTHCARE,
  CIV_BUDGET_WELFARE,
  CIV_BUDGET_RESEARCH,
  CIV_BUDGET_ADMINISTRATION,
  CIV_BUDGET_DEBT_SERVICE,
  CIV_BUDGET_CATEGORY_COUNT
} civ_budget_category_t;

/* Fiscal quarter record */
typedef struct {
  civ_float_t revenue;
  civ_float_t expenditure;
  civ_float_t deficit;
  civ_float_t debt_total;
  civ_float_t debt_to_gdp;
  int         quarter;
  int         year;
} civ_fiscal_record_t;

/* National budget */
typedef struct {
  civ_float_t allocations[CIV_BUDGET_CATEGORY_COUNT]; /* fraction each, sum=1.0 */
  civ_float_t total_revenue;
  civ_float_t total_expenditure;
  civ_float_t deficit;              /* revenue - expenditure (negative = deficit) */
  civ_float_t national_debt;
  civ_float_t debt_interest_rate;
  civ_float_t debt_to_gdp_ratio;
  civ_float_t credit_rating;        /* 0.0-1.0, affects borrowing cost */
  civ_float_t fiscal_health;        /* composite 0.0-1.0 */
  civ_fiscal_record_t *history;
  int                  history_count;
  int                  history_capacity;
  int                  quarters_without_budget; /* triggers government shutdown */
} civ_budget_system_t;

/* --- Lifecycle --- */
civ_budget_system_t *civ_budget_create(void);
void                  civ_budget_destroy(civ_budget_system_t *b);

/* --- Update --- */
void civ_budget_update(civ_budget_system_t *b,
                       civ_float_t time_delta,
                       civ_float_t tax_revenue,
                       civ_float_t gdp,
                       civ_float_t population,
                       civ_float_t inflation_rate);

/* --- Allocation --- */
void civ_budget_set_allocation(civ_budget_system_t *b,
                               civ_budget_category_t cat,
                               civ_float_t fraction);
void civ_budget_normalize_allocations(civ_budget_system_t *b);

/* --- Borrowing --- */
civ_float_t civ_budget_borrow(civ_budget_system_t *b, civ_float_t amount);
civ_float_t civ_budget_repay_debt(civ_budget_system_t *b, civ_float_t amount);
civ_float_t civ_budget_max_borrowing(const civ_budget_system_t *b);

/* --- Query --- */
civ_float_t civ_budget_spending_for(const civ_budget_system_t *b, civ_budget_category_t cat);
civ_float_t civ_budget_surplus_ratio(const civ_budget_system_t *b);
bool        civ_budget_is_sustainable(const civ_budget_system_t *b);

#endif /* CIV_ECONOMY_BUDGET_H */
