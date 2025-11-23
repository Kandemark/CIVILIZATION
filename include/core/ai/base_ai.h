/**
 * @file base_ai.h
 * @brief Base AI system
 */

#ifndef CIVILIZATION_BASE_AI_H
#define CIVILIZATION_BASE_AI_H

#include "../../common.h"
#include "../../types.h"

/* AI personality traits */
typedef struct {
    civ_float_t aggressiveness;  /* 0.0 to 1.0 */
    civ_float_t expansionism;    /* 0.0 to 1.0 */
    civ_float_t diplomacy;       /* 0.0 to 1.0 */
    civ_float_t research_focus;  /* 0.0 to 1.0 */
    civ_float_t economic_focus;  /* 0.0 to 1.0 */
} civ_ai_personality_t;

/* AI decision */
typedef struct {
    char action_type[STRING_SHORT_LEN];
    char target[STRING_SHORT_LEN];
    civ_float_t priority;  /* 0.0 to 1.0 */
    civ_float_t confidence; /* 0.0 to 1.0 */
    time_t timestamp;
} civ_ai_decision_t;

/* Base AI entity */
typedef struct {
    char id[STRING_SHORT_LEN];
    char name[STRING_MEDIUM_LEN];
    
    civ_ai_personality_t personality;
    
    civ_ai_decision_t* decisions;
    size_t decision_count;
    size_t decision_capacity;
    
    civ_float_t intelligence;  /* 0.0 to 1.0 */
    civ_float_t adaptability; /* 0.0 to 1.0 */
    
    time_t last_think;
    time_t creation_time;
} civ_base_ai_t;

/* Function declarations */
civ_base_ai_t* civ_base_ai_create(const char* id, const char* name);
void civ_base_ai_destroy(civ_base_ai_t* ai);
void civ_base_ai_init(civ_base_ai_t* ai, const char* id, const char* name);

civ_result_t civ_base_ai_think(civ_base_ai_t* ai, civ_float_t time_delta);
civ_result_t civ_base_ai_make_decision(civ_base_ai_t* ai, const char* action_type, const char* target, civ_float_t priority);
civ_ai_decision_t* civ_base_ai_get_best_decision(const civ_base_ai_t* ai);
void civ_base_ai_set_personality(civ_base_ai_t* ai, const civ_ai_personality_t* personality);

#endif /* CIVILIZATION_BASE_AI_H */

