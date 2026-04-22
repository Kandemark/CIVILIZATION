/**
 * @file rights.c
 * @brief Rights declaration — civil liberties and constitutional protections
 */
#include "core/governance/legal/rights.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_rights_declaration_t *civ_rights_create(void) {
  civ_rights_declaration_t *r = CIV_MALLOC(sizeof(civ_rights_declaration_t));
  if (!r) return NULL;
  memset(r, 0, sizeof(*r));

  /* Default: minimal rights in early societies */
  for (int i = 0; i < CIV_RIGHT_COUNT; i++) {
    r->rights[i].type = (civ_right_type_t)i;
    r->rights[i].level = CIV_PROTECTION_NONE;
    r->rights[i].enforcement = 0.10f;
  }
  r->civil_liberties_index = 0.15f;
  r->rights_consciousness = 0.25f;
  return r;
}

void civ_rights_destroy(civ_rights_declaration_t *r) { free(r); }

void civ_rights_update(civ_rights_declaration_t *r, float dt,
                       float rule_of_law, float representation,
                       float centralization, float emergency_active,
                       float corruption, float education_level) {
  if (!r) return;

  /* Rights enforcement depends on judiciary rule_of_law */
  for (int i = 0; i < CIV_RIGHT_COUNT; i++) {
    civ_right_t *rt = &r->rights[i];

    if (rt->level == CIV_PROTECTION_NONE) {
      rt->enforcement = 0.05f;
    } else {
      float target_enforcement = rule_of_law * 0.5f
                                 + (rt->level == CIV_PROTECTION_CONSTITUTIONAL ? 0.25f : 0.0f)
                                 + (rt->level == CIV_PROTECTION_ABSOLUTE ? 0.35f : 0.0f)
                                 + (1.0f - corruption) * 0.15f;
      rt->enforcement += (target_enforcement - rt->enforcement) * 0.05f * dt;
    }

    /* Emergency: rights can be suspended */
    rt->under_emergency_suspension = (emergency_active > 0.0f
                                      && rt->type != CIV_RIGHT_DUE_PROCESS);

    /* Centralization increases restriction pressure */
    rt->restriction_pressure = centralization * 0.4f + corruption * 0.3f
                               - representation * 0.3f;
    if (rt->restriction_pressure < 0.0f) rt->restriction_pressure = 0.0f;
    if (rt->restriction_pressure > 1.0f) rt->restriction_pressure = 1.0f;

    /* Violations when enforcement is low and restriction pressure is high */
    if (rt->enforcement < 0.30f && rt->restriction_pressure > 0.50f) {
      float violation_chance = (rt->restriction_pressure - rt->enforcement) * 0.05f;
      if ((float)rand() / RAND_MAX < violation_chance)
        rt->violations_this_cycle++;
    }
  }

  /* Civil liberties index: avg enforcement across all rights */
  float sum = 0.0f;
  for (int i = 0; i < CIV_RIGHT_COUNT; i++)
    sum += r->rights[i].enforcement;
  r->civil_liberties_index = sum / CIV_RIGHT_COUNT;

  /* Rights consciousness: education + representation */
  r->rights_consciousness += (education_level * 0.4f + representation * 0.3f
                              + r->civil_liberties_index * 0.2f
                              - r->rights_consciousness) * 0.03f * dt;

  /* Constitutional challenges filed when rights are violated */
  for (int i = 0; i < CIV_RIGHT_COUNT; i++) {
    if (r->rights[i].violations_this_cycle > 0 && r->rights_consciousness > 0.30f
        && (float)rand() / RAND_MAX < 0.10f) {
      r->constitutional_challenges++;
    }
  }
}

void civ_rights_set_protection(civ_rights_declaration_t *r,
                               civ_right_type_t type,
                               civ_protection_level_t level) {
  if (!r || type >= CIV_RIGHT_COUNT) return;
  r->rights[type].level = level;
}

void civ_rights_violate(civ_rights_declaration_t *r, civ_right_type_t type) {
  if (!r || type >= CIV_RIGHT_COUNT) return;
  r->rights[type].violations_this_cycle++;
}

float civ_rights_happiness_modifier(const civ_rights_declaration_t *r) {
  if (!r) return 1.0f;
  /* Speech + assembly + religion: personal freedom drives happiness */
  float personal = (r->rights[CIV_RIGHT_SPEECH].enforcement
                    + r->rights[CIV_RIGHT_ASSEMBLY].enforcement
                    + r->rights[CIV_RIGHT_RELIGION].enforcement) / 3.0f;
  return 0.7f + personal * 0.3f;
}

float civ_rights_research_modifier(const civ_rights_declaration_t *r) {
  if (!r) return 1.0f;
  /* Speech + privacy enable free inquiry */
  float inquiry = (r->rights[CIV_RIGHT_SPEECH].enforcement
                   + r->rights[CIV_RIGHT_PRIVACY].enforcement) * 0.5f;
  return 0.6f + inquiry * 0.4f;
}

float civ_rights_stability_modifier(const civ_rights_declaration_t *r) {
  if (!r) return 1.0f;
  /* Due process + equality: perceived fairness stabilizes society */
  float fairness = (r->rights[CIV_RIGHT_DUE_PROCESS].enforcement
                    + r->rights[CIV_RIGHT_EQUALITY].enforcement) * 0.5f;
  /* But speech + assembly can destabilize */
  float dissent = (r->rights[CIV_RIGHT_SPEECH].enforcement
                   + r->rights[CIV_RIGHT_ASSEMBLY].enforcement) * 0.5f;
  return 0.6f + fairness * 0.25f - dissent * 0.15f;
}

float civ_rights_economic_modifier(const civ_rights_declaration_t *r) {
  if (!r) return 1.0f;
  /* Property + movement + contract enforcement enables markets */
  float economic = (r->rights[CIV_RIGHT_PROPERTY].enforcement
                    + r->rights[CIV_RIGHT_MOVEMENT].enforcement) * 0.5f;
  return 0.7f + economic * 0.3f;
}
