/**
 * @file government.c
 * @brief Governance HUB — owns and ticks ALL governance subsystems
 */
#include "core/governance/government.h"
#include "core/economy/budget.h"
#include "common.h"
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

  /* Core metrics */
  gov->stability = 0.6f;
  gov->legitimacy = 0.6f;
  gov->efficiency = 0.4f;
  gov->legislative_threshold = 0.5f;

  /* Political positions */
  gov->position_capacity = 8;
  gov->positions = (civ_political_position_t *)CIV_MALLOC(
      sizeof(civ_political_position_t) * gov->position_capacity);

  /* ── Create ALL governance subsystems ── */
  gov->constitution        = civ_constitution_create(name);

  /* Branches: all optional. Default = tripartite (exec + leg + jud).
   * Council and religious_body are NULL by default — create them
   * only for governance forms that use collective or theocratic rule. */
  gov->executive           = civ_executive_create();
  gov->legislative_manager = civ_legislative_manager_create();
  gov->judiciary           = civ_judiciary_create();
  gov->council             = NULL;  /* no collective council by default */
  gov->religious_body      = NULL;  /* no religious authority by default */

  gov->institution_manager = civ_institution_manager_create();
  gov->subdivision_manager = civ_subdivision_manager_create();
  gov->corruption_engine   = civ_corruption_engine_create();
  gov->election_system     = civ_election_create();
  gov->political_violence  = civ_political_violence_create();
  gov->ministry_manager    = civ_ministry_manager_create();
  gov->civil_service       = civ_civil_service_create();
  gov->rights              = civ_rights_create();
  gov->notebook            = civ_notebook_create();

  /* Initialize evolution state (decision-driven governance change) */
  civ_governance_init(&gov->evolution_state);

  /* Create default courts */
  civ_judiciary_create_court(gov->judiciary, "Supreme Court", CIV_COURT_SUPREME, 9);
  civ_judiciary_create_court(gov->judiciary, "Court of Appeals", CIV_COURT_APPEALS, 15);
  civ_judiciary_create_court(gov->judiciary, "District Courts", CIV_COURT_LOCAL, 50);

  /* Create default ministries for all 11 ministry types */
  const char *ministry_names[] = {
    "Defence", "Finance", "Foreign Affairs", "Justice",
    "Interior", "Culture", "Science", "Trade", "Labour", "Health"
  };
  for (int i = 0; i < CIV_MINISTRY_TYPE_COUNT; i++) {
    civ_ministry_t *m = civ_ministry_create((civ_ministry_type_t)i,
                         i < 10 ? ministry_names[i] : "General Affairs");
    if (m) {
      if (gov->ministry_manager->ministry_count < gov->ministry_manager->ministry_capacity) {
        gov->ministry_manager->ministries[gov->ministry_manager->ministry_count++] = *m;
        free(m);
      } else {
        free(m);
      }
    }
  }

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
  if (gov->constitution)        civ_constitution_destroy(gov->constitution);
  if (gov->executive)           civ_executive_destroy(gov->executive);
  if (gov->legislative_manager) civ_legislative_manager_destroy(gov->legislative_manager);
  if (gov->judiciary)           civ_judiciary_destroy(gov->judiciary);
  if (gov->council)             civ_council_destroy(gov->council);
  if (gov->religious_body)      civ_religious_body_destroy(gov->religious_body);
  if (gov->corruption_engine)   civ_corruption_engine_destroy(gov->corruption_engine);
  if (gov->election_system)     civ_election_destroy(gov->election_system);
  if (gov->political_violence)  civ_political_violence_destroy(gov->political_violence);
  if (gov->ministry_manager)    civ_ministry_manager_destroy(gov->ministry_manager);
  if (gov->civil_service)       civ_civil_service_destroy(gov->civil_service);
  if (gov->rights)              civ_rights_destroy(gov->rights);
  if (gov->notebook)            civ_notebook_destroy(gov->notebook);
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

  float total_exec = 0, total_leg = 0, total_jud = 0;
  float top_exec = 0;

  for (size_t i = 0; i < gov->position_count; i++) {
    civ_political_position_t *p = &gov->positions[i];
    float multiplier = (float)p->position_count;
    total_exec += p->executive_weight * multiplier;
    total_leg  += p->legislative_weight * multiplier;
    total_jud  += p->judicial_weight * multiplier;
    if (p->hierarchy_level == 0)
      top_exec += p->executive_weight * multiplier;
  }

  float total = total_exec + total_leg + total_jud;
  if (total < 0.001f) total = 1.0f;

  gov->profile.authority_concentration =
      (total > 0) ? top_exec / (total_exec + 0.001f) : 0.5f;

  gov->profile.representation_index =
      total_leg / (total_exec + total_leg + 0.001f);

  float avg = total / 3.0f;
  float dev = fabsf(total_exec - avg) + fabsf(total_leg - avg) +
              fabsf(total_jud - avg);
  gov->profile.power_balance = 1.0f - (dev / (total * 2.0f + 0.001f));

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

  gov->profile.citizen_happiness =
      gov->stability * 0.5f + gov->profile.representation_index * 0.3f +
      gov->legitimacy * 0.2f;

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

/* ====================================================================
 * MAIN TICK — propagates to ALL governance subsystems
 * ==================================================================== */
void civ_government_update(civ_government_t *gov, float dt,
                           int total_population, float culture_level,
                           float total_budget, float education_level,
                           float economic_confidence, float literacy_rate,
                           float faction_support, int faction_count) {
  if (!gov) return;

  /* Cache cross-module values for use by sub-functions */
  gov->faction_support = faction_support;
  gov->faction_count   = faction_count;

  /* Recompute profile from current political structure */
  civ_government_recompute_profile(gov);

  /* ── Tick institutions: budget-fed growth/decay ── */
  float inst_budget = gov->budget_allocations[CIV_BUDGET_ADMINISTRATION];
  if (inst_budget < 1000.0f) inst_budget = 1000.0f;
  civ_institution_update(gov->institution_manager, inst_budget, gov->efficiency, dt);

  /* ── Tick subdivisions: stability drift ── */
  civ_subdivision_update(gov->subdivision_manager, dt);

  /* ── Tick governance evolution: decision-driven trait changes ── */
  civ_governance_update(&gov->evolution_state,
                        (civ_float_t)total_population, (civ_float_t)culture_level);

  /* Aliases for evolution and corruption — used throughout the tick */
  civ_governance_state_t *ev = &gov->evolution_state;
  civ_corruption_engine_t *ce = gov->corruption_engine;

  /* Evolution state feeds back into government metrics */
  gov->stability  += (gov->evolution_state.stability - gov->stability) * 0.05f * dt;
  gov->legitimacy += (gov->evolution_state.legitimacy - gov->legitimacy) * 0.05f * dt;
  float ev_eff = (float)civ_governance_efficiency(&gov->evolution_state, total_population, culture_level);
  float ev_cr  = (float)civ_governance_corruption_resistance(&gov->evolution_state);
  gov->efficiency += (ev_eff - gov->efficiency) * 0.08f * dt;

  /* ── 4. Tick executive: veto, decrees, political capital ── */
  civ_executive_update(gov->executive, dt,
                       (float)ev->traits.centralization, gov->legitimacy,
                       ev->emergency_active ? (float)ev->emergency_power_grab : 0.0f,
                       (float)ev->traits.representation);

  /* ── 5. Tick judiciary: courts, cases, rule of law ── */
  if (gov->judiciary)
    civ_judiciary_update(gov->judiciary, dt, gov->efficiency,
                         gov->corruption_engine->systemic_index,
                         (float)ev->traits.centralization,
                         (float)ev->traits.representation);

  /* ── 6. Tick council: collective leadership (NULL = not used) ── */
  if (gov->council)
    civ_council_update(gov->council, dt,
                       (float)ev->traits.centralization, gov->legitimacy,
                       gov->corruption_engine->systemic_index,
                       (float)ev->traits.tradition_index);

  /* ── 7. Tick religious body: theocratic authority (NULL = secular) ── */
  if (gov->religious_body)
    civ_religious_body_update(gov->religious_body, dt,
                              (float)ev->traits.religious_authority,
                              (float)ev->traits.tradition_index,
                              (float)ev->traits.representation,
                              gov->corruption_engine->systemic_index);

  /* ── 8. Tick elections: autonomous electoral cycles ── */
  if (gov->election_system) {
    civ_election_update(gov->election_system, dt,
                        total_population, gov->corruption_engine->systemic_index,
                        (float)ev->traits.representation,
                        gov->profile.citizen_happiness, economic_confidence, literacy_rate);
    /* Schedule initial election if none exist and representation > 0.2 */
    if (gov->election_system->election_count == 0 && ev->traits.representation > 0.20) {
      civ_election_schedule(gov->election_system, "general_election",
                            "General Assembly", CIV_ELECTION_FPTP, 100, 40);
      /* Add some candidates */
      civ_election_t *e = &gov->election_system->elections[0];
      civ_election_add_candidate(e, "Incumbent Leader", "NPC_LEADER", "GOVERNING", true);
      civ_election_add_candidate(e, "Challenger", "NPC_CHALLENGER", "OPPOSITION", false);
      civ_election_add_candidate(e, "Independent Voice", "NPC_INDEP", "INDEPENDENT", false);
    }
  }

  /* ── 9. Tick civil service: bureaucracy, merit vs patronage ── */
  if (gov->civil_service) {
    civ_civil_service_update(gov->civil_service, dt,
                             (float)ev->traits.meritocracy,
                             gov->corruption_engine->systemic_index,
                             education_level, total_budget, gov->efficiency);
    /* Create default departments if empty */
    if (gov->civil_service->dept_count == 0) {
      civ_civil_service_add_department(gov->civil_service, "Treasury", 200);
      civ_civil_service_add_department(gov->civil_service, "Public Works", 300);
      civ_civil_service_add_department(gov->civil_service, "Records Office", 150);
      civ_civil_service_add_department(gov->civil_service, "Foreign Service", 100);
    }
  }

  /* ── 10. Tick rights: civil liberties, constitutional protections ── */
  if (gov->rights && gov->judiciary) {
    float rule_of_law = gov->judiciary->rule_of_law;
    civ_rights_update(gov->rights, dt, rule_of_law,
                      (float)ev->traits.representation,
                      (float)ev->traits.centralization,
                      ev->emergency_active ? (float)ev->emergency_power_grab : 0.0f,
                      gov->corruption_engine->systemic_index, education_level);

    /* Default: grant basic rights if representation rises */
    if (ev->traits.representation > 0.30f) {
      if (gov->rights->rights[CIV_RIGHT_SPEECH].level == CIV_PROTECTION_NONE)
        civ_rights_set_protection(gov->rights, CIV_RIGHT_SPEECH, CIV_PROTECTION_STATUTORY);
      if (gov->rights->rights[CIV_RIGHT_ASSEMBLY].level == CIV_PROTECTION_NONE)
        civ_rights_set_protection(gov->rights, CIV_RIGHT_ASSEMBLY, CIV_PROTECTION_STATUTORY);
      if (gov->rights->rights[CIV_RIGHT_PROPERTY].level == CIV_PROTECTION_NONE)
        civ_rights_set_protection(gov->rights, CIV_RIGHT_PROPERTY, CIV_PROTECTION_STATUTORY);
    }
    if (ev->traits.representation > 0.50f) {
      if (gov->rights->rights[CIV_RIGHT_DUE_PROCESS].level == CIV_PROTECTION_NONE)
        civ_rights_set_protection(gov->rights, CIV_RIGHT_DUE_PROCESS, CIV_PROTECTION_CONSTITUTIONAL);
      if (gov->rights->rights[CIV_RIGHT_VOTE].level == CIV_PROTECTION_NONE)
        civ_rights_set_protection(gov->rights, CIV_RIGHT_VOTE, CIV_PROTECTION_CONSTITUTIONAL);
    }

    /* File constitutional challenges when rights violations detected */
    for (int i = 0; i < CIV_RIGHT_COUNT; i++) {
      if (gov->rights->rights[i].violations_this_cycle > 0
          && gov->rights->constitutional_challenges > 0
          && gov->judiciary->case_count < gov->judiciary->case_capacity) {
        civ_judiciary_file_case(gov->judiciary, CIV_CASE_CONSTITUTIONAL,
                                "Rights violation challenge", 0.60f);
        gov->rights->rights[i].violations_this_cycle = 0;
      }
    }
  }

  /* ── 11. Political violence: coups, assassinations, civil war ── */
  if (gov->political_violence) {
    float cohesion = (float)civ_governance_cohesion_bonus(ev);
    civ_political_violence_update(gov->political_violence, dt,
                                  (float)ev->traits.militarization,
                                  (float)ev->traits.centralization,
                                  gov->legitimacy, gov->stability,
                                  gov->corruption_engine->systemic_index,
                                  (float)ev->traits.representation,
                                  gov->executive ? gov->executive->power_consolidation : 0.0f,
                                  gov->executive ? gov->executive->constitution_suspended : false,
                                  (float)gov->faction_count, cohesion);
  }

  /* ── 12. Tick corruption engine ── */
  /* Systemic corruption rises with low efficiency, high institutional rigidity */
  civ_float_t corruption_pressure = (1.0f - gov->efficiency) * 0.02f
                                    + gov->profile.institutional_rigidity * 0.01f;
  /* Add involvement from each actor (simplified: one aggregate actor) */
  civ_corruption_add_involvement(gov->corruption_engine, "STATE", corruption_pressure * dt);
  /* Audit effectiveness from efficiency */
  gov->corruption_engine->audit_effectiveness = gov->efficiency * 0.8f;
  /* Run light audit each tick */
  civ_corruption_run_audit(gov->corruption_engine, 0.02f * dt);

  /* Leakage from corruption reduces effective budget */
  civ_float_t corruption_leakage = civ_corruption_calculate_leakage(gov->corruption_engine,
                                                                     total_budget);
  (void)corruption_leakage; /* used implicitly via reduced budget in economy */

  /* ── Tick ministries: budget competition, reforms, minister dynamics ── */
  civ_ministry_update_all(gov->ministry_manager, total_budget,
                          gov->efficiency, gov->corruption_engine->systemic_index, dt);

  /* ── Stability drift ── */
  /* Legitimacy pulls stability; corruption erodes both */
  float drift = (gov->legitimacy - gov->stability) * 0.02f * dt;
  drift -= gov->corruption_engine->systemic_index * 0.01f * dt;
  gov->stability += drift;

  /* Legitimacy: eroded by corruption, boosted by representation */
  float legit_drift = gov->profile.representation_index * 0.01f
                      - gov->corruption_engine->systemic_index * 0.02f;
  gov->legitimacy += legit_drift * dt;

  /* Efficiency: dragged by corruption and rigidity */
  float eff_target = 0.5f + gov->profile.representation_index * 0.3f
                     - gov->profile.institutional_rigidity * 0.2f
                     - gov->corruption_engine->systemic_index * 0.4f;
  gov->efficiency += (eff_target - gov->efficiency) * 0.03f * dt;

  /* Clamp */
  if (gov->stability < 0.0f)  gov->stability = 0.0f;
  if (gov->stability > 1.0f)  gov->stability = 1.0f;
  if (gov->legitimacy < 0.0f) gov->legitimacy = 0.0f;
  if (gov->legitimacy > 1.0f) gov->legitimacy = 1.0f;
  if (gov->efficiency < 0.0f) gov->efficiency = 0.0f;
  if (gov->efficiency > 1.0f) gov->efficiency = 1.0f;

  /* ── Societal health dashboard ── */

  gov->societal_health.stability_index     = gov->stability;
  gov->societal_health.cohesion_index      = gov->legitimacy;
  gov->societal_health.corruption_index    = ce->systemic_index;
  gov->societal_health.radicalization_index = 1.0f - gov->profile.citizen_happiness;
  gov->societal_health.evolution_velocity  = (float)(ev->reform_cooldown > 0 ? 1.0/ev->reform_cooldown : 1.0);
  gov->societal_health.secularism_index    = 0.5f + ev->traits.meritocracy * 0.3f - ev->traits.religious_authority * 0.3f;
  gov->societal_health.vitality_index      = gov->profile.citizen_happiness;
  gov->societal_health.economic_cohesion   = gov->efficiency;
  gov->societal_health.international_repute = gov->profile.governance_ranking / 100.0f;
  gov->societal_health.gdp_index           = 1.0f; /* set externally from economy */
  gov->societal_health.industrial_stability = 0.8f;
  civ_governance_describe(ev, gov->societal_health.dominant_title, STRING_SHORT_LEN);

  /* ── Periodic legislative session (every ~20 turns) ── */
  gov->turns_since_last_session++;
  if (gov->turns_since_last_session >= 20) {
    civ_government_hold_session(gov);
    gov->turns_since_last_session = 0;
  }

  /* ── Decision trigger from evolution system ── */
  if (civ_governance_should_decide(ev, (civ_float_t)total_population, (civ_float_t)culture_level)) {
    civ_governance_decision_t decision =
      civ_governance_generate_decision(ev, (civ_float_t)total_population, (civ_float_t)culture_level);
    /* Auto-resolve: pick option B (moderate) for now — player UI would intercept this */
    civ_governance_apply_decision(ev, &decision, 1);
  }

  /* ── Notebook: record significant events ── */
  char note[STRING_MAX_LEN], label[64];
  civ_governance_describe(ev, label, sizeof(label));
  snprintf(note, sizeof(note),
           "Turn: stab=%.2f legit=%.2f eff=%.2f corr=%.3f | %s | %s",
           gov->stability, gov->legitimacy, gov->efficiency,
           ce->systemic_index, label, civ_government_proximity_label(gov));
  civ_notebook_add_note(gov->notebook, "Governance Update", note);
}

/* ── Corruption: single source of truth ──────────────────────────── */
float civ_government_get_corruption(const civ_government_t *gov) {
  if (!gov || !gov->corruption_engine) return 0.0f;
  return gov->corruption_engine->systemic_index;
}

float civ_government_get_stability(const civ_government_t *gov) {
  return gov ? gov->stability : 0.0f;
}

/* ── Budget integration ──────────────────────────────────────────── */
void civ_government_set_budget(civ_government_t *gov, int category, float amount) {
  if (!gov || category < 0 || category >= CIV_GOV_BUDGET_CATEGORIES) return;
  gov->budget_allocations[category] = amount;
}

/* ── Legislative session ──────────────────────────────────────────── */
void civ_government_hold_session(civ_government_t *gov) {
  if (!gov || !gov->legislative_manager) return;

  /* Create a simple bill to test legislative machinery */
  civ_rule_t *rule = civ_rule_create("Public Order Act", CIV_RULE_SCOPE_NATIONAL, CIV_RULE_TYPE_LAW);
  if (!rule) return;
  strncpy(rule->description, "Maintain public order through regulated enforcement",
          STRING_MAX_LEN - 1);
  rule->authority_role[0] = '\0';
  rule->active = true;

  /* Propose through first legislative body if one exists */
  if (gov->legislative_manager->body_count > 0) {
    const char *body_id = gov->legislative_manager->bodies[0].id;
    civ_legislative_propose_bill(gov->legislative_manager, body_id, rule, false);

    /* Dynamic voting from faction influence — no hardcoded split. */
    int total_members = gov->legislative_manager->bodies[0].member_count;
    if (total_members < 10) total_members = 100;
    float gov_support = (gov->faction_count > 0) ? gov->faction_support : 0.55f;
    /* Opposition fragments with more factions */
    float fragmentation = (gov->faction_count > 2) ? (gov->faction_count - 2) * 0.05f : 0.0f;
    float yes_ratio = gov_support * (1.0f - fragmentation);
    float no_ratio  = (1.0f - gov_support) * (1.0f - fragmentation * 0.5f);
    if (yes_ratio < 0.15f) yes_ratio = 0.15f;
    if (no_ratio < 0.10f)  no_ratio = 0.10f;
    int yes = (int)(total_members * yes_ratio);
    int no  = (int)(total_members * no_ratio);
    for (int i = 0; i < yes; i++)
      civ_legislative_cast_vote(gov->legislative_manager, rule->id, true);
    for (int i = 0; i < no; i++)
      civ_legislative_cast_vote(gov->legislative_manager, rule->id, false);
    /* remainder = abstain */

    /* Resolve: apply to constitution in custom_governance */
    /* Note: constitution lives in custom_governance_t — accessed via game struct */
    civ_legislative_resolve_bill(gov->legislative_manager, rule->id, gov->constitution);
  } else {
    /* No legislative body exists — create a default one */
    civ_legislative_body_t *body = civ_legislative_body_create("General Assembly", "Leader");
    if (body) {
      body->member_count = 100;
      civ_legislative_manager_add_body(gov->legislative_manager, body);
      free(body);
    }
    civ_rule_destroy(rule);
  }
}
