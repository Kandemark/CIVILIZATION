/**
 * @file financial_markets.h
 * @brief Financial markets — forex, stock exchange, multi-currency wallets
 *
 * Currencies are dynamic ISO-code instruments with fluctuating exchange rates.
 * Income is in local currency — value changes with the global market.
 * Player holds a multi-currency wallet.
 */
#ifndef CIV_ECONOMY_FINANCIAL_MARKETS_H
#define CIV_ECONOMY_FINANCIAL_MARKETS_H

#include "../../common.h"
#include "../../types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_CURRENCY_MAX   280
#define CIV_CURRENCY_CODE  4
#define CIV_CURRENCY_NAME  48
#define CIV_WALLET_SLOTS   8

/* ── Currency ──────────────────────────────────────────────────── */
typedef struct {
  char     iso[CIV_CURRENCY_CODE];    /* "KES", "USD", "EUR" */
  char     name[CIV_CURRENCY_NAME];   /* "Kenyan Shilling" */
  char     symbol[4];                 /* "KSh", "$", "€" */
  float    base_rate;                 /* 1.0 = reference value */
  float    current_rate;              /* fluctuates each turn */
  float    volatility;                /* how much it swings */
  int32_t  nation_count;              /* how many nations use this */
  float    inflation;                 /* current inflation rate */
} civ_market_currency_t;

/* ── Commodity ──────────────────────────────────────────────────── */
typedef struct {
  char     name[32];       /* "Gold", "Crude Oil", "Wheat" */
  char     unit[8];        /* "oz", "barrel", "tonne" */
  float    price_per_unit; /* in reference currency */
  float    volatility;
  float    supply_index;   /* 0-1, affects price */
  float    demand_index;   /* 0-1, affects price */
} civ_commodity_t;

/* ── Wallet entry ───────────────────────────────────────────────── */

/* ── Company ────────────────────────────────────────────────────── */
#define CIV_COMPANY_MAX 64
typedef struct {
  char     name[48];
  char     industry[32];
  char     nation_id[32];
  int32_t  employees;
  float    revenue;
  float    stock_price;
  bool     is_public;
  float    growth_rate;
} civ_company_t;
typedef struct {
  char   currency_iso[4];
  float  balance;          /* amount held in this currency */
} civ_wallet_entry_t;

/* ── Player wallet ──────────────────────────────────────────────── */
typedef struct {
  civ_wallet_entry_t slots[CIV_WALLET_SLOTS];
  int                count;
  float              total_value_ref; /* total in reference currency */
} civ_wallet_t;

/* ── Market engine ──────────────────────────────────────────────── */
typedef struct {
  civ_market_currency_t  currencies[CIV_CURRENCY_MAX];
  int             currency_count;
  civ_commodity_t commodities[12];
  civ_company_t    companies[CIV_COMPANY_MAX];
  int              company_count;
  int             commodity_count;
} civ_market_engine_t;

/* ── API ────────────────────────────────────────────────────────── */
civ_market_engine_t *civ_market_create(void);
void                 civ_market_destroy(civ_market_engine_t *m);

/* Advance one turn — fluctuate rates and prices */
void civ_market_update(civ_market_engine_t *m);

/* Apply real production data to commodity prices and currency strength.
   Pass the global civ_nation_economy_t aggregate as void*. */
void civ_market_apply_production(civ_market_engine_t *m,
                                  float global_gdp, float global_food,
                                  float global_energy, float global_industrial);

/* Currency lookup */
civ_market_currency_t *civ_market_get_currency(civ_market_engine_t *m, const char *iso);
float           civ_market_exchange(civ_market_engine_t *m,
                                    const char *from_iso, const char *to_iso,
                                    float amount);

/* Wallet */
void  civ_wallet_init(civ_wallet_t *w);
void  civ_wallet_add(civ_wallet_t *w, const char *iso, float amount);
float civ_wallet_balance(civ_wallet_t *w, const char *iso);
float civ_wallet_total(civ_wallet_t *w, civ_market_engine_t *m);
void civ_wallet_remove(civ_wallet_t *w, const char *iso);
bool civ_wallet_exchange(civ_wallet_t *w, civ_market_engine_t *m,
                         const char *from_iso, const char *to_iso, float amount);

/* Commodity */
civ_commodity_t *civ_market_get_commodity(civ_market_engine_t *m, const char *name);
void civ_market_generate_companies(civ_market_engine_t *m, const char *nation_id);
civ_company_t *civ_market_get_company(civ_market_engine_t *m, int idx);
float civ_market_cost_of_living(civ_market_engine_t *m, const char *iso);

/* Compute dynamic price scaled by cost of living and currency */
float civ_market_dynamic_price(civ_market_engine_t *m, float base_price,
                               const char *currency_iso, float cost_of_living);


/* Add a currency dynamically */
civ_market_currency_t *civ_market_add_currency(civ_market_engine_t *m,
    const char *iso, const char *name, const char *symbol, float rate);

#ifdef __cplusplus
}
#endif
#endif
