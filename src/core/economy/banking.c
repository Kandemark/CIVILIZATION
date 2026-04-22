/**
 * @file banking.c
 * @brief Central banking — reserve requirements, lending, interest rate dynamics
 */
#include "core/economy/banking.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CIV_BANKING_INITIAL_LOAN_CAP 32
#define CIV_BANKING_RESERVE_FLOOR    0.01
#define CIV_BANKING_RESERVE_CEILING  0.25
#define CIV_BANKING_RATE_FLOOR       0.001
#define CIV_BANKING_RATE_CEILING     0.35

civ_banking_system_t *civ_banking_create(void) {
  civ_banking_system_t *b = CIV_MALLOC(sizeof(civ_banking_system_t));
  if (!b) return NULL;
  memset(b, 0, sizeof(*b));

  /* Initial dynamic defaults — no hardcoded "golden" numbers */
  b->reserve_ratio       = 0.10;   /* 10% reserve requirement */
  b->base_interest_rate  = 0.04;   /* 4% policy rate */
  b->money_supply        = 1000000.0;
  b->reserve_holdings    = b->money_supply * b->reserve_ratio;
  b->credit_multiplier   = 1.0 / (b->reserve_ratio + 0.001);

  b->loan_capacity = CIV_BANKING_INITIAL_LOAN_CAP;
  b->loans = CIV_MALLOC(sizeof(civ_loan_t) * b->loan_capacity);
  if (!b->loans) {
    free(b);
    return NULL;
  }
  return b;
}

void civ_banking_destroy(civ_banking_system_t *b) {
  if (!b) return;
  free(b->loans);
  free(b);
}

/* --- Update: everything responds to macro conditions --- */
void civ_banking_update(civ_banking_system_t *b,
                        civ_float_t time_delta,
                        civ_float_t inflation_rate,
                        civ_float_t gdp_growth,
                        civ_float_t unemployment_rate,
                        civ_float_t government_spending_ratio) {
  (void)time_delta;

  /* Central bank adjusts policy rate:
   * - Raise rates when inflation > 3% (cool the economy)
   * - Lower rates when growth is negative and unemployment > 7%
   * - Neutral otherwise */
  civ_float_t target_rate = 0.04;  /* neutral */

  if (inflation_rate > 0.06) {
    target_rate += (inflation_rate - 0.03) * 0.8;  /* aggressive anti-inflation */
  } else if (inflation_rate > 0.03) {
    target_rate += (inflation_rate - 0.03) * 0.4;
  }

  if (gdp_growth < 0.0 && unemployment_rate > 0.07) {
    target_rate -= 0.02;  /* stimulus */
  } else if (gdp_growth < 0.02) {
    target_rate -= 0.005;
  }

  /* Government spending crowds out private lending somewhat */
  target_rate += government_spending_ratio * 0.01;

  /* Smooth adjustment */
  b->base_interest_rate += (target_rate - b->base_interest_rate) * 0.1;

  /* Clamp */
  if (b->base_interest_rate < CIV_BANKING_RATE_FLOOR)
    b->base_interest_rate = CIV_BANKING_RATE_FLOOR;
  if (b->base_interest_rate > CIV_BANKING_RATE_CEILING)
    b->base_interest_rate = CIV_BANKING_RATE_CEILING;

  /* Reserve ratio tweaked by inflation */
  if (inflation_rate > 0.05) {
    b->reserve_ratio += 0.002;  /* tighten */
  } else if (inflation_rate < 0.01) {
    b->reserve_ratio -= 0.001;  /* loosen */
  }
  if (b->reserve_ratio < CIV_BANKING_RESERVE_FLOOR)
    b->reserve_ratio = CIV_BANKING_RESERVE_FLOOR;
  if (b->reserve_ratio > CIV_BANKING_RESERVE_CEILING)
    b->reserve_ratio = CIV_BANKING_RESERVE_CEILING;

  /* Credit multiplier from reserve ratio */
  b->credit_multiplier = 1.0 / (b->reserve_ratio + 0.001);

  /* Money supply grows with GDP, contracts with rate hikes */
  b->money_supply *= (1.0 + gdp_growth * 0.5 - (b->base_interest_rate - 0.02) * 0.1);

  /* Calculate default rate from unemployment */
  b->default_rate = unemployment_rate * 0.6;
  if (b->default_rate > 1.0) b->default_rate = 1.0;

  /* Process loan repayments */
  civ_banking_repay_loans(b, time_delta);
}

civ_loan_t *civ_banking_issue_loan(civ_banking_system_t *b,
                                   const char *borrower_id,
                                   civ_float_t amount,
                                   int term_months) {
  if (!b || !borrower_id || amount <= 0 || term_months <= 0) return NULL;

  /* Expand loan array if needed */
  if (b->loan_count >= b->loan_capacity) {
    int new_cap = b->loan_capacity * 2;
    civ_loan_t *tmp = CIV_REALLOC(b->loans, sizeof(civ_loan_t) * new_cap);
    if (!tmp) return NULL;
    b->loans = tmp;
    b->loan_capacity = new_cap;
  }

  civ_loan_t *loan = &b->loans[b->loan_count];
  memset(loan, 0, sizeof(*loan));

  /* Loan ID from count */
  snprintf(loan->id, STRING_SHORT_LEN, "LOAN_%d", b->loan_count);
  strncpy(loan->borrower_id, borrower_id, STRING_SHORT_LEN - 1);

  loan->principal    = amount;
  loan->remaining    = amount;
  loan->term_months  = term_months;
  loan->months_remaining = term_months;
  loan->performing   = true;

  /* Interest rate scales with base rate + risk premium */
  loan->risk_tier = (amount > b->money_supply * 0.001) ? CIV_LOAN_RISK_HIGH :
                    (amount > b->money_supply * 0.0001) ? CIV_LOAN_RISK_MEDIUM :
                    CIV_LOAN_RISK_LOW;

  loan->interest_rate = civ_banking_get_lending_rate(b, loan->risk_tier);

  /* Monthly payment (amortized) */
  if (loan->interest_rate > 0.0 && term_months > 0) {
    civ_float_t monthly_rate = loan->interest_rate / 12.0;
    civ_float_t denom = 1.0 - pow(1.0 + monthly_rate, (civ_float_t)(-term_months));
    if (denom > 0.0)
      loan->monthly_payment = amount * monthly_rate / denom;
    else
      loan->monthly_payment = amount / (civ_float_t)term_months;
  } else {
    loan->monthly_payment = amount / (civ_float_t)term_months;
  }

  b->lending_volume += amount;
  b->loan_count++;
  return loan;
}

void civ_banking_repay_loans(civ_banking_system_t *b, civ_float_t time_delta) {
  if (!b) return;
  (void)time_delta;

  for (int i = 0; i < b->loan_count; i++) {
    civ_loan_t *loan = &b->loans[i];
    if (!loan->performing || loan->remaining <= 0) continue;

    /* Check for default based on risk tier */
    civ_float_t default_chance = 0.0;
    switch (loan->risk_tier) {
      case CIV_LOAN_RISK_LOW:      default_chance = 0.002; break;
      case CIV_LOAN_RISK_MEDIUM:   default_chance = 0.01;  break;
      case CIV_LOAN_RISK_HIGH:     default_chance = 0.03;  break;
      case CIV_LOAN_RISK_PREDATORY: default_chance = 0.08;  break;
      default: break;
    }

    /* Default chance scales with banking system default rate */
    default_chance += b->default_rate * 0.1;

    civ_float_t roll = (civ_float_t)rand() / (civ_float_t)RAND_MAX;
    if (roll < default_chance) {
      loan->performing = false;
      b->lending_volume -= loan->remaining;
      continue;
    }

    /* Pay down principal */
    loan->remaining -= loan->monthly_payment;
    loan->months_remaining--;
    b->lending_volume -= loan->monthly_payment;

    if (loan->remaining <= 0 || loan->months_remaining <= 0) {
      loan->remaining = 0;
      loan->months_remaining = 0;
    }
  }
}

civ_float_t civ_banking_credit_available(const civ_banking_system_t *b) {
  if (!b) return 0.0;
  civ_float_t max_lending = b->reserve_holdings * b->credit_multiplier;
  civ_float_t available = max_lending - b->lending_volume;
  return available > 0 ? available : 0;
}

civ_float_t civ_banking_get_lending_rate(const civ_banking_system_t *b, civ_loan_risk_t risk) {
  if (!b) return 0.05;
  civ_float_t risk_premium[] = { 0.005, 0.02, 0.06, 0.15 };
  int idx = (int)risk;
  if (idx < 0) idx = 0;
  if (idx >= CIV_LOAN_RISK_COUNT) idx = CIV_LOAN_RISK_COUNT - 1;
  return b->base_interest_rate + risk_premium[idx];
}

civ_float_t civ_banking_get_savings_rate(const civ_banking_system_t *b) {
  if (!b) return 0.01;
  return b->base_interest_rate * 0.6;  /* savings rate below lending rate */
}

civ_float_t civ_banking_get_money_multiplier(const civ_banking_system_t *b) {
  if (!b) return 10.0;
  return b->credit_multiplier;
}
