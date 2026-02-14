/**
 * @file government.h
 * @brief Government and governance system
 */

#ifndef CIVILIZATION_GOVERNMENT_H
#define CIVILIZATION_GOVERNMENT_H

#include "../../common.h"
#include "../../types.h"
#include "custom_governance.h"
#include "institution.h"
#include "legislative_system.h"
#include "subdivision.h"

/* Constitutional Power Definition */
typedef enum {
  CIV_PWR_LEGISLATIVE = 0, /* Creation of laws */
  CIV_PWR_EXECUTIVE,       /* Implementation/Enforcement */
  CIV_PWR_JUDICIAL,        /* Interpretation/Review */
  CIV_PWR_OVERSIGHT,       /* Monitoring development */
  CIV_PWR_SOVEREIGNTY      /* Ultimate authority/Veto */
} civ_power_type_t;

/* Government Type */
typedef enum {
  CIV_GOV_CHIEFDOM = 0,
  CIV_GOV_DESPOTISM,
  CIV_GOV_MONARCHY,
  CIV_GOV_REPUBLIC,
  CIV_GOV_DEMOCRACY
} civ_government_type_t;

/* Phase 11: Stature Tiers */
typedef enum {
  CIV_STATURE_FAILED_STATE = 0,
  CIV_STATURE_FRONTIER_NATION,
  CIV_STATURE_DEVELOPING_STATE,
  CIV_STATURE_STABLE_STATE,
  CIV_STATURE_REGIONAL_POWER,
  CIV_STATURE_GREAT_POWER,
  CIV_STATURE_HEGEMON
} civ_stature_tier_t;

/* Governance Function (Who holds the power?) */
typedef struct {
  civ_power_type_t type;
  char holder_role[STRING_SHORT_LEN]; /* e.g. "Supreme Leader", "Parliament",
                                         "Public" */
  civ_voting_method_t voting_method;
  civ_float_t autonomy; /* 0.0 to 1.0 */
} civ_governance_function_t;

/* Government structure */
typedef struct civ_government {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  civ_government_type_t government_type;

  civ_governance_function_t *functions;
  size_t function_count;
  size_t function_capacity;

  civ_float_t stability;
  civ_float_t legitimacy;
  civ_float_t efficiency;
  char **subunit_ids;
  size_t subunit_count;
  size_t subunit_capacity;
  civ_float_t *decision_priorities;
  size_t priority_count;

  /* Phase 11: Modular Extensions */
  civ_institution_manager_t *institution_manager;
  civ_subdivision_manager_t *subdivision_manager;
  civ_legislative_manager_t *legislative_manager;

  civ_constitution_t *constitution; /* Can be NULL for "Unbound Authority" */
  civ_float_t
      legislative_threshold; /* e.g. 0.5 for simple majority, 0.66 for super */

  civ_stature_tier_t stature_tier;
} civ_government_t;

/* Function declarations */
civ_government_t *civ_government_create(const char *name);
void civ_government_destroy(civ_government_t *gov);
void civ_government_init(civ_government_t *gov, const char *name);

civ_result_t civ_government_assign_power(civ_government_t *gov,
                                         civ_power_type_t type,
                                         const char *role);

civ_float_t civ_government_collect_taxes(civ_government_t *gov);
void civ_government_update(civ_government_t *gov, civ_float_t time_delta);
void civ_government_add_subunit(civ_government_t *gov, const char *subunit_id);
civ_float_t civ_government_get_stability(const civ_government_t *gov);

#endif /* CIVILIZATION_GOVERNMENT_H */
