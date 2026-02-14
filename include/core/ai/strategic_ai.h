/**
 * @file strategic_ai.h
 * @brief Strategic AI for long-term planning
 */

#ifndef CIVILIZATION_STRATEGIC_AI_H
#define CIVILIZATION_STRATEGIC_AI_H

#include "../../common.h"
#include "../../types.h"
#include "../diplomacy/relations.h"
#include "base_ai.h"

/* Strategic goal */
typedef struct {
  char goal_type[STRING_SHORT_LEN];
  char description[STRING_MAX_LEN];
  civ_float_t priority;
  civ_float_t progress; /* 0.0 to 1.0 */
  time_t deadline;
  time_t created;
} civ_strategic_goal_t;

/* Strategic AI */
typedef struct {
  civ_base_ai_t *base_ai;

  civ_strategic_goal_t *goals;
  size_t goal_count;
  size_t goal_capacity;

  civ_float_t planning_horizon; /* Years ahead to plan */
  civ_float_t risk_tolerance;   /* 0.0 to 1.0 */

  int32_t last_expansion_turn;
  int32_t expansion_frequency;
  civ_personality_type_t personality;

  void *game_ptr; /* Opaque pointer to civ_game_t */
} civ_strategic_ai_t;

/* Function declarations */
civ_strategic_ai_t *civ_strategic_ai_create(const char *id, const char *name);
void civ_strategic_ai_destroy(civ_strategic_ai_t *ai);
void civ_strategic_ai_init(civ_strategic_ai_t *ai, const char *id,
                           const char *name);

civ_result_t civ_strategic_ai_plan(civ_strategic_ai_t *ai,
                                   civ_float_t time_delta);
civ_result_t civ_strategic_ai_add_goal(civ_strategic_ai_t *ai,
                                       const char *goal_type,
                                       const char *description,
                                       civ_float_t priority);
civ_strategic_goal_t *
civ_strategic_ai_get_priority_goal(const civ_strategic_ai_t *ai);
civ_result_t civ_strategic_ai_update_goal(civ_strategic_ai_t *ai,
                                          size_t goal_index,
                                          civ_float_t progress);

/* Proactive Actions */
civ_result_t civ_strategic_ai_evaluate_threats(civ_strategic_ai_t *ai,
                                               void *game_ptr);
bool civ_strategic_ai_should_declare_war(civ_strategic_ai_t *ai, void *game_ptr,
                                         const char *target_id);
bool civ_strategic_ai_should_offer_peace(civ_strategic_ai_t *ai, void *game_ptr,
                                         const char *target_id);

civ_result_t civ_strategic_ai_process_expansion(civ_strategic_ai_t *ai,
                                                void *game_ptr);

#endif /* CIVILIZATION_STRATEGIC_AI_H */
