/**
 * @file rights.h
 * @brief Rights declaration — civil liberties, constitutional protections
 *
 * Tracks which rights are constitutionally protected and how well they
 * are enforced. Rights exist on a spectrum — no configuration is "bad."
 * Strong speech protections enable dissent but can destabilize.
 * Strong property rights enable investment but entrench inequality.
 * Strong due process protects the accused but slows justice.
 *
 * Every right is a tradeoff. Enforcement depends on judiciary rule_of_law.
 */
#ifndef CIV_GOVERNANCE_RIGHTS_H
#define CIV_GOVERNANCE_RIGHTS_H

#include "../../../common.h"
#include "../../../types.h"

typedef enum {
  CIV_RIGHT_SPEECH,          /* free speech, press freedom */
  CIV_RIGHT_ASSEMBLY,        /* right to gather, protest */
  CIV_RIGHT_RELIGION,        /* religious freedom */
  CIV_RIGHT_DUE_PROCESS,     /* fair trial, habeas corpus */
  CIV_RIGHT_PROPERTY,        /* property rights, contracts */
  CIV_RIGHT_ARMS,            /* right to bear arms / militia */
  CIV_RIGHT_PRIVACY,         /* protection from search/seizure */
  CIV_RIGHT_VOTE,            /* suffrage, voting rights */
  CIV_RIGHT_EQUALITY,        /* equal protection under law */
  CIV_RIGHT_MOVEMENT,        /* freedom of movement, migration */
  CIV_RIGHT_COUNT
} civ_right_type_t;

typedef enum {
  CIV_PROTECTION_NONE,        /* not protected — state can restrict freely */
  CIV_PROTECTION_STATUTORY,   /* protected by ordinary law (can be changed) */
  CIV_PROTECTION_CONSTITUTIONAL, /* constitutionally entrenched */
  CIV_PROTECTION_ABSOLUTE,    /* cannot be restricted under any circumstance */
} civ_protection_level_t;

typedef struct {
  civ_right_type_t     type;
  civ_protection_level_t level;
  float enforcement;          /* 0.0–1.0, depends on judiciary rule_of_law */
  float restriction_pressure; /* 0.0–1.0, government desire to restrict */
  int   violations_this_cycle;
  bool  under_emergency_suspension;
} civ_right_t;

typedef struct {
  civ_right_t rights[CIV_RIGHT_COUNT];
  float civil_liberties_index;    /* composite 0.0–1.0 */
  float rights_consciousness;     /* how much the population values rights */
  int   constitutional_challenges; /* rights-based court cases filed */
} civ_rights_declaration_t;

civ_rights_declaration_t *civ_rights_create(void);
void civ_rights_destroy(civ_rights_declaration_t *r);
void civ_rights_update(civ_rights_declaration_t *r, float dt,
                       float rule_of_law, float representation,
                       float centralization, float emergency_active,
                       float corruption, float education_level);

void civ_rights_set_protection(civ_rights_declaration_t *r,
                               civ_right_type_t type,
                               civ_protection_level_t level);
void civ_rights_violate(civ_rights_declaration_t *r, civ_right_type_t type);
float civ_rights_happiness_modifier(const civ_rights_declaration_t *r);
float civ_rights_research_modifier(const civ_rights_declaration_t *r);
float civ_rights_stability_modifier(const civ_rights_declaration_t *r);
float civ_rights_economic_modifier(const civ_rights_declaration_t *r);

#endif
