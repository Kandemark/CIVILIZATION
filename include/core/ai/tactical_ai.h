/**
 * @file tactical_ai.h
 * @brief Tactical AI for short-term decisions
 */

#ifndef CIVILIZATION_TACTICAL_AI_H
#define CIVILIZATION_TACTICAL_AI_H

#include "../../common.h"
#include "../../types.h"
#include "base_ai.h"

/* Tactical action */
typedef struct {
    char action_type[STRING_SHORT_LEN];
    char target[STRING_SHORT_LEN];
    civ_float_t urgency;  /* 0.0 to 1.0 */
    civ_float_t cost;
    civ_float_t expected_benefit;
    time_t timestamp;
} civ_tactical_action_t;

/* Tactical AI */
typedef struct {
    civ_base_ai_t* base_ai;
    
    civ_tactical_action_t* actions;
    size_t action_count;
    size_t action_capacity;
    
    civ_float_t reaction_time;  /* Seconds to react */
    civ_float_t decision_threshold; /* Minimum urgency to act */
} civ_tactical_ai_t;

/* Function declarations */
civ_tactical_ai_t* civ_tactical_ai_create(const char* id, const char* name);
void civ_tactical_ai_destroy(civ_tactical_ai_t* ai);
void civ_tactical_ai_init(civ_tactical_ai_t* ai, const char* id, const char* name);

civ_result_t civ_tactical_ai_react(civ_tactical_ai_t* ai, civ_float_t time_delta);
civ_result_t civ_tactical_ai_add_action(civ_tactical_ai_t* ai, const char* action_type, const char* target, civ_float_t urgency);
civ_tactical_action_t* civ_tactical_ai_get_best_action(const civ_tactical_ai_t* ai);
civ_float_t civ_tactical_ai_calculate_utility(const civ_tactical_action_t* action);

#endif /* CIVILIZATION_TACTICAL_AI_H */

