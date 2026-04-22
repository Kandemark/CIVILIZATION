/**
 * @file banking.h
 * @brief Central banking — reserve requirements, lending, interest rates, credit
 *
 * All rates and thresholds respond to inflation, GDP growth, and government
 * policy. No hardcoded "gold standard" — the banking system evolves with the economy.
 */
#ifndef CIV_ECONOMY_BANKING_H
#define CIV_ECONOMY_BANKING_H

#include "../../common.h"
#include "../../types.h"

/* Loan risk tiers */
typedef enum {
  CIV_LOAN_RISK_LOW = 0,
  CIV_LOAN_RISK_MEDIUM,
  CIV_LOAN_RISK_HIGH,
  CIV_LOAN_RISK_PREDATORY,
  CIV_LOAN_RISK_COUNT
} civ_loan_risk_t;

/* A single loan instrument */
typedef struct {
  char id[STRING_SHORT_LEN];
  char borrower_id[STRING_SHORT_LEN];
  civ_float_t principal;
  civ_float_t remaining;
  civ_float_t interest_rate;      /* APR, compounded per update cycle */
  civ_float_t monthly_payment;
  int        term_months;
  int        months_remaining;
  civ_loan_risk_t risk_tier;
  bool       performing;           /* false = in default */
} civ_loan_t;

/* Central bank / national reserve */
typedef struct {
  civ_float_t reserve_ratio;       /* fraction of deposits held in reserve */
  civ_float_t base_interest_rate;  /* central bank policy rate */
  civ_float_t money_supply;        /* M2 broad money */
  civ_float_t reserve_holdings;    /* total reserves on hand */
  civ_float_t lending_volume;      /* outstanding loan principal */
  civ_float_t default_rate;        /* fraction of loans in default */
  civ_loan_t *loans;               /* dynamic array */
  int         loan_count;
  int         loan_capacity;
  civ_float_t credit_multiplier;   /* money creation factor */
} civ_banking_system_t;

/* --- Lifecycle --- */
civ_banking_system_t *civ_banking_create(void);
void                   civ_banking_destroy(civ_banking_system_t *b);

/* --- Update --- */
void civ_banking_update(civ_banking_system_t *b,
                        civ_float_t time_delta,
                        civ_float_t inflation_rate,
                        civ_float_t gdp_growth,
                        civ_float_t unemployment_rate,
                        civ_float_t government_spending_ratio);

/* --- Lending --- */
civ_loan_t *civ_banking_issue_loan(civ_banking_system_t *b,
                                   const char *borrower_id,
                                   civ_float_t amount,
                                   int term_months);
void        civ_banking_repay_loans(civ_banking_system_t *b, civ_float_t time_delta);
civ_float_t civ_banking_credit_available(const civ_banking_system_t *b);

/* --- Query --- */
civ_float_t civ_banking_get_lending_rate(const civ_banking_system_t *b, civ_loan_risk_t risk);
civ_float_t civ_banking_get_savings_rate(const civ_banking_system_t *b);
civ_float_t civ_banking_get_money_multiplier(const civ_banking_system_t *b);

#endif /* CIV_ECONOMY_BANKING_H */
