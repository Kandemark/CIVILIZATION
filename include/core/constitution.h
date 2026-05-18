/**
 * @file constitution.h
 * @brief Per-nation constitution — all game actions gate through national rules
 *
 * Every nation has a constitution defining what's legal, what requires
 * permission, and what's prohibited. All player actions check against
 * the relevant rule. Constitutions change through legislation, decree,
 * revolution, or referendum — never fixed.
 */
#ifndef CIV_CORE_CONSTITUTION_H
#define CIV_CORE_CONSTITUTION_H

#include "../common.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_CONST_RULE_MAX 32
#define CIV_CONST_NAME_MAX 96

/* ── Permission level ─────────────────────────────────────────── */
typedef enum {
  CIV_PERM_PROHIBITED       = 0,  /* illegal, criminal penalties */
  CIV_PERM_RESTRICTED       = 1,  /* requires license/permit/approval */
  CIV_PERM_ALLOWED          = 2,  /* legal without restriction */
  CIV_PERM_PROTECTED_RIGHT  = 3,  /* constitutionally guaranteed */
} civ_perm_level_t;

/* ── Rule categories ───────────────────────────────────────────── */
typedef enum {
  CIV_RULE_EMPLOYMENT,        /* who can work, minimum wage, unions */
  CIV_RULE_HOUSING,           /* property ownership, rent control */
  CIV_RULE_EDUCATION,         /* compulsory education, university access */
  CIV_RULE_POLITICAL_PARTY,   /* party formation, membership */
  CIV_RULE_RUN_FOR_OFFICE,    /* candidacy requirements */
  CIV_RULE_BUSINESS,          /* starting a business, licensing */
  CIV_RULE_CURRENCY_USE,      /* which currencies are legal tender */
  CIV_RULE_FREE_SPEECH,       /* public criticism, press freedom */
  CIV_RULE_ASSEMBLY,          /* protests, gatherings, unions */
  CIV_RULE_RELIGION,          /* religious practice, state religion */
  CIV_RULE_MILITARY_SERVICE,  /* conscription, voluntary service */
  CIV_RULE_TAXATION,          /* income tax, property tax rates */
  CIV_RULE_HEALTHCARE,        /* public/private healthcare */
  CIV_RULE_BANKING,           /* interest, lending, foreign accounts */
  CIV_RULE_TRAVEL,            /* domestic movement, emigration */
  CIV_RULE_VOTING,            /* who can vote, election frequency */
  CIV_RULE_COUNT
} civ_rule_category_t;

/* ── Rule ──────────────────────────────────────────────────────── */
typedef struct {
  civ_rule_category_t category;
  civ_perm_level_t     level;
  char                 description[128]; /* human-readable */
  float                tax_rate;         /* if applicable */
  float                minimum_age;      /* if applicable */
  float                cost;             /* license/permit cost in local currency */
} civ_national_constitution_rule_t;

/* ── Constitution ──────────────────────────────────────────────── */
typedef struct {
  char                    name[CIV_CONST_NAME_MAX];
  char                    nation_id[32];
  int32_t                 ratified_year;     /* global year */
  civ_national_constitution_rule_t rules[CIV_RULE_COUNT];

  /* Amendability */
  float                   amend_threshold;    /* votes needed to change (0-1) */
  bool                    can_suspend;        /* emergency powers allowed? */
} civ_national_constitution_t;

/* ── API ───────────────────────────────────────────────────────── */
civ_national_constitution_t *civ_national_constitution_create(const char *nation_id);
void                civ_national_constitution_destroy(civ_national_constitution_t *c);

/* Check if an action is permitted under this constitution */
bool civ_national_constitution_check(const civ_national_constitution_t *c, civ_rule_category_t cat,
                            float actor_age, float actor_wealth);

/* Get the permission level for a rule */
civ_perm_level_t civ_national_constitution_get(const civ_national_constitution_t *c,
                                      civ_rule_category_t cat);

/* Change a rule (called when legislation passes) */
void civ_national_constitution_amend(civ_national_constitution_t *c, civ_rule_category_t cat,
                            civ_perm_level_t new_level, const char *reason);

/* Default constitution for a nation (moderate liberal democracy defaults) */
void civ_national_constitution_init(civ_national_constitution_t *c, const char *nation_id);

const char *civ_rule_category_name(civ_rule_category_t cat);
const char *civ_perm_level_name(civ_perm_level_t level);

#ifdef __cplusplus
}
#endif
#endif
