/**
 * @file judiciary.c
 * @brief Court system — cases, judicial review, rule of law
 */
#include "core/governance/branches/judiciary.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_judiciary_t *civ_judiciary_create(void) {
  civ_judiciary_t *j = CIV_MALLOC(sizeof(civ_judiciary_t));
  if (!j) return NULL;
  memset(j, 0, sizeof(*j));
  j->court_capacity = 4;
  j->courts = CIV_MALLOC(sizeof(civ_court_t) * j->court_capacity);
  j->case_capacity = 16;
  j->cases = CIV_MALLOC(sizeof(civ_case_t) * j->case_capacity);
  j->rule_of_law = 0.55;
  j->judicial_independence = 0.45;
  j->due_process_index = 0.50;
  return j;
}

void civ_judiciary_destroy(civ_judiciary_t *j) {
  if (!j) return;
  free(j->courts);
  free(j->cases);
  free(j);
}

void civ_judiciary_update(civ_judiciary_t *j, float dt,
                          float gov_efficiency, float corruption,
                          float centralization, float representation) {
  if (!j) return;

  /* Judicial independence: representation supports it, centralization undermines it */
  float target_independence = 0.30f + representation * 0.40f - centralization * 0.20f;
  j->judicial_independence += (target_independence - j->judicial_independence) * 0.03f * dt;

  /* Corruption erodes judicial integrity */
  j->judicial_independence -= corruption * 0.02f * dt;
  if (j->judicial_independence < 0.05f) j->judicial_independence = 0.05f;
  if (j->judicial_independence > 0.95f) j->judicial_independence = 0.95f;

  /* Per-court update */
  for (int i = 0; i < j->court_count; i++) {
    civ_court_t *c = &j->courts[i];
    c->independence = j->judicial_independence * (0.7f + 0.3f * (float)(c->level == CIV_COURT_SUPREME));
    c->efficiency = gov_efficiency * (1.0f - corruption * 0.5f) - c->backlog * 0.001f;
    if (c->efficiency < 0.05f) c->efficiency = 0.05f;
    c->public_trust += (j->judicial_independence * 0.3f + (1.0f - corruption) * 0.3f
                        - c->backlog * 0.0001f - c->public_trust) * 0.05f * dt;
  }

  /* Process cases */
  civ_judiciary_process_docket(j, dt);

  /* Rule of law: composite of independence + efficiency + due process */
  j->rule_of_law = j->judicial_independence * 0.40f + j->due_process_index * 0.35f
                   + (1.0f - corruption) * 0.25f;
  if (j->rule_of_law < 0.05f) j->rule_of_law = 0.05f;
  if (j->rule_of_law > 1.0f) j->rule_of_law = 1.0f;

  /* Due process: representation + independence - backlog */
  j->due_process_index += (representation * 0.3f + j->judicial_independence * 0.2f
                           - j->due_process_index) * 0.04f * dt;
}

civ_court_t *civ_judiciary_create_court(civ_judiciary_t *j, const char *name,
                                        civ_court_level_t level, int max_judges) {
  if (!j || !name) return NULL;
  if (j->court_count >= j->court_capacity) {
    int nc = j->court_capacity * 2;
    civ_court_t *tmp = CIV_REALLOC(j->courts, sizeof(civ_court_t) * nc);
    if (!tmp) return NULL;
    j->courts = tmp;
    j->court_capacity = nc;
  }
  civ_court_t *c = &j->courts[j->court_count];
  memset(c, 0, sizeof(*c));
  snprintf(c->id, STRING_SHORT_LEN, "COURT_%d", j->court_count);
  strncpy(c->name, name, STRING_MEDIUM_LEN - 1);
  c->level = level;
  c->max_judges = max_judges;
  c->independence = 0.45f;
  c->efficiency = 0.50f;
  c->public_trust = 0.55f;
  j->court_count++;
  return c;
}

civ_case_t *civ_judiciary_file_case(civ_judiciary_t *j, civ_case_type_t type,
                                    const char *title, float complexity) {
  if (!j || !title) return NULL;
  if (j->case_count >= j->case_capacity) {
    int nc = j->case_capacity * 2;
    civ_case_t *tmp = CIV_REALLOC(j->cases, sizeof(civ_case_t) * nc);
    if (!tmp) return NULL;
    j->cases = tmp;
    j->case_capacity = nc;
  }
  civ_case_t *cs = &j->cases[j->case_count];
  memset(cs, 0, sizeof(*cs));
  snprintf(cs->id, STRING_SHORT_LEN, "CASE_%d", j->case_count);
  strncpy(cs->title, title, STRING_MEDIUM_LEN - 1);
  cs->type = type;
  cs->status = CIV_CASE_FILED;
  cs->complexity = complexity;
  cs->government_interest = (type == CIV_CASE_CONSTITUTIONAL) ? 0.70f : 0.30f;
  j->case_count++;
  return cs;
}

void civ_judiciary_process_docket(civ_judiciary_t *j, float dt) {
  if (!j) return;
  (void)dt;

  for (int i = 0; i < j->case_count; i++) {
    civ_case_t *cs = &j->cases[i];
    cs->age_days++;

    /* Dismiss old unresolved cases */
    if (cs->age_days > 100 && cs->status == CIV_CASE_FILED) {
      cs->status = CIV_CASE_DISMISSED;
      continue;
    }

    if (cs->status != CIV_CASE_FILED) continue;

    /* Find best court for this case */
    civ_court_t *best_court = NULL;
    for (int ci = 0; ci < j->court_count; ci++) {
      civ_court_t *c = &j->courts[ci];
      if (!best_court || (c->level > best_court->level && c->backlog < c->max_judges * 2))
        best_court = c;
    }
    if (!best_court && j->court_count > 0) best_court = &j->courts[0];

    if (best_court) {
      float resolve_chance = best_court->efficiency / (cs->complexity + 0.2f) * 0.1f;
      if (cs->type == CIV_CASE_CONSTITUTIONAL)
        resolve_chance *= 0.5f; /* constitutional cases take longer */

      if ((float)rand() / RAND_MAX < resolve_chance) {
        cs->status = CIV_CASE_DECIDED;
        j->precedents_set++;
        best_court->backlog += 1.0f;

        /* Constitutional cases: may strike down laws */
        if (cs->type == CIV_CASE_CONSTITUTIONAL && civ_judiciary_can_strike_law(j)) {
          if ((float)rand() / RAND_MAX < j->judicial_independence * 0.5f) {
            cs->struck_down_law = true;
            j->laws_struck_down++;
            snprintf(cs->ruling_summary, STRING_MEDIUM_LEN, "Struck down: %s",
                     cs->affected_rule_id[0] ? cs->affected_rule_id : "executive action");
          } else {
            snprintf(cs->ruling_summary, STRING_MEDIUM_LEN, "Upheld: constitutional");
          }
        } else {
          snprintf(cs->ruling_summary, STRING_MEDIUM_LEN, "Resolved: %s", cs->title);
        }
      }
    }
  }
}

bool civ_judiciary_can_strike_law(const civ_judiciary_t *j) {
  return j ? (j->judicial_independence > 0.25f && j->rule_of_law > 0.30f) : false;
}

float civ_judiciary_rights_protection(const civ_judiciary_t *j) {
  return j ? (j->rule_of_law * 0.6f + j->judicial_independence * 0.4f) : 0.30f;
}

float civ_judiciary_executive_constraint(const civ_judiciary_t *j) {
  /* How much the judiciary constrains executive action */
  return j ? (j->judicial_independence * 0.5f + j->laws_struck_down * 0.01f) : 0.0f;
}
