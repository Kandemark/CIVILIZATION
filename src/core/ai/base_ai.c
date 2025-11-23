/**
 * @file base_ai.c
 * @brief Implementation of base AI system
 */

#include "../../../include/core/ai/base_ai.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_base_ai_t* civ_base_ai_create(const char* id, const char* name) {
    if (!id || !name) return NULL;
    
    civ_base_ai_t* ai = (civ_base_ai_t*)CIV_MALLOC(sizeof(civ_base_ai_t));
    if (!ai) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate base AI");
        return NULL;
    }
    
    civ_base_ai_init(ai, id, name);
    return ai;
}

void civ_base_ai_destroy(civ_base_ai_t* ai) {
    if (!ai) return;
    CIV_FREE(ai->decisions);
    CIV_FREE(ai);
}

void civ_base_ai_init(civ_base_ai_t* ai, const char* id, const char* name) {
    if (!ai || !id || !name) return;
    
    memset(ai, 0, sizeof(civ_base_ai_t));
    strncpy(ai->id, id, sizeof(ai->id) - 1);
    strncpy(ai->name, name, sizeof(ai->name) - 1);
    
    /* Default personality */
    ai->personality.aggressiveness = 0.5f;
    ai->personality.expansionism = 0.5f;
    ai->personality.diplomacy = 0.5f;
    ai->personality.research_focus = 0.5f;
    ai->personality.economic_focus = 0.5f;
    
    ai->intelligence = 0.5f;
    ai->adaptability = 0.5f;
    ai->decision_capacity = 32;
    ai->decisions = (civ_ai_decision_t*)CIV_CALLOC(ai->decision_capacity, sizeof(civ_ai_decision_t));
    ai->creation_time = time(NULL);
    ai->last_think = ai->creation_time;
}

civ_result_t civ_base_ai_think(civ_base_ai_t* ai, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Clear old decisions */
    ai->decision_count = 0;
    
    /* AI thinking process (simplified) */
    /* In a full implementation, this would evaluate various factors and generate decisions */
    
    ai->last_think = time(NULL);
    return result;
}

civ_result_t civ_base_ai_make_decision(civ_base_ai_t* ai, const char* action_type, const char* target, civ_float_t priority) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai || !action_type) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    priority = CLAMP(priority, 0.0f, 1.0f);
    
    /* Expand if needed */
    if (ai->decision_count >= ai->decision_capacity) {
        ai->decision_capacity *= 2;
        ai->decisions = (civ_ai_decision_t*)CIV_REALLOC(ai->decisions,
                                                       ai->decision_capacity * sizeof(civ_ai_decision_t));
    }
    
    if (ai->decisions) {
        civ_ai_decision_t* decision = &ai->decisions[ai->decision_count++];
        strncpy(decision->action_type, action_type, sizeof(decision->action_type) - 1);
        if (target) {
            strncpy(decision->target, target, sizeof(decision->target) - 1);
        }
        decision->priority = priority;
        decision->confidence = ai->intelligence;
        decision->timestamp = time(NULL);
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_ai_decision_t* civ_base_ai_get_best_decision(const civ_base_ai_t* ai) {
    if (!ai || ai->decision_count == 0) return NULL;
    
    civ_ai_decision_t* best = &ai->decisions[0];
    for (size_t i = 1; i < ai->decision_count; i++) {
        if (ai->decisions[i].priority > best->priority) {
            best = &ai->decisions[i];
        }
    }
    
    return best;
}

void civ_base_ai_set_personality(civ_base_ai_t* ai, const civ_ai_personality_t* personality) {
    if (!ai || !personality) return;
    ai->personality = *personality;
}

