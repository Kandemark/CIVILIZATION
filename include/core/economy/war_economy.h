/**
 * @file war_economy.h
 * @brief War economy — mobilization, rationing, military procurement, conversion
 */
#ifndef CIV_ECONOMY_WAR_ECONOMY_H
#define CIV_ECONOMY_WAR_ECONOMY_H

#include "../../common.h"
#include "../../types.h"

typedef enum { CIV_MOB_PEACETIME, CIV_MOB_PARTIAL, CIV_MOB_FULL, CIV_MOB_TOTAL } civ_mobilization_level_t;

typedef struct {
  civ_mobilization_level_t level;
  civ_float_t              military_spending_ratio;  /* fraction of budget */
  civ_float_t              civilian_to_military_conversion; /* factory conversion % */
  civ_float_t              rationing_level;          /* 0.0-1.0 */
  civ_float_t              conscription_rate;        /* fraction of workforce drafted */
  civ_float_t              war_materiel_stockpile;
  civ_float_t              materiel_consumption_rate;
  civ_float_t              civilian_economy_penalty; /* GDP hit from mobilization */
  civ_float_t              war_exhaustion;           /* 0.0-1.0 */
  int                      months_at_war;
  bool                     at_war;
} civ_war_economy_system_t;

civ_war_economy_system_t *civ_war_economy_create(void);
void civ_war_economy_destroy(civ_war_economy_system_t *w);
void civ_war_economy_update(civ_war_economy_system_t *w, civ_float_t time_delta,
                            civ_float_t gdp, civ_float_t industrial_output,
                            int population, bool actively_fighting);

void civ_war_economy_set_mobilization(civ_war_economy_system_t *w, civ_mobilization_level_t level);
void civ_war_economy_produce_materiel(civ_war_economy_system_t *w, civ_float_t industrial_input);
civ_float_t civ_war_economy_gdp_penalty(const civ_war_economy_system_t *w);
bool civ_war_economy_is_sustainable(const civ_war_economy_system_t *w);

#endif
