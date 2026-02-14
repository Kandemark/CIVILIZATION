/**
 * @file interaction.c
 * @brief Implementation of flexible interaction system
 */

#include "../../../include/core/governance/interaction.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_interaction_manager_t *civ_interaction_manager_create(void) {
  civ_interaction_manager_t *manager = (civ_interaction_manager_t *)CIV_MALLOC(
      sizeof(civ_interaction_manager_t));
  if (!manager)
    return NULL;

  memset(manager, 0, sizeof(civ_interaction_manager_t));
  manager->interaction_capacity = 16;
  manager->active_interactions = (civ_interaction_t *)CIV_CALLOC(
      manager->interaction_capacity, sizeof(civ_interaction_t));

  return manager;
}

void civ_interaction_manager_destroy(civ_interaction_manager_t *manager) {
  if (!manager)
    return;

  for (size_t i = 0; i < manager->interaction_count; i++) {
    CIV_FREE(manager->active_interactions[i].participants);
  }
  CIV_FREE(manager->active_interactions);
  CIV_FREE(manager);
}

civ_interaction_t *civ_interaction_create(civ_interaction_type_t type) {
  civ_interaction_t *interaction =
      (civ_interaction_t *)CIV_MALLOC(sizeof(civ_interaction_t));
  if (!interaction)
    return NULL;

  memset(interaction, 0, sizeof(civ_interaction_t));
  interaction->type = type;
  interaction->status = CIV_INTERACTION_STATUS_PENDING;
  interaction->participant_capacity = 8;
  interaction->participants = (civ_interaction_participant_t *)CIV_CALLOC(
      interaction->participant_capacity, sizeof(civ_interaction_participant_t));

  strncpy(interaction->translation_mode, "Direct", STRING_SHORT_LEN - 1);
  interaction->start_time = time(NULL);

  return interaction;
}

civ_result_t civ_interaction_add_participant(civ_interaction_t *interaction,
                                             const char *id, const char *role,
                                             const char *lang_id) {
  civ_result_t result = {CIV_OK, NULL};
  if (!interaction || !id || !role || !lang_id) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (interaction->participant_count >= interaction->participant_capacity) {
    interaction->participant_capacity *= 2;
    interaction->participants = (civ_interaction_participant_t *)CIV_REALLOC(
        interaction->participants, interaction->participant_capacity *
                                       sizeof(civ_interaction_participant_t));
  }

  if (interaction->participants) {
    civ_interaction_participant_t *p =
        &interaction->participants[interaction->participant_count++];
    strncpy(p->id, id, STRING_SHORT_LEN - 1);
    strncpy(p->role, role, STRING_SHORT_LEN - 1);
    strncpy(p->native_language_id, lang_id, STRING_SHORT_LEN - 1);
    p->relationship = 0.5f;
    p->influence = 1.0f;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_result_t civ_interaction_start(civ_interaction_t *interaction) {
  if (!interaction)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null interaction"};
  interaction->status = CIV_INTERACTION_STATUS_ACTIVE;
  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_interaction_update(civ_interaction_t *interaction,
                                    civ_float_t time_delta) {
  if (!interaction || interaction->status != CIV_INTERACTION_STATUS_ACTIVE) {
    return (civ_result_t){CIV_OK, NULL};
  }
  /* Update understanding or relationship based on interaction progress */
  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_interaction_conclude(civ_interaction_t *interaction) {
  if (!interaction)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null interaction"};
  interaction->status = CIV_INTERACTION_STATUS_CONCLUDED;
  interaction->end_time = time(NULL);
  return (civ_result_t){CIV_OK, NULL};
}

civ_float_t civ_interaction_calculate_understanding(
    const civ_interaction_t *interaction,
    const civ_language_evolution_t *lang_env) {
  if (!interaction || !lang_env || interaction->participant_count < 2)
    return 1.0f;

  /* Simplified: Check similarity between first two participants' languages */
  const char *lang1_id = interaction->participants[0].native_language_id;
  const char *lang2_id = interaction->participants[1].native_language_id;

  civ_language_t *lang1 = civ_language_evolution_find(lang_env, lang1_id);
  civ_language_t *lang2 = civ_language_evolution_find(lang_env, lang2_id);

  if (!lang1 || !lang2)
    return 0.2f; /* Basic gestures only */
  if (strcmp(lang1_id, lang2_id) == 0)
    return 1.0f;

  return civ_language_calculate_similarity(lang1, lang2);
}
