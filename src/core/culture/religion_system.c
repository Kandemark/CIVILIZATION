/**
 * @file religion_system.c
 * @brief Implementation of emergent religions
 */

#include "../../../include/core/culture/religion_system.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_religion_system_t *civ_religion_system_create(void) {
  civ_religion_system_t *system =
      (civ_religion_system_t *)CIV_MALLOC(sizeof(civ_religion_system_t));
  if (!system)
    return NULL;

  system->religion_capacity = 16;
  system->religion_count = 0;
  system->religions = (civ_religion_t *)CIV_CALLOC(system->religion_capacity,
                                                   sizeof(civ_religion_t));

  return system;
}

void civ_religion_system_destroy(civ_religion_system_t *system) {
  if (!system)
    return;
  CIV_FREE(system->religions);
  CIV_FREE(system);
}

civ_religion_t *civ_religion_emerge(civ_religion_system_t *system,
                                    const char *name, const char *culture_id) {
  if (!system || !name || !culture_id)
    return NULL;

  if (system->religion_count >= system->religion_capacity) {
    system->religion_capacity *= 2;
    system->religions = (civ_religion_t *)CIV_REALLOC(
        system->religions, system->religion_capacity * sizeof(civ_religion_t));
  }

  if (system->religions) {
    civ_religion_t *rel = &system->religions[system->religion_count++];
    memset(rel, 0, sizeof(civ_religion_t));
    strncpy(rel->id, name, STRING_SHORT_LEN - 1); /* Simple ID generation */
    strncpy(rel->name, name, STRING_MEDIUM_LEN - 1);
    strncpy(rel->origin_culture_id, culture_id, STRING_SHORT_LEN - 1);

    rel->fervor = 0.5f;
    rel->global_reach = 0.01f; /* Starts small */
    rel->creation_time = time(NULL);

    /* Procedural tenets based on rand for now */
    rel->tenet_count = 2;
    rel->tenets[0] = (civ_religion_tenet_t)(rand() % 6);
    rel->tenets[1] = (civ_religion_tenet_t)(rand() % 6);

    return rel;
  }

  return NULL;
}

civ_result_t civ_religion_spread(civ_religion_t *religion,
                                 const char *target_region_id,
                                 civ_float_t rate) {
  if (!religion || !target_region_id)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  /* Logic to convert pop in region based on rate/fervor/current reach */
  religion->global_reach += rate * religion->fervor * 0.01f;
  religion->global_reach = CLAMP(religion->global_reach, 0.0f, 1.0f);

  civ_log(CIV_LOG_INFO, "Religion %s spread to %s (New Reach: %.2f)",
          religion->name, target_region_id, religion->global_reach);

  return (civ_result_t){CIV_OK, NULL};
}
