/**
 * @file ministry.c
 * @brief Ministry system — NPC ministers, budget competition, reforms
 */
#include "core/governance/institutions/ministry.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void civ_ministry_approve_reform(civ_reform_proposal_t *r);
static void civ_ministry_reject_reform(civ_reform_proposal_t *r);

civ_ministry_manager_t *civ_ministry_manager_create(void) {
  civ_ministry_manager_t *mgr = CIV_MALLOC(sizeof(civ_ministry_manager_t));
  if (!mgr) return NULL;
  memset(mgr, 0, sizeof(*mgr));
  mgr->ministry_capacity = CIV_MINISTRY_TYPE_COUNT;
  mgr->ministries = CIV_MALLOC(sizeof(civ_ministry_t) * mgr->ministry_capacity);
  return mgr;
}

void civ_ministry_manager_destroy(civ_ministry_manager_t *mgr) {
  if (!mgr) return;
  for (int i = 0; i < mgr->ministry_count; i++)
    free(mgr->ministries[i].proposals);
  free(mgr->ministries);
  free(mgr);
}

civ_ministry_t *civ_ministry_create(civ_ministry_type_t type, const char *minister_name) {
  civ_ministry_t *m = CIV_MALLOC(sizeof(civ_ministry_t));
  if (!m) return NULL;
  memset(m, 0, sizeof(*m));
  m->type  = type;
  m->efficiency = 0.45f;
  m->budget     = 1000.0f;
  m->power_index = 0.30f;
  strncpy(m->minister.name, minister_name ? minister_name : "Vacant", STRING_MEDIUM_LEN - 1);

  /* Personalities differ slightly per ministry — randomized at creation */
  m->minister.competence = 0.40f + (float)(rand() % 40) / 100.0f;
  m->minister.loyalty    = 0.50f + (float)(rand() % 30) / 100.0f;
  m->minister.ambition   = 0.20f + (float)(rand() % 50) / 100.0f;
  m->minister.corruption_vulnerability = 0.05f + (float)(rand() % 20) / 100.0f;

  m->proposal_capacity = 4;
  m->proposals = CIV_MALLOC(sizeof(civ_reform_proposal_t) * m->proposal_capacity);
  return m;
}

/* ====================================================================
 * PER-TICK UPDATE — all ministries processed
 * ==================================================================== */
void civ_ministry_update_all(civ_ministry_manager_t *mgr, float total_budget,
                             float gov_efficiency, float corruption, float dt) {
  if (!mgr || mgr->ministry_count == 0) return;
  mgr->total_budget = total_budget;

  /* Equal base share, modified by power_index */
  float base_share = total_budget / (float)mgr->ministry_count;

  /* Compute budget pressure: how contested funds are */
  float total_claimed = 0.0f;
  for (int i = 0; i < mgr->ministry_count; i++)
    total_claimed += base_share * (0.7f + mgr->ministries[i].power_index * 0.6f);
  mgr->budget_pressure = (total_budget > 0) ? total_claimed / total_budget : 1.0f;
  if (mgr->budget_pressure > 2.0f) mgr->budget_pressure = 2.0f;

  for (int i = 0; i < mgr->ministry_count; i++) {
    civ_ministry_t *m = &mgr->ministries[i];

    /* Budget allocation: power_index influences share */
    m->budget = base_share * (0.7f + m->power_index * 0.6f);
    if (mgr->budget_pressure > 1.0f)
      m->budget /= mgr->budget_pressure;

    /* Efficiency: minister competence * governance efficiency */
    float target_eff = m->minister.competence * 0.5f + gov_efficiency * 0.3f
                       + (m->budget / (base_share + 1.0f)) * 0.2f;
    m->efficiency += (target_eff - m->efficiency) * 0.1f * dt;

    /* Corruption: personal vulnerability * systemic corruption */
    m->corruption_level += (m->minister.corruption_vulnerability * corruption
                            + (1.0f - m->efficiency) * 0.01f
                            - m->corruption_level) * 0.05f * dt;
    if (m->corruption_level < 0.0f) m->corruption_level = 0.0f;
    if (m->corruption_level > 0.5f) m->corruption_level = 0.5f;

    /* Domain output: budget * efficiency * (1 - corruption) */
    m->domain_output = m->budget * m->efficiency * (1.0f - m->corruption_level);

    /* Public satisfaction: driven by output relative to expectation */
    float expected = base_share * 0.8f;
    m->public_satisfaction += ((m->domain_output / (expected + 1.0f))
                                - m->public_satisfaction) * 0.1f * dt;
    if (m->public_satisfaction < 0.1f) m->public_satisfaction = 0.1f;
    if (m->public_satisfaction > 1.0f) m->public_satisfaction = 1.0f;

    /* Minister tenure */
    m->minister.tenure_turns++;

    /* Process reforms */
    civ_ministry_process_reforms(m, m->budget * 0.1f, dt);

    /* Ambition: high-ambition ministers may propose reforms unprompted */
    if (m->minister.ambition > 0.60f && m->proposal_count < 3
        && (rand() % 100) < (int)(m->minister.ambition * 5.0f)) {
      civ_ministry_propose_reform(m, "Efficiency Reform", "Restructure operations");
    }
  }
}

/* ── Minister management ─────────────────────────────────────────── */
void civ_ministry_appoint_minister(civ_ministry_t *m, const char *npc_id,
                                   const char *name, float comp, float loy, float amb) {
  if (!m) return;
  if (npc_id) strncpy(m->minister.npc_id, npc_id, STRING_SHORT_LEN - 1);
  if (name)   strncpy(m->minister.name, name, STRING_MEDIUM_LEN - 1);
  m->minister.competence = comp;
  m->minister.loyalty    = loy;
  m->minister.ambition   = amb;
  m->minister.tenure_turns = 0;
}

void civ_ministry_remove_minister(civ_ministry_t *m) {
  if (!m) return;
  m->minister.npc_id[0] = '\0';
  strcpy(m->minister.name, "Vacant");
  m->minister.competence = 0.20f;
  m->minister.loyalty    = 0.30f;
  m->minister.ambition   = 0.10f;
  m->minister.tenure_turns = 0;
  m->efficiency *= 0.70f;
}

bool civ_ministry_minister_may_defect(const civ_ministry_t *m, float legitimacy_crisis) {
  if (!m) return false;
  float defect_score = (1.0f - m->minister.loyalty) * 0.5f
                       + m->minister.ambition * 0.3f
                       + legitimacy_crisis * 0.2f
                       + m->corruption_level * 0.2f;
  return defect_score > 0.55f;
}

/* ── Reforms ─────────────────────────────────────────────────────── */
civ_reform_proposal_t *civ_ministry_propose_reform(civ_ministry_t *m,
                                                    const char *title, const char *desc) {
  if (!m || !title) return NULL;
  if (m->proposal_count >= m->proposal_capacity) {
    int nc = m->proposal_capacity * 2;
    civ_reform_proposal_t *tmp = CIV_REALLOC(m->proposals, sizeof(civ_reform_proposal_t) * nc);
    if (!tmp) return NULL;
    m->proposals = tmp;
    m->proposal_capacity = nc;
  }
  civ_reform_proposal_t *r = &m->proposals[m->proposal_count];
  memset(r, 0, sizeof(*r));
  snprintf(r->id, STRING_SHORT_LEN, "REF_%s_%d", m->minister.name, m->proposal_count);
  strncpy(r->title, title, STRING_MEDIUM_LEN - 1);
  if (desc) strncpy(r->description, desc, STRING_MAX_LEN - 1);

  /* Cost and impact scale with minister competence */
  r->cost   = 500.0f + m->minister.competence * 2000.0f;
  r->impact = 0.05f + m->minister.competence * 0.20f + m->minister.ambition * 0.10f;
  r->ongoing_cost = r->cost * 0.15f;
  r->status = CIV_REFORM_PENDING;
  m->proposal_count++;
  return r;
}

void civ_ministry_process_reforms(civ_ministry_t *m, float available_budget, float dt) {
  if (!m) return;
  for (int i = 0; i < m->proposal_count; i++) {
    civ_reform_proposal_t *r = &m->proposals[i];
    if (r->status == CIV_REFORM_PENDING && available_budget > r->cost) {
      civ_ministry_approve_reform(r);
    }
    if (r->status == CIV_REFORM_ACTIVE) {
      r->implementation_progress += (0.05f + m->efficiency * 0.10f) * dt;
      r->turns_active++;
      if (r->implementation_progress >= 1.0f) {
        r->implementation_progress = 1.0f;
        r->status = CIV_REFORM_COMPLETE;
        m->reforms_completed++;
        m->efficiency += r->impact * 0.3f;
        if (m->efficiency > 0.95f) m->efficiency = 0.95f;
        m->power_index += r->impact * 0.1f;
        if (m->power_index > 0.85f) m->power_index = 0.85f;
      }
    }
  }
}

static void civ_ministry_approve_reform(civ_reform_proposal_t *r) {
  if (!r) return;
  r->status = CIV_REFORM_ACTIVE;
  r->implementation_progress = 0.0f;
}

static void civ_ministry_reject_reform(civ_reform_proposal_t *r) {
  if (!r) return;
  r->status = CIV_REFORM_REJECTED;
}

/* ── Domain outputs — connect to game systems ────────────────────── */
float civ_ministry_defence_output(const civ_ministry_t *m) {
  if (!m || m->type != CIV_MINISTRY_DEFENCE) return 0.0f;
  return m->domain_output * (0.3f + m->minister.competence * 0.3f
                              + m->efficiency * 0.4f);
}

float civ_ministry_trade_output(const civ_ministry_t *m) {
  if (!m || m->type != CIV_MINISTRY_TRADE) return 0.0f;
  return m->domain_output * (0.2f + m->efficiency * 0.5f
                              + m->minister.competence * 0.3f);
}

float civ_ministry_science_output(const civ_ministry_t *m) {
  if (!m || m->type != CIV_MINISTRY_SCIENCE) return 0.0f;
  return m->domain_output * (0.25f + m->efficiency * 0.4f
                              + m->minister.competence * 0.35f);
}

float civ_ministry_health_output(const civ_ministry_t *m) {
  if (!m || m->type != CIV_MINISTRY_HEALTH) return 0.0f;
  return m->domain_output * (0.20f + m->efficiency * 0.4f
                              + m->minister.competence * 0.4f);
}

civ_ministry_t *civ_ministry_find(const civ_ministry_manager_t *mgr,
                                  civ_ministry_type_t type) {
  if (!mgr) return NULL;
  for (int i = 0; i < mgr->ministry_count; i++)
    if (mgr->ministries[i].type == type) return &mgr->ministries[i];
  return NULL;
}
