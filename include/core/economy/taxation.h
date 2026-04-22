/**
 * @file taxation.h
 * @brief Taxation — brackets, types, collection efficiency, evasion
 *
 * Tax burden affects population happiness, economic growth, and government
 * revenue. Collection efficiency depends on governance quality and corruption.
 */
#ifndef CIV_ECONOMY_TAXATION_H
#define CIV_ECONOMY_TAXATION_H

#include "../../common.h"
#include "../../types.h"

/* Tax types */
typedef enum {
  CIV_TAX_INCOME = 0,
  CIV_TAX_CORPORATE,
  CIV_TAX_PROPERTY,
  CIV_TAX_SALES,
  CIV_TAX_TARIFF,
  CIV_TAX_LUXURY,
  CIV_TAX_INHERITANCE,
  CIV_TAX_TYPE_COUNT
} civ_tax_type_t;

/* Progressive income tax bracket */
typedef struct {
  civ_float_t threshold;       /* income level where this bracket starts */
  civ_float_t rate;            /* 0.0 - 1.0 */
} civ_tax_bracket_t;

/* Single tax policy */
typedef struct {
  civ_tax_type_t    type;
  bool              active;
  civ_float_t       flat_rate;          /* for non-progressive types */
  civ_tax_bracket_t *brackets;         /* dynamic array for income/corporate */
  int                bracket_count;
  int                bracket_capacity;
  civ_float_t       collection_efficiency; /* 0.0-1.0, affected by governance */
  civ_float_t       evasion_rate;         /* fraction lost to evasion */
} civ_tax_policy_t;

/* National tax authority */
typedef struct {
  civ_tax_policy_t policies[CIV_TAX_TYPE_COUNT];
  civ_float_t      total_revenue;       /* collected last cycle */
  civ_float_t      projected_revenue;   /* estimated next cycle */
  civ_float_t      tax_burden_index;    /* 0.0-1.0 overall burden on population */
  civ_float_t      gini_coefficient;    /* inequality measure */
} civ_taxation_system_t;

/* --- Lifecycle --- */
civ_taxation_system_t *civ_taxation_create(void);
void                    civ_taxation_destroy(civ_taxation_system_t *t);

/* --- Update --- */
void civ_taxation_update(civ_taxation_system_t *t,
                         civ_float_t time_delta,
                         civ_float_t total_gdp,
                         civ_float_t population,
                         civ_float_t governance_efficiency,
                         civ_float_t corruption_level);

/* --- Policy --- */
void civ_taxation_set_policy(civ_taxation_system_t *t,
                             civ_tax_type_t type,
                             civ_float_t flat_rate);
void civ_taxation_add_bracket(civ_taxation_system_t *t,
                              civ_tax_type_t type,
                              civ_float_t threshold,
                              civ_float_t rate);
void civ_taxation_toggle_tax(civ_taxation_system_t *t, civ_tax_type_t type, bool active);

/* --- Query --- */
civ_float_t civ_taxation_calculate_income_tax(const civ_taxation_system_t *t, civ_float_t income);
civ_float_t civ_taxation_effective_tax_rate(const civ_taxation_system_t *t);
civ_float_t civ_taxation_revenue_forecast(const civ_taxation_system_t *t);

#endif /* CIV_ECONOMY_TAXATION_H */
