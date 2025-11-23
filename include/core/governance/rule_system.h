/**
 * @file rule_system.h
 * @brief Dynamic Rule and Constitution System
 *
 * Defines the structure for rules, laws, and constitutions that can be
 * applied at local, national, and international levels.
 */

#ifndef CIVILIZATION_RULE_SYSTEM_H
#define CIVILIZATION_RULE_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Rule Scope */
typedef enum {
  CIV_RULE_SCOPE_LOCAL = 0,    /* City/Town level */
  CIV_RULE_SCOPE_REGIONAL,     /* State/Province level */
  CIV_RULE_SCOPE_NATIONAL,     /* Nation level */
  CIV_RULE_SCOPE_INTERNATIONAL /* Alliance/Global level */
} civ_rule_scope_t;

/* Rule Type */
typedef enum {
  CIV_RULE_TYPE_LAW = 0,   /* Enforceable law */
  CIV_RULE_TYPE_RIGHT,     /* Guaranteed right */
  CIV_RULE_TYPE_PROCEDURE, /* Administrative procedure */
  CIV_RULE_TYPE_CUSTOM     /* Cultural custom */
} civ_rule_type_t;

/* Rule */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  char description[STRING_MAX_LEN];

  civ_rule_scope_t scope;
  civ_rule_type_t type;

  char authority_role[STRING_SHORT_LEN]; /* Role required to change/enforce */
  civ_float_t required_authority;        /* 0.0 to 1.0 */

  /* Effects (simplified for C) */
  char target_attribute[STRING_SHORT_LEN]; /* e.g., "tax_rate", "speech_freedom"
                                            */
  civ_float_t modifier_value;

  bool active;
  time_t enacted_date;
} civ_rule_t;

/* Constitution / Charter */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];

  civ_rule_t *rules;
  size_t rule_count;
  size_t rule_capacity;

  /* Amendment Process */
  civ_float_t amendment_threshold;       /* e.g., 0.66 for 2/3 majority */
  char amendment_body[STRING_SHORT_LEN]; /* ID of body that votes */

  time_t last_amendment;
} civ_constitution_t;

/* Functions */
civ_constitution_t *civ_constitution_create(const char *name);
void civ_constitution_destroy(civ_constitution_t *constitution);

civ_rule_t *civ_rule_create(const char *name, civ_rule_scope_t scope,
                            civ_rule_type_t type);
void civ_rule_destroy(civ_rule_t *rule);

civ_result_t civ_constitution_add_rule(civ_constitution_t *constitution,
                                       civ_rule_t *rule);
civ_result_t civ_constitution_remove_rule(civ_constitution_t *constitution,
                                          const char *rule_id);
civ_rule_t *civ_constitution_find_rule(const civ_constitution_t *constitution,
                                       const char *rule_id);

/* Validation */
bool civ_rule_is_valid_in_scope(const civ_rule_t *rule,
                                civ_rule_scope_t context_scope);

#endif /* CIVILIZATION_RULE_SYSTEM_H */
