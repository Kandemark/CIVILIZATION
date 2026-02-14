/**
 * @file government.c
 * @brief Implementation of emergent, function-based governance
 */

#include "../../../include/core/governance/government.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_government_t *civ_government_create(const char *name) {
  civ_government_t *gov =
      (civ_government_t *)CIV_MALLOC(sizeof(civ_government_t));
  if (!gov)
    return NULL;

  civ_government_init(gov, name);
  return gov;
}

void civ_government_init(civ_government_t *gov, const char *name) {
  if (!gov)
    return;

  memset(gov, 0, sizeof(civ_government_t));
  strncpy(gov->name, name, STRING_MEDIUM_LEN - 1);
  gov->government_type = CIV_GOV_CHIEFDOM;

  gov->stability = 0.6f;
  gov->legitimacy = 0.6f;
  gov->efficiency = 0.4f;

  gov->function_capacity = 8;
  gov->functions = (civ_governance_function_t *)CIV_CALLOC(
      gov->function_capacity, sizeof(civ_governance_function_t));

  /* Phase 11: Modular Extensions Allocation */
  gov->institution_manager = civ_institution_manager_create();
  gov->subdivision_manager = civ_subdivision_manager_create();
  gov->legislative_manager = civ_legislative_manager_create();
  gov->legislative_threshold = 0.5f; /* Default: Simple Majority */
}

void civ_government_destroy(civ_government_t *gov) {
  if (!gov)
    return;

  CIV_FREE(gov->functions);
  if (gov->subunit_ids) {
    for (size_t i = 0; i < gov->subunit_count; i++) {
      CIV_FREE(gov->subunit_ids[i]);
    }
    CIV_FREE(gov->subunit_ids);
  }
  CIV_FREE(gov->decision_priorities);

  /* Phase 11: Modular Extensions Destruction */
  if (gov->institution_manager)
    civ_institution_manager_destroy(gov->institution_manager);
  if (gov->subdivision_manager)
    civ_subdivision_manager_destroy(gov->subdivision_manager);
  if (gov->legislative_manager)
    civ_legislative_manager_destroy(gov->legislative_manager);

  CIV_FREE(gov);
}

civ_result_t civ_government_assign_power(civ_government_t *gov,
                                         civ_power_type_t type,
                                         const char *role) {
  if (!gov || !role)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  /* Find existing or add new */
  for (size_t i = 0; i < gov->function_count; i++) {
    if (gov->functions[i].type == type) {
      strncpy(gov->functions[i].holder_role, role, STRING_SHORT_LEN - 1);
      return (civ_result_t){CIV_OK, NULL};
    }
  }

  if (gov->function_count >= gov->function_capacity) {
    gov->function_capacity *= 2;
    gov->functions = (civ_governance_function_t *)CIV_REALLOC(
        gov->functions,
        gov->function_capacity * sizeof(civ_governance_function_t));
  }

  if (gov->functions) {
    civ_governance_function_t *f = &gov->functions[gov->function_count++];
    f->type = type;
    strncpy(f->holder_role, role, STRING_SHORT_LEN - 1);
    f->voting_method = CIV_VOTE_METHOD_SIMPLE_MAJORITY;
    f->autonomy = 0.5f;
    return (civ_result_t){CIV_OK, NULL};
  }

  return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
}

civ_float_t civ_government_collect_taxes(civ_government_t *gov) {
  if (!gov)
    return 0.0f;

  /* Modifiers based on type */
  civ_float_t type_mult = 1.0f;
  switch (gov->government_type) {
  case CIV_GOV_DESPOTISM:
    type_mult = 1.2f; /* Stronger enforcement */
    gov->efficiency = MIN(1.0f, gov->efficiency + 0.05f);
    gov->legitimacy = MAX(0.1f, gov->legitimacy - 0.02f);
    break;
  case CIV_GOV_REPUBLIC:
    type_mult = 1.1f;
    gov->legitimacy = MIN(1.0f, gov->legitimacy + 0.02f);
    gov->efficiency = MIN(1.0f, gov->efficiency + 0.01f);
    break;
  case CIV_GOV_DEMOCRACY:
    type_mult = 0.9f; /* Bureaucratic cost */
    gov->legitimacy = MIN(1.0f, gov->legitimacy + 0.05f);
    gov->stability = MIN(1.0f, gov->stability + 0.01f);
    break;
  default:
    break;
  }

  /* Revenue is population-scale product of core metrics */
  civ_float_t base_revenue = 100.0f;
  return base_revenue * gov->efficiency * gov->stability * gov->legitimacy *
         type_mult;
}

void civ_government_update(civ_government_t *gov, civ_float_t time_delta) {
  if (!gov)
    return;

  /* Stability and Legitimacy drift toward baseline equilibrium (0.7) */
  float drift_speed = 0.02f;
  gov->stability += (0.7f - gov->stability) * drift_speed * time_delta;
  gov->legitimacy += (0.7f - gov->legitimacy) * drift_speed * time_delta;

  /* Stability affects efficiency */
  if (gov->stability < 0.3f) {
    gov->efficiency = MAX(0.1f, gov->efficiency - 0.05f * time_delta);
  } else if (gov->stability > 0.8f) {
    gov->efficiency = MIN(1.0f, gov->efficiency + 0.02f * time_delta);
  }

  /* Clamp all metrics */
  gov->stability = CLAMP(gov->stability, 0.0f, 1.0f);
  gov->legitimacy = CLAMP(gov->legitimacy, 0.0f, 1.0f);
  gov->efficiency = CLAMP(gov->efficiency, 0.0f, 1.0f);
}
