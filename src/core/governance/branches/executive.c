/**
 * @file executive.c
 * @brief Executive branch — veto, decrees, orders, appointments
 */
#include "core/governance/branches/executive.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_executive_t *civ_executive_create(void) {
  civ_executive_t *e = CIV_MALLOC(sizeof(civ_executive_t));
  if (!e) return NULL;
  memset(e, 0, sizeof(*e));
  e->action_capacity = 8;
  e->actions = CIV_MALLOC(sizeof(civ_executive_action_t) * e->action_capacity);
  e->executive_strength = 0.50f;
  e->political_capital = 0.60f;
  e->decree_power = 0.20f;
  e->veto.veto_power = 0.50f;
  return e;
}

void civ_executive_destroy(civ_executive_t *e) {
  if (!e) return;
  free(e->actions);
  free(e);
}

void civ_executive_update(civ_executive_t *e, float dt,
                          float centralization, float legitimacy,
                          float emergency_power, float representation) {
  if (!e) return;

  /* Executive strength: centralization + emergency - representation */
  e->executive_strength = centralization * 0.40f + emergency_power * 0.30f
                          - representation * 0.20f + legitimacy * 0.20f + 0.25f;
  if (e->executive_strength < 0.10f) e->executive_strength = 0.10f;
  if (e->executive_strength > 0.95f) e->executive_strength = 0.95f;

  /* Political capital: legitimacy feeds it, actions spend it */
  e->political_capital += (legitimacy * 0.02f - (1.0f - e->political_capital) * 0.01f) * dt;
  if (e->political_capital < 0.05f) e->political_capital = 0.05f;
  if (e->political_capital > 1.0f)  e->political_capital = 1.0f;

  /* Decree power: centralization + emergency, constrained by representation */
  e->decree_power = centralization * 0.50f + emergency_power * 0.30f
                    - representation * 0.20f;
  if (e->decree_power < 0.0f)  e->decree_power = 0.0f;
  if (e->decree_power > 1.0f)  e->decree_power = 1.0f;
  e->can_issue_decrees = (e->decree_power > 0.30f);

  /* Constitution suspension: power consolidates over time */
  if (e->constitution_suspended) {
    e->turns_suspended++;
    e->power_consolidation += 0.003f * dt; /* slow drift toward autocracy */
    if (e->power_consolidation > 1.0f) e->power_consolidation = 1.0f;
    /* Long suspensions normalize — after ~80 turns, it's effectively permanent */
    e->decree_power += 0.002f * dt;
    if (e->decree_power > 1.0f) e->decree_power = 1.0f;
  }

  /* Veto power: strong executive can veto with less chance of override */
  e->veto.veto_power = e->executive_strength * 0.80f + emergency_power * 0.20f;

  /* Process active actions */
  civ_executive_process_actions(e, dt);
}

civ_executive_action_t *civ_executive_issue_order(civ_executive_t *e,
                                                   civ_exec_action_t type,
                                                   const char *title, float power_cost) {
  if (!e || !title || power_cost > e->political_capital) return NULL;
  if (e->action_count >= e->action_capacity) {
    int nc = e->action_capacity * 2;
    civ_executive_action_t *tmp = CIV_REALLOC(e->actions,
                                               sizeof(civ_executive_action_t) * nc);
    if (!tmp) return NULL;
    e->actions = tmp;
    e->action_capacity = nc;
  }
  civ_executive_action_t *a = &e->actions[e->action_count];
  memset(a, 0, sizeof(*a));
  snprintf(a->id, STRING_SHORT_LEN, "EXEC_%d", e->action_count);
  strncpy(a->title, title, STRING_MEDIUM_LEN - 1);
  a->type = type;
  a->status = CIV_EXEC_ISSUED;
  a->power_cost = power_cost;
  a->effect_magnitude = power_cost * 0.80f + e->executive_strength * 0.20f;
  a->turns_remaining = (type == CIV_EXEC_DECREE) ? 40 : 10;
  a->is_permanent = (type == CIV_EXEC_APPOINTMENT);
  e->political_capital -= power_cost;
  e->action_count++;
  if (type == CIV_EXEC_APPOINTMENT) e->appointments_made++;
  if (type == CIV_EXEC_PARDON) e->pardons_issued++;
  return a;
}

civ_executive_action_t *civ_executive_issue_decree(civ_executive_t *e,
                                                    const char *title, float effect) {
  if (!e || !e->can_issue_decrees) return NULL;
  return civ_executive_issue_order(e, CIV_EXEC_DECREE, title, effect * 0.3f);
}

bool civ_executive_veto_bill(civ_executive_t *e, const char *bill_id) {
  if (!e || !bill_id) return false;
  e->veto.vetoes_issued++;
  e->veto.bills_awaiting_signature--;
  e->political_capital -= 0.05f;
  return true;
}

bool civ_executive_override_possible(const civ_executive_t *e, float legislative_support) {
  if (!e) return true; /* no executive = no veto */
  /* Override requires support > veto_power threshold */
  float override_threshold = 0.50f + e->veto.veto_power * 0.35f;
  return legislative_support > override_threshold;
}

void civ_executive_sign_bill(civ_executive_t *e) {
  if (!e) return;
  e->veto.bills_awaiting_signature--;
  e->political_capital += 0.02f;
}

void civ_executive_process_actions(civ_executive_t *e, float dt) {
  if (!e) return;
  (void)dt;
  for (int i = 0; i < e->action_count; i++) {
    civ_executive_action_t *a = &e->actions[i];
    if (a->status != CIV_EXEC_ISSUED && a->status != CIV_EXEC_CHALLENGED) continue;
    if (!a->is_permanent) {
      a->turns_remaining--;
      if (a->turns_remaining <= 0) {
        a->status = CIV_EXEC_EXPIRED;
      }
    }
  }
}

float civ_executive_power_index(const civ_executive_t *e) {
  return e ? e->executive_strength : 0.50f;
}

/* ── Constitution suspension ──────────────────────────────────── */
bool civ_executive_suspend_constitution(civ_executive_t *e) {
  if (!e) return false;
  if (!e->can_issue_decrees && e->power_consolidation < 0.60f) return false;
  e->constitution_suspended = true;
  e->turns_suspended = 0;
  e->power_consolidation += 0.15f;
  e->political_capital -= 0.20f;
  return true;
}

bool civ_executive_restore_constitution(civ_executive_t *e) {
  if (!e || !e->constitution_suspended) return false;
  /* Ruler may resist — the more power consolidated, the harder to restore */
  float resistance = e->power_consolidation * 0.8f;
  float restoration = e->political_capital * 0.3f + (1.0f - e->power_consolidation) * 0.4f;
  if (restoration < resistance) return false;
  e->constitution_suspended = false;
  e->turns_suspended = 0;
  e->power_consolidation -= 0.10f;
  return true;
}

bool civ_executive_is_constitution_suspended(const civ_executive_t *e) {
  return e ? e->constitution_suspended : false;
}

float civ_executive_power_consolidation(const civ_executive_t *e) {
  return e ? e->power_consolidation : 0.0f;
}

void civ_executive_consolidate_power(civ_executive_t *e, float amount) {
  if (!e) return;
  e->power_consolidation += amount;
  if (e->power_consolidation > 1.0f) e->power_consolidation = 1.0f;
  if (e->power_consolidation < 0.0f) e->power_consolidation = 0.0f;
}

bool civ_executive_can_rule_by_decree(const civ_executive_t *e) {
  return e ? (e->constitution_suspended || e->can_issue_decrees
              || e->power_consolidation > 0.70f) : false;
}

bool civ_executive_is_autocratic_takeover(const civ_executive_t *e) {
  return e ? (e->power_consolidation > 0.75f && e->constitution_suspended
              && e->turns_suspended > 40) : false;
}
