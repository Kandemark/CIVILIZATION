/**
 * @file economic_policy.h
 * @brief Economic policy — subsidies, price controls, regulations, minimum wage
 *
 * Policy levers that affect market behavior. Every lever has tradeoffs.
 * Price caps cause shortages. Subsidies drain the budget. Deregulation
 * boosts growth but increases inequality and corruption risk.
 */
#ifndef CIV_ECONOMY_ECONOMIC_POLICY_H
#define CIV_ECONOMY_ECONOMIC_POLICY_H

#include "../../common.h"
#include "../../types.h"

/* Policy instrument types */
typedef enum {
  CIV_POLICY_SUBSIDY = 0,
  CIV_POLICY_PRICE_CAP,
  CIV_POLICY_PRICE_FLOOR,
  CIV_POLICY_MINIMUM_WAGE,
  CIV_POLICY_INTEREST_CAP,
  CIV_POLICY_IMPORT_QUOTA,
  CIV_POLICY_EXPORT_SUBSIDY,
  CIV_POLICY_INVESTMENT_TAX_CREDIT,
  CIV_POLICY_TYPE_COUNT
} civ_policy_type_t;

/* Regulation stance per sector */
typedef enum {
  CIV_REGULATION_NONE = 0,
  CIV_REGULATION_LIGHT,
  CIV_REGULATION_MODERATE,
  CIV_REGULATION_HEAVY,
  CIV_REGULATION_STATE_CONTROLLED,
  CIV_REGULATION_COUNT
} civ_regulation_level_t;

/* A single active policy instrument */
typedef struct {
  civ_policy_type_t type;
  bool              active;
  civ_float_t       magnitude;     /* intensity of the policy 0.0-1.0 */
  civ_float_t       cost;          /* government cost per cycle */
  civ_float_t       effectiveness; /* 0.0-1.0, affected by corruption */
  char              target_sector[STRING_SHORT_LEN];
} civ_policy_instrument_t;

/* Policy engine */
typedef struct {
  civ_policy_instrument_t instruments[CIV_POLICY_TYPE_COUNT];
  civ_regulation_level_t   regulation;        /* overall regulatory stance */
  civ_float_t              minimum_wage;      /* hourly */
  civ_float_t              total_policy_cost; /* sum of all active instruments */
  civ_float_t              consumer_confidence; /* 0.0-1.0 */
  civ_float_t              business_confidence; /* 0.0-1.0 */
  civ_float_t              economic_freedom_index; /* composite */
} civ_economic_policy_system_t;

/* --- Lifecycle --- */
civ_economic_policy_system_t *civ_economic_policy_create(void);
void                           civ_economic_policy_destroy(civ_economic_policy_system_t *p);

/* --- Update --- */
void civ_economic_policy_update(civ_economic_policy_system_t *p,
                                civ_float_t time_delta,
                                civ_float_t inflation_rate,
                                civ_float_t unemployment_rate,
                                civ_float_t gdp_growth,
                                civ_float_t corruption_level);

/* --- Instruments --- */
void civ_economic_policy_set_instrument(civ_economic_policy_system_t *p,
                                        civ_policy_type_t type,
                                        bool active,
                                        civ_float_t magnitude,
                                        const char *target_sector);
void civ_economic_policy_set_regulation(civ_economic_policy_system_t *p,
                                        civ_regulation_level_t level);

/* --- Query --- */
civ_float_t civ_economic_policy_subsidy_multiplier(const civ_economic_policy_system_t *p,
                                                    const char *sector);
civ_float_t civ_economic_policy_price_modifier(const civ_economic_policy_system_t *p,
                                                const char *commodity);
civ_float_t civ_economic_policy_growth_modifier(const civ_economic_policy_system_t *p);
civ_float_t civ_economic_policy_inequality_modifier(const civ_economic_policy_system_t *p);

#endif /* CIV_ECONOMY_ECONOMIC_POLICY_H */
