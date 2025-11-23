/**
 * @file rule_system.c
 * @brief Implementation of Dynamic Rule System
 */

#include "../../../include/core/governance/rule_system.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

civ_constitution_t *civ_constitution_create(const char *name) {
  civ_constitution_t *constitution = CIV_MALLOC(sizeof(civ_constitution_t));
  if (constitution) {
    snprintf(constitution->id, STRING_SHORT_LEN, "const_%ld", (long)time(NULL));
    strncpy(constitution->name, name, STRING_MEDIUM_LEN - 1);

    constitution->rules = NULL;
    constitution->rule_count = 0;
    constitution->rule_capacity = 0;

    constitution->amendment_threshold = 0.51f; // Simple majority default
    memset(constitution->amendment_body, 0, STRING_SHORT_LEN);

    constitution->last_amendment = 0;
  }
  return constitution;
}

void civ_constitution_destroy(civ_constitution_t *constitution) {
  if (constitution) {
    if (constitution->rules) {
      // Rules are stored by value in array, but might have deep pointers later
      // For now, flat struct
      CIV_FREE(constitution->rules);
    }
    CIV_FREE(constitution);
  }
}

civ_rule_t *civ_rule_create(const char *name, civ_rule_scope_t scope,
                            civ_rule_type_t type) {
  civ_rule_t *rule = CIV_MALLOC(sizeof(civ_rule_t));
  if (rule) {
    snprintf(rule->id, STRING_SHORT_LEN, "rule_%ld_%d", (long)time(NULL),
             rand() % 1000);
    strncpy(rule->name, name, STRING_MEDIUM_LEN - 1);
    memset(rule->description, 0, STRING_MAX_LEN);

    rule->scope = scope;
    rule->type = type;

    memset(rule->authority_role, 0, STRING_SHORT_LEN);
    rule->required_authority = 0.5f;

    memset(rule->target_attribute, 0, STRING_SHORT_LEN);
    rule->modifier_value = 0.0f;

    rule->active = true;
    rule->enacted_date = time(NULL);
  }
  return rule;
}

void civ_rule_destroy(civ_rule_t *rule) {
  if (rule) {
    CIV_FREE(rule);
  }
}

civ_result_t civ_constitution_add_rule(civ_constitution_t *constitution,
                                       civ_rule_t *rule) {
  if (!constitution || !rule)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (constitution->rule_count >= constitution->rule_capacity) {
    size_t new_cap =
        constitution->rule_capacity == 0 ? 8 : constitution->rule_capacity * 2;
    civ_rule_t *new_rules =
        CIV_REALLOC(constitution->rules, new_cap * sizeof(civ_rule_t));
    if (!new_rules)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    constitution->rules = new_rules;
    constitution->rule_capacity = new_cap;
  }

  constitution->rules[constitution->rule_count++] = *rule;
  return (civ_result_t){CIV_OK, "Rule added"};
}

civ_result_t civ_constitution_remove_rule(civ_constitution_t *constitution,
                                          const char *rule_id) {
  if (!constitution || !rule_id)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  for (size_t i = 0; i < constitution->rule_count; i++) {
    if (strcmp(constitution->rules[i].id, rule_id) == 0) {
      // Found, remove by shifting
      if (i < constitution->rule_count - 1) {
        memmove(&constitution->rules[i], &constitution->rules[i + 1],
                sizeof(civ_rule_t) * (constitution->rule_count - 1 - i));
      }
      constitution->rule_count--;
      return (civ_result_t){CIV_OK, "Rule removed"};
    }
  }

  return (civ_result_t){CIV_ERROR_NOT_FOUND, "Rule not found"};
}

civ_rule_t *civ_constitution_find_rule(const civ_constitution_t *constitution,
                                       const char *rule_id) {
  if (!constitution || !rule_id)
    return NULL;

  for (size_t i = 0; i < constitution->rule_count; i++) {
    if (strcmp(constitution->rules[i].id, rule_id) == 0) {
      return &constitution->rules[i];
    }
  }
  return NULL;
}

bool civ_rule_is_valid_in_scope(const civ_rule_t *rule,
                                civ_rule_scope_t context_scope) {
  if (!rule)
    return false;
  // Simple check: Rule scope must match or be broader/narrower depending on
  // logic For now, exact match or rule is "higher" (e.g. National rule in Local
  // context?) Actually, usually a National rule applies to Local context. So if
  // context is LOCAL, rule can be LOCAL, REGIONAL, or NATIONAL. If context is
  // NATIONAL, rule must be NATIONAL or INTERNATIONAL.

  if (context_scope == CIV_RULE_SCOPE_LOCAL)
    return true; // All rules apply? No.

  // Let's say: A rule defined as NATIONAL is valid in a NATIONAL context.
  // A rule defined as LOCAL is valid in a LOCAL context.
  return rule->scope == context_scope;
}
