/**
 * @file civil_service.c
 * @brief Civil service — career bureaucracy, merit vs patronage tradeoff
 */
#include "core/governance/institutions/civil_service.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_civil_service_t *civ_civil_service_create(void) {
  civ_civil_service_t *cs = CIV_MALLOC(sizeof(civ_civil_service_t));
  if (!cs) return NULL;
  memset(cs, 0, sizeof(*cs));
  cs->dept_capacity = 8;
  cs->departments = CIV_MALLOC(sizeof(civ_cs_department_t) * cs->dept_capacity);
  cs->recruitment_model = CIV_CS_RECRUITMENT_HYBRID;
  cs->total_employees = 1000.0f;
  cs->avg_competence = 0.45f;
  cs->institutional_memory = 0.50f;
  cs->bureaucratic_rigidity = 0.35f;
  cs->corruption_vulnerability = 0.20f;
  cs->political_neutrality = 0.50f;
  cs->budget_efficiency = 0.55f;
  return cs;
}

void civ_civil_service_destroy(civ_civil_service_t *cs) {
  if (!cs) return;
  free(cs->departments);
  free(cs);
}

void civ_civil_service_update(civ_civil_service_t *cs, float dt,
                              float meritocracy_trait, float corruption,
                              float education_level, float total_budget,
                              float government_efficiency) {
  if (!cs) return;

  /* Recruitment model effects */
  switch (cs->recruitment_model) {
    case CIV_CS_RECRUITMENT_EXAM:
      /* Merit-based: high competence, high neutrality, high rigidity */
      cs->avg_competence += (meritocracy_trait * 0.6f + education_level * 0.3f
                             - cs->avg_competence) * 0.04f * dt;
      cs->political_neutrality += (0.80f - cs->political_neutrality) * 0.03f * dt;
      cs->bureaucratic_rigidity += (0.65f - cs->bureaucratic_rigidity) * 0.02f * dt;
      cs->corruption_vulnerability = 0.10f;
      break;
    case CIV_CS_RECRUITMENT_APPOINTMENT:
      /* Politically responsive, moderate competence */
      cs->avg_competence += (0.50f - cs->avg_competence) * 0.04f * dt;
      cs->political_neutrality += (0.30f - cs->political_neutrality) * 0.04f * dt;
      cs->bureaucratic_rigidity += (0.25f - cs->bureaucratic_rigidity) * 0.03f * dt;
      cs->corruption_vulnerability = 0.35f;
      break;
    case CIV_CS_RECRUITMENT_PATRONAGE:
      /* Loyal to patrons, low competence, high corruption */
      cs->avg_competence += (0.30f - cs->avg_competence) * 0.04f * dt;
      cs->political_neutrality += (0.15f - cs->political_neutrality) * 0.05f * dt;
      cs->bureaucratic_rigidity += (0.15f - cs->bureaucratic_rigidity) * 0.04f * dt;
      cs->corruption_vulnerability = 0.55f;
      break;
    case CIV_CS_RECRUITMENT_HYBRID:
    default:
      cs->avg_competence += (meritocracy_trait * 0.4f + 0.30f
                             - cs->avg_competence) * 0.03f * dt;
      cs->political_neutrality += (0.50f - cs->political_neutrality) * 0.03f * dt;
      cs->bureaucratic_rigidity += (0.40f - cs->bureaucratic_rigidity) * 0.02f * dt;
      cs->corruption_vulnerability = 0.25f;
      break;
  }

  /* Corruption vulnerability feeds actual corruption */
  float actual_corruption = corruption * cs->corruption_vulnerability * 2.0f;
  cs->avg_competence -= actual_corruption * 0.01f * dt;

  /* Institutional memory: grows with time, eroded by patronage turnover */
  float memory_target = (cs->recruitment_model == CIV_CS_RECRUITMENT_EXAM) ? 0.80f
                      : (cs->recruitment_model == CIV_CS_RECRUITMENT_PATRONAGE) ? 0.25f : 0.55f;
  cs->institutional_memory += (memory_target - cs->institutional_memory) * 0.02f * dt;

  /* Budget efficiency: competence * neutrality */
  cs->budget_efficiency = cs->avg_competence * 0.5f + cs->political_neutrality * 0.3f
                          + (1.0f - actual_corruption) * 0.2f;

  /* Update departments */
  float per_dept_budget = (cs->dept_count > 0) ? total_budget / cs->dept_count : total_budget;
  for (int i = 0; i < cs->dept_count; i++) {
    civ_cs_department_t *d = &cs->departments[i];
    d->budget = per_dept_budget;
    d->avg_competence += (cs->avg_competence - d->avg_competence) * 0.05f * dt;
    d->morale += (government_efficiency * 0.3f + (1.0f - actual_corruption) * 0.3f
                  - d->morale * 0.1f) * 0.05f * dt;
    if (d->morale < 0.10f) d->morale = 0.10f;
    if (d->morale > 1.0f) d->morale = 1.0f;
    d->output = d->headcount * d->avg_competence * d->morale * cs->budget_efficiency;
  }

  /* Total employees from department sum */
  cs->total_employees = 0.0f;
  for (int i = 0; i < cs->dept_count; i++)
    cs->total_employees += cs->departments[i].headcount;
}

civ_cs_department_t *civ_civil_service_add_department(civ_civil_service_t *cs,
                                                       const char *name, int headcount) {
  if (!cs || !name || cs->dept_count >= cs->dept_capacity) return NULL;
  civ_cs_department_t *d = &cs->departments[cs->dept_count];
  memset(d, 0, sizeof(*d));
  snprintf(d->id, STRING_SHORT_LEN, "CSDEPT_%d", cs->dept_count);
  strncpy(d->department, name, STRING_SHORT_LEN - 1);
  d->headcount = headcount;
  d->avg_competence = cs->avg_competence;
  d->morale = 0.60f;
  cs->dept_count++;
  return d;
}

void civ_civil_service_set_recruitment(civ_civil_service_t *cs,
                                       civ_cs_recruitment_t model) {
  if (!cs) return;
  cs->recruitment_model = model;
}

float civ_civil_service_governance_buffer(const civ_civil_service_t *cs) {
  /* How well governance survives political transitions */
  return cs ? (cs->institutional_memory * 0.6f + cs->political_neutrality * 0.4f) : 0.0f;
}

float civ_civil_service_reform_resistance(const civ_civil_service_t *cs) {
  return cs ? cs->bureaucratic_rigidity : 0.0f;
}

float civ_civil_service_competence_bonus(const civ_civil_service_t *cs) {
  return cs ? (cs->avg_competence * cs->budget_efficiency) : 0.0f;
}
