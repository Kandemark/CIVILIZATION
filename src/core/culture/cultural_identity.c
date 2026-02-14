/**
 * @file cultural_identity.c
 * @brief Implementation of cultural identity system
 */

#include "../../../include/core/culture/cultural_identity.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_cultural_identity_manager_t *civ_cultural_identity_manager_create(void) {
  civ_cultural_identity_manager_t *manager =
      (civ_cultural_identity_manager_t *)CIV_MALLOC(
          sizeof(civ_cultural_identity_manager_t));
  if (!manager) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate cultural identity manager");
    return NULL;
  }

  civ_cultural_identity_manager_init(manager);
  return manager;
}

void civ_cultural_identity_manager_destroy(
    civ_cultural_identity_manager_t *manager) {
  if (!manager)
    return;

  for (size_t i = 0; i < manager->identity_count; i++) {
    civ_cultural_identity_destroy(&manager->identities[i]);
  }
  CIV_FREE(manager->identities);
  CIV_FREE(manager);
}

void civ_cultural_identity_manager_init(
    civ_cultural_identity_manager_t *manager) {
  if (!manager)
    return;

  memset(manager, 0, sizeof(civ_cultural_identity_manager_t));
  manager->identity_capacity = 32;
  manager->identities = (civ_cultural_identity_t *)CIV_CALLOC(
      manager->identity_capacity, sizeof(civ_cultural_identity_t));
}

civ_cultural_identity_t *civ_cultural_identity_create(const char *id,
                                                      const char *name) {
  if (!id || !name)
    return NULL;

  civ_cultural_identity_t *identity =
      (civ_cultural_identity_t *)CIV_MALLOC(sizeof(civ_cultural_identity_t));
  if (!identity) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate cultural identity");
    return NULL;
  }

  memset(identity, 0, sizeof(civ_cultural_identity_t));
  strncpy(identity->id, id, sizeof(identity->id) - 1);
  strncpy(identity->name, name, sizeof(identity->name) - 1);
  identity->cohesion = 0.5f;
  identity->distinctiveness = 0.5f;
  identity->influence_radius = 10.0f;
  identity->creation_time = time(NULL);
  identity->last_update = identity->creation_time;
  identity->trait_capacity = 16;
  identity->traits = (civ_cultural_trait_t *)CIV_CALLOC(
      identity->trait_capacity, sizeof(civ_cultural_trait_t));

  identity->value_capacity = 8;
  identity->core_values = (civ_cultural_value_t *)CIV_CALLOC(
      identity->value_capacity, sizeof(civ_cultural_value_t));

  memset(identity->parent_culture_id, 0, STRING_SHORT_LEN);

  return identity;
}

void civ_cultural_identity_destroy(civ_cultural_identity_t *identity) {
  if (!identity)
    return;
  CIV_FREE(identity->traits);
  CIV_FREE(identity->core_values);
}

civ_result_t civ_cultural_identity_add_trait(civ_cultural_identity_t *identity,
                                             const char *trait_name,
                                             civ_float_t strength) {
  civ_result_t result = {CIV_OK, NULL};

  if (!identity || !trait_name) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  strength = CLAMP(strength, 0.0f, 1.0f);

  /* Expand if needed */
  if (identity->trait_count >= identity->trait_capacity) {
    identity->trait_capacity *= 2;
    identity->traits = (civ_cultural_trait_t *)CIV_REALLOC(
        identity->traits,
        identity->trait_capacity * sizeof(civ_cultural_trait_t));
  }

  if (identity->traits) {
    civ_cultural_trait_t *trait = &identity->traits[identity->trait_count++];
    strncpy(trait->name, trait_name, sizeof(trait->name) - 1);
    trait->strength = strength;
    trait->influence = strength * 0.5f;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_result_t civ_cultural_identity_update(civ_cultural_identity_t *identity,
                                          civ_float_t time_delta) {
  civ_result_t result = {CIV_OK, NULL};

  if (!identity) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Update trait influences */
  for (size_t i = 0; i < identity->trait_count; i++) {
    civ_cultural_trait_t *trait = &identity->traits[i];
    trait->influence = trait->strength * identity->cohesion;
  }

  /* Update distinctiveness based on traits */
  if (identity->trait_count > 0) {
    civ_float_t avg_strength = 0.0f;
    for (size_t i = 0; i < identity->trait_count; i++) {
      avg_strength += identity->traits[i].strength;
    }
    avg_strength /= (civ_float_t)identity->trait_count;
    identity->distinctiveness = avg_strength;
  }

  identity->last_update = time(NULL);
  return result;
}

civ_float_t
civ_cultural_identity_calculate_similarity(const civ_cultural_identity_t *a,
                                           const civ_cultural_identity_t *b) {
  if (!a || !b)
    return 0.0f;

  /* Calculate similarity based on shared traits */
  civ_float_t similarity = 0.0f;
  size_t matches = 0;

  for (size_t i = 0; i < a->trait_count; i++) {
    for (size_t j = 0; j < b->trait_count; j++) {
      if (strcmp(a->traits[i].name, b->traits[j].name) == 0) {
        civ_float_t strength_diff = (civ_float_t)fabs(
            (double)(a->traits[i].strength - b->traits[j].strength));
        similarity += 1.0f - strength_diff;
        matches++;
        break;
      }
    }
  }

  if (matches > 0) {
    similarity /= (civ_float_t)matches;
  }

  return CLAMP(similarity, 0.0f, 1.0f);
}

civ_result_t
civ_cultural_identity_manager_add(civ_cultural_identity_manager_t *manager,
                                  civ_cultural_identity_t *identity) {
  civ_result_t result = {CIV_OK, NULL};

  if (!manager || !identity) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Expand if needed */
  if (manager->identity_count >= manager->identity_capacity) {
    manager->identity_capacity *= 2;
    manager->identities = (civ_cultural_identity_t *)CIV_REALLOC(
        manager->identities,
        manager->identity_capacity * sizeof(civ_cultural_identity_t));
  }

  if (manager->identities) {
    manager->identities[manager->identity_count++] = *identity;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_cultural_identity_t *civ_cultural_identity_manager_find(
    const civ_cultural_identity_manager_t *manager, const char *id) {
  if (!manager || !id)
    return NULL;

  for (size_t i = 0; i < manager->identity_count; i++) {
    if (strcmp(manager->identities[i].id, id) == 0) {
      return (civ_cultural_identity_t *)&manager->identities[i];
    }
  }

  return NULL;
}

civ_cultural_identity_t *
civ_cultural_identity_split(civ_cultural_identity_manager_t *manager,
                            const civ_cultural_identity_t *parent,
                            const char *name) {
  if (!manager || !parent || !name)
    return NULL;

  char new_id[STRING_SHORT_LEN];
  snprintf(new_id, STRING_SHORT_LEN, "%s_br", parent->id);

  civ_cultural_identity_t *child = civ_cultural_identity_create(new_id, name);
  if (child) {
    strncpy(child->parent_culture_id, parent->id, STRING_SHORT_LEN - 1);
    child->cohesion = parent->cohesion * 0.8f;

    /* Inherit traits with variation */
    for (size_t i = 0; i < parent->trait_count; i++) {
      civ_cultural_identity_add_trait(child, parent->traits[i].name,
                                      parent->traits[i].strength * 0.9f);
    }
  }

  civ_cultural_identity_manager_add(manager, child);
  return child;
}

char *civ_cultural_identity_generate_custom_name(
    const civ_cultural_identity_t *identity, const char *type) {
  if (!identity || !type)
    return NULL;

  /* Simplified procedural name based on culture name */
  char *custom_name = (char *)CIV_MALLOC(STRING_MEDIUM_LEN);
  if (custom_name) {
    snprintf(custom_name, STRING_MEDIUM_LEN, "%s %s", identity->name, type);
  }
  return custom_name;
}
