#include "core/economy/war_economy.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>

civ_war_economy_system_t *civ_war_economy_create(void) {
  civ_war_economy_system_t *w = CIV_MALLOC(sizeof(civ_war_economy_system_t));
  if (!w) return NULL;
  memset(w, 0, sizeof(*w));
  w->level = CIV_MOB_PEACETIME;
  w->military_spending_ratio = 0.02;
  w->rationing_level = 0.0;
  w->conscription_rate = 0.0;
  w->war_materiel_stockpile = 1000.0;
  return w;
}

void civ_war_economy_destroy(civ_war_economy_system_t *w) { free(w); }

void civ_war_economy_update(civ_war_economy_system_t *w, civ_float_t time_delta,
                            civ_float_t gdp, civ_float_t industrial_output,
                            int population, bool actively_fighting) {
  if (!w) return;
  (void)time_delta;
  (void)population;

  if (!w->at_war && !actively_fighting) {
    /* Peacetime: gradual demobilization */
    w->military_spending_ratio += (0.02 - w->military_spending_ratio) * 0.1;
    w->civilian_to_military_conversion *= 0.95;
    w->rationing_level *= 0.9;
    w->conscription_rate *= 0.9;
    w->months_at_war = 0;
    w->war_exhaustion *= 0.95;
  } else {
    w->at_war = true;
    if (actively_fighting) w->months_at_war++;

    /* Mobilization effects based on level */
    switch (w->level) {
      case CIV_MOB_PEACETIME:
        w->military_spending_ratio = 0.03;
        w->civilian_economy_penalty = 0.0;
        break;
      case CIV_MOB_PARTIAL:
        w->military_spending_ratio = 0.12;
        w->civilian_economy_penalty = 0.08;
        w->civilian_to_military_conversion = 0.10;
        w->conscription_rate = 0.02;
        break;
      case CIV_MOB_FULL:
        w->military_spending_ratio = 0.30;
        w->civilian_economy_penalty = 0.20;
        w->civilian_to_military_conversion = 0.40;
        w->rationing_level = 0.30;
        w->conscription_rate = 0.08;
        break;
      case CIV_MOB_TOTAL:
        w->military_spending_ratio = 0.60;
        w->civilian_economy_penalty = 0.45;
        w->civilian_to_military_conversion = 0.75;
        w->rationing_level = 0.70;
        w->conscription_rate = 0.20;
        break;
    }

    /* War exhaustion: increases with time at war + rationing */
    w->war_exhaustion += (0.01 + w->rationing_level * 0.03
                          + w->civilian_economy_penalty * 0.02);
    if (w->war_exhaustion > 1.0) w->war_exhaustion = 1.0;
  }

  /* Materiel production from converted industry */
  civ_float_t military_industry = industrial_output * w->civilian_to_military_conversion;
  civ_war_economy_produce_materiel(w, military_industry);

  /* Materiel consumption in combat */
  if (actively_fighting) {
    w->materiel_consumption_rate = w->war_materiel_stockpile * 0.05;
    w->war_materiel_stockpile -= w->materiel_consumption_rate;
    if (w->war_materiel_stockpile < 0) w->war_materiel_stockpile = 0;
  }
}

void civ_war_economy_set_mobilization(civ_war_economy_system_t *w, civ_mobilization_level_t level) {
  if (!w) return;
  w->level = level;
  w->at_war = (level != CIV_MOB_PEACETIME);
}

void civ_war_economy_produce_materiel(civ_war_economy_system_t *w, civ_float_t industrial_input) {
  if (!w) return;
  w->war_materiel_stockpile += industrial_input * 0.5; /* conversion efficiency */
}

civ_float_t civ_war_economy_gdp_penalty(const civ_war_economy_system_t *w) {
  return w ? w->civilian_economy_penalty : 0.0;
}

bool civ_war_economy_is_sustainable(const civ_war_economy_system_t *w) {
  return w ? (w->war_exhaustion < 0.8) : true;
}
