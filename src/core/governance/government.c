/**
 * @file government.c
 * @brief Dynamic governance — political positions, computed descriptors
 */
#include "../../../include/core/governance/government.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_government_t *civ_government_create(const char *name) {
  civ_government_t *gov =
      (civ_government_t *)CIV_MALLOC(sizeof(civ_government_t));
  if (!gov) return NULL;

  memset(gov, 0, sizeof(civ_government_t));
  strncpy(gov->name, name, STRING_MEDIUM_LEN - 1);
  snprintf(gov->id, sizeof(gov->id), "gov_%s", name);

  gov->stability = 0.6f;
  gov->legitimacy = 0.6f;
  gov->efficiency = 0.4f;
  gov->legislative_threshold = 0.5f;

  gov->position_capacity = 8;
  gov->positions = (civ_political_position_t *)CIV_MALLOC(
      sizeof(civ_political_position_t) * gov->position_capacity);

  gov->institution_manager = civ_institution_manager_create();
  gov->subdivision_manager = civ_subdivision_manager_create();
  gov->legislative_manager = civ_legislative_manager_create();

  /* Default minimal structure: one executive position */
  civ_government_add_position(gov, "Leader", 0, 0.50f, 0.30f, 0.20f,
                              "emergence", "indefinite", 1);

  civ_government_recompute_profile(gov);
  return gov;
}

void civ_government_destroy(civ_government_t *gov) {
  if (!gov) return;
  CIV_FREE(gov->positions);
  if (gov->institution_manager) civ_institution_manager_destroy(gov->institution_manager);
  if (gov->subdivision_manager) civ_subdivision_manager_destroy(gov->subdivision_manager);
  if (gov->legislative_manager) civ_legislative_manager_destroy(gov->legislative_manager);
  CIV_FREE(gov);
}

civ_political_position_t *civ_government_add_position(
    civ_government_t *gov, const char *title, int level,
    float exec_w, float leg_w, float jud_w,
    const char *selection, const char *term, int count) {
  if (!gov || !title) return NULL;

  if (gov->position_count >= gov->position_capacity) {
    size_t new_cap = gov->position_capacity * 2;
    civ_political_position_t *new_p = (civ_political_position_t *)realloc(
        gov->positions, sizeof(civ_political_position_t) * new_cap);
    if (!new_p) return NULL;
    gov->positions = new_p;
    gov->position_capacity = new_cap;
  }

  civ_political_position_t *p = &gov->positions[gov->position_count++];
  memset(p, 0, sizeof(*p));
  strncpy(p->title, title, CIV_POSITION_TITLE_MAX - 1);
  p->hierarchy_level = level;
  p->executive_weight = exec_w;
  p->legislative_weight = leg_w;
  p->judicial_weight = jud_w;
  if (selection) strncpy(p->selection_method, selection, CIV_POSITION_SELECT_MAX - 1);
  if (term) strncpy(p->term, term, CIV_POSITION_TERM_MAX - 1);
  p->position_count = count;
  return p;
}

void civ_government_recompute_profile(civ_government_t *gov) {
  if (!gov || gov->position_count == 0) return;

  /* Aggregate weights across all positions */
  float total_exec = 0, total_leg = 0, total_jud = 0;
  float top_exec = 0;

  for (size_t i = 0; i < gov->position_count; i++) {
    civ_political_position_t *p = &gov->positions[i];
    float multiplier = (float)p->position_count;
    total_exec += p->executive_weight * multiplier;
    total_leg  += p->legislative_weight * multiplier;
    total_jud  += p->judicial_weight * multiplier;
    /* Top-level concentration */
    if (p->hierarchy_level == 0)
      top_exec += p->executive_weight * multiplier;
  }

  float total = total_exec + total_leg + total_jud;
  if (total < 0.001f) total = 1.0f;

  /* Authority concentration: how much power sits at hierarchy_level 0 */
  gov->profile.authority_concentration =
      (total > 0) ? top_exec / (total_exec + 0.001f) : 0.5f;

  /* Representation: legislative-to-executive ratio */
  gov->profile.representation_index =
      total_leg / (total_exec + total_leg + 0.001f);

  /* Power balance: how evenly the three branches split */
  float avg = total / 3.0f;
  float dev = fabsf(total_exec - avg) + fabsf(total_leg - avg) +
              fabsf(total_jud - avg);
  gov->profile.power_balance = 1.0f - (dev / (total * 2.0f + 0.001f));

  /* Institutional rigidity: based on term lengths and selection methods */
  float rigidity = 0.3f;
  int life_terms = 0, hereditary = 0;
  for (size_t i = 0; i < gov->position_count; i++) {
    if (strstr(gov->positions[i].term, "life")) life_terms++;
    if (strstr(gov->positions[i].selection_method, "hereditary")) hereditary++;
  }
  rigidity += (float)life_terms / (float)(gov->position_count + 1) * 0.35f;
  rigidity += (float)hereditary / (float)(gov->position_count + 1) * 0.35f;
  if (rigidity > 1.0f) rigidity = 1.0f;
  gov->profile.institutional_rigidity = rigidity;

  /* Citizen happiness: derived from stability + representation */
  gov->profile.citizen_happiness =
      gov->stability * 0.5f + gov->profile.representation_index * 0.3f +
      gov->legitimacy * 0.2f;

  /* Governance ranking: aggregate from all metrics */
  gov->profile.governance_ranking =
      (gov->stability * 100.0f + gov->legitimacy * 100.0f +
       gov->efficiency * 100.0f + gov->profile.citizen_happiness * 100.0f +
       gov->profile.power_balance * 50.0f) / 4.5f;
}

const char *civ_government_proximity_label(const civ_government_t *gov) {
  if (!gov) return "Unclassified";

  float ac = gov->profile.authority_concentration;
  float ri = gov->profile.representation_index;
  float ir = gov->profile.institutional_rigidity;

  if (ac > 0.75f && ri < 0.25f) {
    if (ir > 0.7f) return "Hereditary Centralized Authority";
    return "Centralized Executive Authority";
  }
  if (ac > 0.60f && ri < 0.35f) return "Concentrated Governance";
  if (ri > 0.55f && gov->profile.power_balance > 0.5f)
    return "Broad Representative System";
  if (ri > 0.40f) return "Mixed Representative Structure";
  if (ir > 0.70f) return "Traditional Institutional Order";
  if (gov->profile.power_balance > 0.6f) return "Balanced Tripartite System";
  return "Emergent Governance Structure";
}

float civ_government_collect_taxes(civ_government_t *gov) {
  if (!gov) return 0.0f;
  return gov->efficiency * 50.0f * (0.5f + gov->legitimacy * 0.5f);
}

void civ_government_update(civ_government_t *gov, float dt) {
  if (!gov) return;
  civ_government_recompute_profile(gov);
  /* Stability drifts toward equilibrium with legitimacy */
  float drift = (gov->legitimacy - gov->stability) * 0.02f * dt;
  gov->stability += drift;
  if (gov->stability < 0.0f) gov->stability = 0.0f;
  if (gov->stability > 1.0f) gov->stability = 1.0f;
}

float civ_government_get_stability(const civ_government_t *gov) {
  return gov ? gov->stability : 0.0f;
}
