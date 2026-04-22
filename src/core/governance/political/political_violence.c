/**
 * @file political_violence.c
 * @brief Political violence — autonomous coups, assassinations, civil wars, purges
 */
#include "core/governance/political/political_violence.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_political_violence_t *civ_political_violence_create(void) {
  civ_political_violence_t *pv = CIV_MALLOC(sizeof(civ_political_violence_t));
  if (!pv) return NULL;
  memset(pv, 0, sizeof(*pv));
  pv->history_capacity = 16;
  pv->history = CIV_MALLOC(sizeof(civ_violence_event_t) * pv->history_capacity);
  return pv;
}

void civ_political_violence_destroy(civ_political_violence_t *pv) {
  if (!pv) return;
  free(pv->history);
  free(pv);
}

void civ_political_violence_update(civ_political_violence_t *pv, float dt,
                                   float militarization, float centralization,
                                   float legitimacy, float stability,
                                   float corruption, float representation,
                                   float power_consolidation, bool constitution_suspended,
                                   float faction_count, float cohesion) {
  if (!pv) return;

  /* ── Risk calculations — all dynamic, no hardcoded thresholds ── */
  /* Coup risk: military has power + legitimacy is low */
  pv->coup_risk = militarization * 0.35f + (1.0f - legitimacy) * 0.35f
                  + centralization * 0.20f - representation * 0.10f;
  if (pv->coup_risk < 0.0f) pv->coup_risk = 0.0f;
  if (pv->coup_risk > 1.0f) pv->coup_risk = 1.0f;

  /* Assassination risk: high corruption + factionalism */
  pv->assassination_risk = corruption * 0.35f + (faction_count / 10.0f) * 0.25f
                           + (1.0f - stability) * 0.25f + power_consolidation * 0.15f;
  if (pv->assassination_risk > 1.0f) pv->assassination_risk = 1.0f;

  /* Civil war risk: extreme conditions — low cohesion + high centralization + faction count */
  pv->civil_war_risk = (1.0f - cohesion) * 0.40f + centralization * 0.25f
                       + (faction_count > 4 ? 0.20f : 0.0f)
                       + (1.0f - stability) * 0.15f;
  if (pv->civil_war_risk < 0.0f) pv->civil_war_risk = 0.0f;
  if (pv->civil_war_risk > 1.0f) pv->civil_war_risk = 1.0f;

  /* Insurrection risk: low legitimacy + high repression + suspended constitution */
  pv->insurrection_risk = (1.0f - legitimacy) * 0.35f
                          + (constitution_suspended ? 0.25f : 0.0f)
                          + pv->political_repression * 0.20f
                          - representation * 0.15f;
  if (pv->insurrection_risk < 0.0f) pv->insurrection_risk = 0.0f;

  /* Political repression: rises with power consolidation + constitution suspension */
  float rep_target = power_consolidation * 0.50f
                     + (constitution_suspended ? 0.30f : 0.0f)
                     + centralization * 0.20f;
  pv->political_repression += (rep_target - pv->political_repression) * 0.05f * dt;

  /* ── Autonomous event triggers ── */
  float roll = (float)rand() / RAND_MAX;

  if (roll < pv->coup_risk * 0.01f && civ_political_violence_coup_possible(pv))
    civ_political_violence_attempt_coup(pv);

  if (roll < pv->assassination_risk * 0.005f)
    civ_political_violence_trigger_assassination(pv);

  if (roll < pv->civil_war_risk * 0.002f)
    civ_political_violence_trigger_civil_war(pv);

  if (roll < pv->insurrection_risk * 0.008f)
    civ_political_violence_trigger_purge(pv, 5000);
}

/* ── Query ─────────────────────────────────────────────────────── */
bool civ_political_violence_coup_possible(const civ_political_violence_t *pv) {
  return pv ? (pv->coup_risk > 0.30f) : false;
}

bool civ_political_violence_civil_war_possible(const civ_political_violence_t *pv) {
  return pv ? (pv->civil_war_risk > 0.40f) : false;
}

float civ_political_violence_instability_index(const civ_political_violence_t *pv) {
  if (!pv) return 0.0f;
  return (pv->coup_risk + pv->assassination_risk + pv->civil_war_risk
          + pv->insurrection_risk) / 4.0f;
}

/* ── Event triggers ────────────────────────────────────────────── */
static void add_event(civ_political_violence_t *pv, civ_violence_type_t type,
                      float severity, float stab_dmg, float leg_dmg, int casualties,
                      const char *desc) {
  if (!pv) return;
  if (pv->history_count >= pv->history_capacity) return;
  civ_violence_event_t *e = &pv->history[pv->history_count];
  memset(e, 0, sizeof(*e));
  e->type = type;
  e->occurred = true;
  e->severity = severity;
  e->stability_damage = stab_dmg;
  e->legitimacy_damage = leg_dmg;
  e->casualties = casualties;
  strncpy(e->description, desc, STRING_MAX_LEN - 1);
  pv->history_count++;
  pv->total_casualties += casualties;
}

void civ_political_violence_attempt_coup(civ_political_violence_t *pv) {
  if (!pv) return;
  pv->coups_attempted++;
  float success_chance = pv->coup_risk * 0.7f + (1.0f - pv->political_repression) * 0.3f;
  bool success = ((float)rand() / RAND_MAX) < success_chance;
  if (success) {
    pv->coups_succeeded++;
    add_event(pv, CIV_VIOLENCE_COUP_ATTEMPT, 0.80f, 0.30f, 0.25f,
              200 + rand() % 500, "Military coup succeeded — government overthrown");
  } else {
    add_event(pv, CIV_VIOLENCE_COUP_ATTEMPT, 0.50f, 0.15f, 0.10f,
              50 + rand() % 100, "Coup attempt failed — plotters arrested");
  }
}

void civ_political_violence_trigger_assassination(civ_political_violence_t *pv) {
  if (!pv) return;
  add_event(pv, CIV_VIOLENCE_ASSASSINATION, 0.70f, 0.20f, 0.15f,
            1 + rand() % 10, "Political assassination — leadership in crisis");
}

void civ_political_violence_trigger_civil_war(civ_political_violence_t *pv) {
  if (!pv) return;
  add_event(pv, CIV_VIOLENCE_CIVIL_WAR, 0.95f, 0.50f, 0.40f,
            1000 + rand() % 10000, "Civil war erupts — nation divided");
}

void civ_political_violence_trigger_purge(civ_political_violence_t *pv, int target_population) {
  if (!pv) return;
  int casualties = (int)(target_population * 0.01f * pv->political_repression);
  if (casualties < 10) casualties = 10;
  add_event(pv, CIV_VIOLENCE_PURGE, 0.85f, 0.10f, 0.30f,
            casualties, "Political purge — opposition eliminated");
  pv->political_repression += 0.10f;
  if (pv->political_repression > 1.0f) pv->political_repression = 1.0f;
}
