/**
 * @file ideology_system.c
 * @brief Implementation of Emergent Ideology System
 */

#include "../../../include/core/culture/ideology_system.h"

civ_ideology_system_t *civ_ideology_system_create(void) {
  civ_ideology_system_t *system = CIV_MALLOC(sizeof(civ_ideology_system_t));
  if (system) {
    system->ideologies = NULL;
    system->ideology_count = 0;
    system->ideology_capacity = 0;
  }
  return system;
}

void civ_ideology_system_destroy(civ_ideology_system_t *system) {
  if (system) {
    if (system->ideologies) {
      for (size_t i = 0; i < system->ideology_count; i++) {
        // Deep clean would be needed here if ideologies own their memory deeply
        // For now assuming flat array for simplicity or need helper
      }
      CIV_FREE(system->ideologies);
    }
    CIV_FREE(system);
  }
}

civ_ideology_t *civ_ideology_create(const char *name) {
  civ_ideology_t *ideology = CIV_MALLOC(sizeof(civ_ideology_t));
  if (ideology) {
    strncpy(ideology->name, name, STRING_MEDIUM_LEN - 1);
    ideology->name[STRING_MEDIUM_LEN - 1] = '\0';

    ideology->values = NULL;
    ideology->value_count = 0;
    ideology->value_capacity = 0;

    ideology->policies = NULL;
    ideology->policy_count = 0;
    ideology->policy_capacity = 0;

    ideology->coherence = 1.0f;
    ideology->radicalism = 0.0f;
  }
  return ideology;
}

void civ_ideology_destroy(civ_ideology_t *ideology) {
  if (ideology) {
    CIV_FREE(ideology->values);
    CIV_FREE(ideology->policies);
    CIV_FREE(ideology);
  }
}

civ_result_t civ_ideology_set_value(civ_ideology_t *ideology, const char *axis,
                                    civ_float_t value) {
  if (!ideology || !axis)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid arguments"};

  // Clamp value
  value = CLAMP(value, -1.0f, 1.0f);

  // Find existing
  for (size_t i = 0; i < ideology->value_count; i++) {
    if (strcmp(ideology->values[i].name, axis) == 0) {
      ideology->values[i].value = value;
      civ_ideology_update_metrics(ideology);
      return (civ_result_t){CIV_OK, "Value updated"};
    }
  }

  // Add new
  if (ideology->value_count >= ideology->value_capacity) {
    size_t new_cap =
        ideology->value_capacity == 0 ? 4 : ideology->value_capacity * 2;
    civ_ideology_value_t *new_values =
        CIV_REALLOC(ideology->values, new_cap * sizeof(civ_ideology_value_t));
    if (!new_values)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "Out of memory"};
    ideology->values = new_values;
    ideology->value_capacity = new_cap;
  }

  strncpy(ideology->values[ideology->value_count].name, axis,
          STRING_SHORT_LEN - 1);
  ideology->values[ideology->value_count].name[STRING_SHORT_LEN - 1] = '\0';
  ideology->values[ideology->value_count].value = value;
  ideology->value_count++;

  civ_ideology_update_metrics(ideology);
  return (civ_result_t){CIV_OK, "Value added"};
}

civ_float_t civ_ideology_get_value(const civ_ideology_t *ideology,
                                   const char *axis) {
  if (!ideology || !axis)
    return 0.0f;

  for (size_t i = 0; i < ideology->value_count; i++) {
    if (strcmp(ideology->values[i].name, axis) == 0) {
      return ideology->values[i].value;
    }
  }
  return 0.0f; // Default neutral
}

civ_result_t civ_ideology_evolve(civ_ideology_t *ideology, const char *axis,
                                 civ_float_t shift) {
  civ_float_t current = civ_ideology_get_value(ideology, axis);
  return civ_ideology_set_value(ideology, axis, current + shift);
}

void civ_ideology_update_metrics(civ_ideology_t *ideology) {
  if (!ideology || ideology->value_count == 0)
    return;

  civ_float_t total_mag = 0.0f;
  for (size_t i = 0; i < ideology->value_count; i++) {
    total_mag += fabs(ideology->values[i].value);
  }

  ideology->radicalism = total_mag / ideology->value_count;
  // Coherence calc is complex, placeholder for now
  ideology->coherence = 1.0f;
}

civ_float_t civ_ideology_distance(const civ_ideology_t *a,
                                  const civ_ideology_t *b) {
  if (!a || !b)
    return 0.0f;

  // Simple Euclidean-like distance on shared axes
  civ_float_t dist_sq = 0.0f;
  // This is O(N*M), could be optimized if axes are sorted or hashed
  for (size_t i = 0; i < a->value_count; i++) {
    civ_float_t val_b = civ_ideology_get_value(b, a->values[i].name);
    civ_float_t diff = a->values[i].value - val_b;
    dist_sq += diff * diff;
  }

  // Check for axes in B not in A
  for (size_t i = 0; i < b->value_count; i++) {
    bool in_a = false;
    for (size_t j = 0; j < a->value_count; j++) {
      if (strcmp(b->values[i].name, a->values[j].name) == 0) {
        in_a = true;
        break;
      }
    }
    if (!in_a) {
      civ_float_t diff = b->values[i].value; // A has 0
      dist_sq += diff * diff;
    }
  }

  return sqrt(dist_sq);
}

civ_ideology_t *civ_ideology_split(civ_ideology_system_t *system,
                                   const civ_ideology_t *parent,
                                   const char *name) {
  if (!system || !parent || !name)
    return NULL;

  civ_ideology_t *child = civ_ideology_create(name);
  if (child) {
    strncpy(child->parent_ideology_id, parent->id, STRING_SHORT_LEN - 1);

    /* Copy values with slight variation */
    for (size_t i = 0; i < parent->value_count; i++) {
      civ_float_t variant = (civ_float_t)((rand() % 100 - 50) * 0.01f);
      civ_ideology_set_value(child, parent->values[i].name,
                             parent->values[i].value + variant);
    }
  }
  return child;
}

civ_result_t civ_ideology_drift(civ_ideology_t *ideology,
                                civ_float_t corruption, civ_float_t stability) {
  if (!ideology)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null ideology"};

  /* High corruption pushes towards radicalism or specific axes */
  if (corruption > 0.5f) {
    civ_ideology_evolve(ideology, "Authority", 0.05f * corruption);
  }

  /* Low stability causes random drift in values */
  if (stability < 0.4f) {
    for (size_t i = 0; i < ideology->value_count; i++) {
      civ_float_t shift =
          (civ_float_t)((rand() % 100 - 50) * 0.001f * (1.0f - stability));
      ideology->values[i].value =
          CLAMP(ideology->values[i].value + shift, -1.0f, 1.0f);
    }
  }

  civ_ideology_update_metrics(ideology);
  return (civ_result_t){CIV_OK, NULL};
}
