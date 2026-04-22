/**
 * @file budget.c
 * @brief Government budget — revenue, expenditure, debt, fiscal health
 */
#include "core/economy/budget.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CIV_BUDGET_INITIAL_HISTORY_CAP 40

civ_budget_system_t *civ_budget_create(void) {
  civ_budget_system_t *b = CIV_MALLOC(sizeof(civ_budget_system_t));
  if (!b) return NULL;
  memset(b, 0, sizeof(*b));

  /* Balanced initial allocations */
  civ_float_t default_alloc = 1.0 / (civ_float_t)CIV_BUDGET_CATEGORY_COUNT;
  for (int i = 0; i < CIV_BUDGET_CATEGORY_COUNT; i++) {
    b->allocations[i] = default_alloc;
  }

  b->debt_interest_rate = 0.03;
  b->credit_rating      = 0.80;
  b->fiscal_health      = 0.75;

  b->history_capacity = CIV_BUDGET_INITIAL_HISTORY_CAP;
  b->history = CIV_MALLOC(sizeof(civ_fiscal_record_t) * b->history_capacity);

  return b;
}

void civ_budget_destroy(civ_budget_system_t *b) {
  if (!b) return;
  free(b->history);
  free(b);
}

void civ_budget_update(civ_budget_system_t *b,
                       civ_float_t time_delta,
                       civ_float_t tax_revenue,
                       civ_float_t gdp,
                       civ_float_t population,
                       civ_float_t inflation_rate) {
  if (!b) return;
  (void)time_delta;
  (void)population;

  b->total_revenue = tax_revenue;

  /* Expenditure = allocations scaled to revenue + borrowing capacity */
  civ_float_t base_spending = tax_revenue;
  if (base_spending < gdp * 0.01) base_spending = gdp * 0.01; /* floor */

  b->total_expenditure = 0.0;
  for (int i = 0; i < CIV_BUDGET_CATEGORY_COUNT; i++) {
    b->total_expenditure += base_spending * b->allocations[i];
  }

  /* Deficit spending: spend more than revenue if needed */
  b->deficit = b->total_revenue - b->total_expenditure;

  /* If running deficit, borrow */
  if (b->deficit < 0.0) {
    civ_budget_borrow(b, -b->deficit);
  } else if (b->deficit > 0.0 && b->national_debt > 0.0) {
    /* Surplus → repay debt */
    civ_budget_repay_debt(b, b->deficit * 0.7);
  }

  /* Debt interest accumulates */
  civ_float_t interest_payment = b->national_debt * b->debt_interest_rate;
  b->national_debt += interest_payment;

  /* Debt service allocation covers interest */
  b->total_expenditure += interest_payment;
  b->deficit = b->total_revenue - b->total_expenditure;

  /* Debt-to-GDP */
  b->debt_to_gdp_ratio = (gdp > 0) ? (b->national_debt / gdp) : 0.0;

  /* Credit rating: falls as debt/GDP rises, rises with fiscal discipline */
  civ_float_t target_rating = 0.95 - b->debt_to_gdp_ratio * 0.8;
  if (b->deficit > 0) target_rating += 0.05;
  if (target_rating < 0.05) target_rating = 0.05;
  if (target_rating > 0.98) target_rating = 0.98;
  b->credit_rating += (target_rating - b->credit_rating) * 0.1;

  /* Borrowing cost from credit rating */
  b->debt_interest_rate = 0.01 + (1.0 - b->credit_rating) * 0.15;

  /* Fiscal health composite */
  civ_float_t deficit_health = (b->deficit >= 0) ? 1.0 :
    (1.0 - fabs(b->deficit) / (gdp * 0.1 + 1.0));
  civ_float_t debt_health = 1.0 - b->debt_to_gdp_ratio;
  if (debt_health < 0.0) debt_health = 0.0;
  if (deficit_health < 0.0) deficit_health = 0.0;

  b->fiscal_health = (deficit_health * 0.4 + debt_health * 0.4 + b->credit_rating * 0.2);

  /* Record in history (one entry per update, up to capacity) */
  if (b->history_count < b->history_capacity) {
    civ_fiscal_record_t *rec = &b->history[b->history_count];
    rec->revenue      = b->total_revenue;
    rec->expenditure  = b->total_expenditure;
    rec->deficit      = b->deficit;
    rec->debt_total   = b->national_debt;
    rec->debt_to_gdp  = b->debt_to_gdp_ratio;
    rec->quarter      = b->history_count;
    rec->year         = 0;
    b->history_count++;
  }
}

void civ_budget_set_allocation(civ_budget_system_t *b,
                               civ_budget_category_t cat,
                               civ_float_t fraction) {
  if (!b || cat >= CIV_BUDGET_CATEGORY_COUNT || fraction < 0.0 || fraction > 1.0) return;
  b->allocations[cat] = fraction;
  civ_budget_normalize_allocations(b);
}

void civ_budget_normalize_allocations(civ_budget_system_t *b) {
  if (!b) return;
  civ_float_t sum = 0.0;
  for (int i = 0; i < CIV_BUDGET_CATEGORY_COUNT; i++) sum += b->allocations[i];
  if (sum <= 0.0) {
    civ_float_t even = 1.0 / (civ_float_t)CIV_BUDGET_CATEGORY_COUNT;
    for (int i = 0; i < CIV_BUDGET_CATEGORY_COUNT; i++) b->allocations[i] = even;
  } else {
    for (int i = 0; i < CIV_BUDGET_CATEGORY_COUNT; i++) b->allocations[i] /= sum;
  }
}

civ_float_t civ_budget_borrow(civ_budget_system_t *b, civ_float_t amount) {
  if (!b || amount <= 0) return 0.0;
  civ_float_t max_borrow = civ_budget_max_borrowing(b);
  civ_float_t actual = (amount > max_borrow) ? max_borrow : amount;
  b->national_debt += actual;
  return actual;
}

civ_float_t civ_budget_repay_debt(civ_budget_system_t *b, civ_float_t amount) {
  if (!b || amount <= 0 || b->national_debt <= 0) return 0.0;
  civ_float_t actual = (amount > b->national_debt) ? b->national_debt : amount;
  b->national_debt -= actual;
  return actual;
}

civ_float_t civ_budget_max_borrowing(const civ_budget_system_t *b) {
  if (!b) return 0.0;
  /* Can borrow more with good credit rating, less when debt/GDP is high */
  return b->total_revenue * b->credit_rating * 5.0 * (1.0 - b->debt_to_gdp_ratio * 0.5);
}

civ_float_t civ_budget_spending_for(const civ_budget_system_t *b, civ_budget_category_t cat) {
  if (!b || cat >= CIV_BUDGET_CATEGORY_COUNT) return 0.0;
  return b->total_expenditure * b->allocations[cat];
}

civ_float_t civ_budget_surplus_ratio(const civ_budget_system_t *b) {
  if (!b || b->total_expenditure <= 0) return 0.0;
  return b->deficit / b->total_expenditure;
}

bool civ_budget_is_sustainable(const civ_budget_system_t *b) {
  return b ? (b->fiscal_health > 0.3) : false;
}
