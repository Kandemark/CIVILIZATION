/**
 * @file custom_governance.h
 * @brief Custom, emergent governance system
 */

#ifndef CIVILIZATION_CUSTOM_GOVERNANCE_H
#define CIVILIZATION_CUSTOM_GOVERNANCE_H

#include "../../common.h"
#include "../../types.h"
#include "rule_system.h"

/* Governance role */
typedef struct {
  char role_name[STRING_MEDIUM_LEN];
  char description[STRING_MAX_LEN];
  civ_float_t authority;      /* 0.0 to 1.0 */
  civ_float_t responsibility; /* 0.0 to 1.0 */
  int32_t holder_count;       /* How many people hold this role */
} civ_governance_role_t;

/* Governance structure */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  char description[STRING_MAX_LEN];

  civ_governance_role_t *roles;
  size_t role_count;
  size_t role_capacity;

  civ_float_t centralization;  /* 0.0 = decentralized, 1.0 = centralized */
  civ_float_t democracy_level; /* 0.0 = autocracy, 1.0 = full democracy */
  civ_float_t stability;
  civ_float_t efficiency;

  /* Custom political mechanics */
  char *custom_rules;
  size_t custom_rules_size;

  time_t creation_time;
  time_t last_reform;

  civ_constitution_t *constitution; /* The dynamic constitution */
} civ_custom_governance_t;

/* Custom governance manager */
typedef struct {
  civ_custom_governance_t *governments;
  size_t government_count;
  size_t government_capacity;
} civ_custom_governance_manager_t;

/* Function declarations */
civ_custom_governance_manager_t *civ_custom_governance_manager_create(void);
void civ_custom_governance_manager_destroy(
    civ_custom_governance_manager_t *manager);
void civ_custom_governance_manager_init(
    civ_custom_governance_manager_t *manager);

civ_custom_governance_t *civ_custom_governance_create(const char *id,
                                                      const char *name);
void civ_custom_governance_destroy(civ_custom_governance_t *gov);
civ_result_t civ_custom_governance_add_role(civ_custom_governance_t *gov,
                                            const char *role_name,
                                            const char *description,
                                            civ_float_t authority);
civ_result_t civ_custom_governance_reform(civ_custom_governance_t *gov,
                                          const char *reform_description);
civ_result_t civ_custom_governance_update(civ_custom_governance_t *gov,
                                          civ_float_t time_delta);
civ_result_t
civ_custom_governance_manager_add(civ_custom_governance_manager_t *manager,
                                  civ_custom_governance_t *gov);
civ_custom_governance_t *civ_custom_governance_manager_find(
    const civ_custom_governance_manager_t *manager, const char *id);

#endif /* CIVILIZATION_CUSTOM_GOVERNANCE_H */
