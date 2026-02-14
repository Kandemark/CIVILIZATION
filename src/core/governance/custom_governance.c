/**
 * @file custom_governance.c
 * @brief Implementation of custom governance
 */

#include "../../../include/core/governance/custom_governance.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_custom_governance_manager_t *civ_custom_governance_manager_create(void) {
  civ_custom_governance_manager_t *manager =
      (civ_custom_governance_manager_t *)CIV_MALLOC(
          sizeof(civ_custom_governance_manager_t));
  if (!manager) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate custom governance manager");
    return NULL;
  }

  civ_custom_governance_manager_init(manager);
  return manager;
}

void civ_custom_governance_manager_destroy(
    civ_custom_governance_manager_t *manager) {
  if (!manager)
    return;

  for (size_t i = 0; i < manager->government_count; i++) {
    civ_custom_governance_destroy(&manager->governments[i]);
  }
  CIV_FREE(manager->governments);
  CIV_FREE(manager);
}

void civ_custom_governance_manager_init(
    civ_custom_governance_manager_t *manager) {
  if (!manager)
    return;

  memset(manager, 0, sizeof(civ_custom_governance_manager_t));
  manager->government_capacity = 32;
  manager->governments = (civ_custom_governance_t *)CIV_CALLOC(
      manager->government_capacity, sizeof(civ_custom_governance_t));
}

civ_custom_governance_t *civ_custom_governance_create(const char *id,
                                                      const char *name) {
  if (!id || !name)
    return NULL;

  civ_custom_governance_t *gov =
      (civ_custom_governance_t *)CIV_MALLOC(sizeof(civ_custom_governance_t));
  if (!gov) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate custom governance");
    return NULL;
  }

  memset(gov, 0, sizeof(civ_custom_governance_t));
  strncpy(gov->id, id, sizeof(gov->id) - 1);
  strncpy(gov->name, name, sizeof(gov->name) - 1);
  gov->centralization = 0.5f;
  gov->democracy_level = 0.3f;
  gov->stability = 0.7f;
  gov->efficiency = 0.5f;
  gov->creation_time = time(NULL);
  gov->last_reform = gov->creation_time;
  gov->role_capacity = 16;
  gov->roles = (civ_governance_role_t *)CIV_CALLOC(
      gov->role_capacity, sizeof(civ_governance_role_t));

  gov->party_system = CIV_PARTY_NONE;

  // Initialize default constitution
  gov->constitution = civ_constitution_create(name);

  return gov;
}

void civ_custom_governance_destroy(civ_custom_governance_t *gov) {
  if (!gov)
    return;
  CIV_FREE(gov->roles);
  CIV_FREE(gov->custom_rules);
  if (gov->constitution) {
    civ_constitution_destroy(gov->constitution);
  }
}

civ_result_t civ_custom_governance_add_role(civ_custom_governance_t *gov,
                                            const char *role_name,
                                            const char *description,
                                            civ_float_t authority) {
  civ_result_t result = {CIV_OK, NULL};

  if (!gov || !role_name) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  authority = CLAMP(authority, 0.0f, 1.0f);

  if (gov->role_count >= gov->role_capacity) {
    gov->role_capacity *= 2;
    gov->roles = (civ_governance_role_t *)CIV_REALLOC(
        gov->roles, gov->role_capacity * sizeof(civ_governance_role_t));
  }

  if (gov->roles) {
    civ_governance_role_t *role = &gov->roles[gov->role_count++];
    strncpy(role->role_name, role_name, sizeof(role->role_name) - 1);
    if (description) {
      strncpy(role->description, description, sizeof(role->description) - 1);
    }
    role->authority = authority;
    role->responsibility = authority * 0.8f;
    role->holder_count = 1;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_result_t civ_custom_governance_reform(civ_custom_governance_t *gov,
                                          const char *reform_description) {
  civ_result_t result = {CIV_OK, NULL};

  if (!gov || !reform_description) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  size_t desc_len = strlen(reform_description);
  if (gov->custom_rules) {
    size_t new_size = gov->custom_rules_size + desc_len + 2;
    gov->custom_rules = (char *)CIV_REALLOC(gov->custom_rules, new_size);
  } else {
    gov->custom_rules = (char *)CIV_MALLOC(desc_len + 1);
    gov->custom_rules_size = 0;
  }

  if (gov->custom_rules) {
    if (gov->custom_rules_size > 0) {
      strcat(gov->custom_rules, "\n");
    }
    strcat(gov->custom_rules, reform_description);
    gov->custom_rules_size += desc_len + 1;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
    return result;
  }

  gov->last_reform = time(NULL);

  /* Apply stats changes based on keywords in reform description */
  if (strstr(reform_description, "Centralize") ||
      strstr(reform_description, "centralize")) {
    gov->centralization += 0.1f;
    gov->democracy_level -= 0.05f;
  } else if (strstr(reform_description, "Decentralize") ||
             strstr(reform_description, "decentralize")) {
    gov->centralization -= 0.1f;
    gov->democracy_level += 0.05f;
  }

  if (strstr(reform_description, "Democratize") ||
      strstr(reform_description, "democratize")) {
    gov->democracy_level += 0.1f;
  } else if (strstr(reform_description, "Autocratize") ||
             strstr(reform_description, "autocratize")) {
    gov->democracy_level -= 0.1f;
  }

  // Clamp values
  gov->centralization = CLAMP(gov->centralization, 0.0f, 1.0f);
  gov->democracy_level = CLAMP(gov->democracy_level, 0.0f, 1.0f);

  /* Reforms temporarily reduce stability */
  gov->stability = CLAMP(gov->stability - 0.15f, 0.05f, 1.0f);

  return result;
}

civ_result_t civ_custom_governance_update(civ_custom_governance_t *gov,
                                          civ_float_t time_delta) {
  civ_result_t result = {CIV_OK, NULL};

  if (!gov) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Efficiency based on centralization and role distribution */
  civ_float_t role_efficiency = 0.0f;
  if (gov->role_count > 0) {
    for (size_t i = 0; i < gov->role_count; i++) {
      role_efficiency += gov->roles[i].responsibility * gov->roles[i].authority;
    }
    role_efficiency /= (civ_float_t)gov->role_count;
  } else {
    // No roles defined? Low efficiency
    role_efficiency = 0.1f;
  }

  // Efficiency benefit from centralization, but capped by corruption if low
  // democracy
  civ_float_t central_eff = gov->centralization;
  if (gov->democracy_level < 0.3f) {
    // Autocracy efficiency penalty (corruption)
    central_eff *= 0.8f;
  }

  gov->efficiency = central_eff * 0.5f + role_efficiency * 0.5f;

  /* Stability Logic */
  // Natural recovery
  civ_float_t stability_recovery = 0.02f * time_delta;

  // Tension from opposing values (High Centralization + High Democracy =
  // Tension? Or Low Centralization + Low Democracy = Chaos?) Let's model: High
  // Centralization reduces stability IF democracy is high (people want local
  // control?) Actually, usually High Centralization + High Democracy is fine
  // (Unitary State). Let's model: Rapid changes cause instability (handled in
  // reform). Let's model: Low Efficiency reduces Stability.
  if (gov->efficiency < 0.3f) {
    stability_recovery -= 0.05f * time_delta;
  }

  civ_custom_governance_evolve(gov, time_delta);

  gov->stability = CLAMP(gov->stability + stability_recovery, 0.0f, 1.0f);

  return result;
}

void civ_custom_governance_evolve(civ_custom_governance_t *gov,
                                  civ_float_t time_delta) {
  if (!gov)
    return;

  // Corruption Growth:
  // Grows faster with high centralization if Democracy is low (no
  // checks/balances). Grows faster with low Stability (chaos).
  civ_float_t corruption_growth = 0.0f;
  if (gov->democracy_level < 0.5f) {
    corruption_growth += 0.01f * gov->centralization;
  }
  if (gov->stability < 0.4f) {
    corruption_growth += 0.02f;
  }
  // Democracy reduces corruption (transparency)
  if (gov->democracy_level > 0.7f) {
    corruption_growth -= 0.02f;
  }

  gov->corruption =
      CLAMP(gov->corruption + corruption_growth * time_delta, 0.0f, 1.0f);

  // Tension Update:
  // Tension rises if Efficiency is low or Corruption is high
  if (gov->efficiency < 0.4f || gov->corruption > 0.6f) {
    gov->political_tension += 0.05f * time_delta;
  } else {
    gov->political_tension -= 0.02f * time_delta;
  }
  gov->political_tension = CLAMP(gov->political_tension, 0.0f, 1.0f);

  // Forced Reform / Drift (The "Internal Unit" acting on itself)
  if (gov->political_tension > 0.8f) {
    // Crisis! Government structure shifts to relieve tension.
    // Usually towards extremes:
    if (gov->centralization > 0.5f) {
      // Tighten grip to maintain order
      gov->centralization += 0.05f;
      gov->democracy_level -= 0.05f;
      // But prolonged crisis might shatter centralization
    } else {
      // Anarchy/Decentralization
      gov->centralization -= 0.05f;
    }

    // Reset tension slightly after "event"
    gov->political_tension -= 0.1f;
    // Stability hit
    gov->stability -= 0.1f;
  }

  // Clamp all
  gov->centralization = CLAMP(gov->centralization, 0.0f, 1.0f);
  gov->democracy_level = CLAMP(gov->democracy_level, 0.0f, 1.0f);
}

void civ_custom_governance_generate_name(civ_custom_governance_t *gov,
                                         char *out_name, size_t max_len) {
  if (!gov || !out_name)
    return;

  const char *adj = "";
  const char *noun = "Government";

  // Noun selection
  if (gov->centralization > 0.8f) {
    if (gov->democracy_level < 0.2f)
      noun = "Empire";
    else if (gov->democracy_level < 0.5f)
      noun = "Autocracy";
    else
      noun = "Union"; // Centralized Democracy
  } else if (gov->centralization < 0.2f) {
    if (gov->democracy_level > 0.8f)
      noun = "Commune";
    else
      noun = "Confederacy";
  } else {
    if (gov->democracy_level > 0.6f)
      noun = "Republic";
    else
      noun = "State";
  }

  // Adjective selection
  if (gov->corruption > 0.7f)
    adj = "Corrupt ";
  else if (gov->stability < 0.3f)
    adj = "Fractured ";
  // else if (gov->military_power > 0.8f) adj = "Martial "; // hypothetical
  // field
  else if (gov->efficiency > 0.8f)
    adj = "Ordered ";
  else if (gov->democracy_level > 0.9f)
    adj = "Free ";

  snprintf(out_name, max_len, "%s%s", adj, noun);
}

civ_result_t
civ_custom_governance_manager_add(civ_custom_governance_manager_t *manager,
                                  civ_custom_governance_t *gov) {
  civ_result_t result = {CIV_OK, NULL};

  if (!manager || !gov) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (manager->government_count >= manager->government_capacity) {
    manager->government_capacity *= 2;
    manager->governments = (civ_custom_governance_t *)CIV_REALLOC(
        manager->governments,
        manager->government_capacity * sizeof(civ_custom_governance_t));
  }

  if (manager->governments) {
    // Note: Shallow copy of gov struct. If gov has pointers (roles,
    // constitution), we must be careful who owns them. The manager seems to
    // store by value `civ_custom_governance_t` in the array? Yes:
    // `civ_custom_governance_t *governments`. If we copy *gov into
    // manager->governments[i], we copy the pointers. We should probably assume
    // ownership transfer or deep copy. For simplicity here, we assume ownership
    // transfer. The caller shouldn't free the passed `gov` pointer if it was
    // malloced separately, OR `gov` was stack allocated. Given
    // `civ_custom_governance_create` returns a pointer, we should probably copy
    // the CONTENT and then free the input container, or just store pointers in
    // the manager. The current struct uses `civ_custom_governance_t
    // *governments;` (array of structs). So we copy the content.
    manager->governments[manager->government_count++] = *gov;
    // IMPORTANT: The original `gov` pointer from create() should be freed by
    // the caller, BUT `gov->roles` etc are now pointed to by the manager's
    // copy. So the caller MUST NOT call destroy() on the original `gov`. We
    // will just Free the container.
    CIV_FREE(gov);
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_custom_governance_t *civ_custom_governance_manager_find(
    const civ_custom_governance_manager_t *manager, const char *id) {
  if (!manager || !id)
    return NULL;

  for (size_t i = 0; i < manager->government_count; i++) {
    if (strcmp(manager->governments[i].id, id) == 0) {
      return (civ_custom_governance_t *)&manager->governments[i];
    }
  }

  return NULL;
}

civ_result_t civ_custom_governance_map_title(civ_custom_governance_t *gov,
                                             const char *title,
                                             const char *functional_role) {
  if (!gov || !title || !functional_role)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  /* Log the mapping of a linguistic title to a functional role */
  civ_log(CIV_LOG_INFO, "Mapped title '%s' to functional role '%s' in gov %s",
          title, functional_role, gov->id);

  /* In a full implementation, this could update a role name or registry */
  return (civ_result_t){CIV_OK, NULL};
}
