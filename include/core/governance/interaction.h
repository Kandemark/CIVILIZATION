/**
 * @file interaction.h
 * @brief Flexible NPC Interaction System
 */

#ifndef CIVILIZATION_INTERACTION_H
#define CIVILIZATION_INTERACTION_H

#include "../../common.h"
#include "../../types.h"
#include "../culture/language_evolution.h"

/* Interaction Type */
typedef enum {
  CIV_INTERACTION_PRIVATE_MEETING = 0,
  CIV_INTERACTION_SUMMIT,
  CIV_INTERACTION_SECRETARY_BRIEFING,
  CIV_INTERACTION_COURT_HEARING,
  CIV_INTERACTION_DIPLOMATIC_MEETING
} civ_interaction_type_t;

/* Interaction Status */
typedef enum {
  CIV_INTERACTION_STATUS_PENDING = 0,
  CIV_INTERACTION_STATUS_ACTIVE,
  CIV_INTERACTION_STATUS_CONCLUDED,
  CIV_INTERACTION_STATUS_CANCELLED
} civ_interaction_status_t;

/* Participant */
typedef struct {
  char id[STRING_SHORT_LEN];
  char role[STRING_SHORT_LEN];
  char native_language_id[STRING_SHORT_LEN];
  civ_float_t relationship; /* With the player/initiator */
  civ_float_t influence;
} civ_interaction_participant_t;

/* Interaction */
typedef struct {
  char id[STRING_SHORT_LEN];
  civ_interaction_type_t type;
  civ_interaction_status_t status;

  civ_interaction_participant_t *participants;
  size_t participant_count;
  size_t participant_capacity;

  char translation_mode[STRING_SHORT_LEN]; /* "Direct", "Translator", "Magic" ;)
                                            */
  civ_float_t mutual_understanding;        /* 0.0 to 1.0, based on languages */

  time_t start_time;
  time_t end_time;
} civ_interaction_t;

/* Interaction Manager */
typedef struct {
  civ_interaction_t *active_interactions;
  size_t interaction_count;
  size_t interaction_capacity;
} civ_interaction_manager_t;

/* Functions */
civ_interaction_manager_t *civ_interaction_manager_create(void);
void civ_interaction_manager_destroy(civ_interaction_manager_t *manager);

civ_interaction_t *civ_interaction_create(civ_interaction_type_t type);
civ_result_t civ_interaction_add_participant(civ_interaction_t *interaction,
                                             const char *id, const char *role,
                                             const char *lang_id);

/* Processing */
civ_result_t civ_interaction_start(civ_interaction_t *interaction);
civ_result_t civ_interaction_update(civ_interaction_t *interaction,
                                    civ_float_t time_delta);
civ_result_t civ_interaction_conclude(civ_interaction_t *interaction);

/* Language Logic */
civ_float_t civ_interaction_calculate_understanding(
    const civ_interaction_t *interaction,
    const civ_language_evolution_t *lang_env);

#endif /* CIVILIZATION_INTERACTION_H */
