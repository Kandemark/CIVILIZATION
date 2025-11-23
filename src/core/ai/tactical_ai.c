/**
 * @file tactical_ai.c
 * @brief Implementation of tactical AI
 */

#include "../../../include/core/ai/tactical_ai.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

civ_tactical_ai_t* civ_tactical_ai_create(const char* id, const char* name) {
    civ_tactical_ai_t* ai = (civ_tactical_ai_t*)CIV_MALLOC(sizeof(civ_tactical_ai_t));
    if (!ai) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate tactical AI");
        return NULL;
    }
    
    civ_tactical_ai_init(ai, id, name);
    return ai;
}

void civ_tactical_ai_destroy(civ_tactical_ai_t* ai) {
    if (!ai) return;
    
    if (ai->base_ai) {
        civ_base_ai_destroy(ai->base_ai);
    }
    CIV_FREE(ai->actions);
    CIV_FREE(ai);
}

void civ_tactical_ai_init(civ_tactical_ai_t* ai, const char* id, const char* name) {
    if (!ai || !id || !name) return;
    
    memset(ai, 0, sizeof(civ_tactical_ai_t));
    ai->base_ai = civ_base_ai_create(id, name);
    ai->reaction_time = 1.0f;  /* 1 second */
    ai->decision_threshold = 0.3f;
    ai->action_capacity = 32;
    ai->actions = (civ_tactical_action_t*)CIV_CALLOC(ai->action_capacity, sizeof(civ_tactical_action_t));
}

civ_result_t civ_tactical_ai_react(civ_tactical_ai_t* ai, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update base AI */
    if (ai->base_ai) {
        civ_base_ai_think(ai->base_ai, time_delta);
    }
    
    /* Remove old actions */
    time_t now = time(NULL);
    for (size_t i = 0; i < ai->action_count; i++) {
        /* Remove actions older than reaction time */
        if (now - ai->actions[i].timestamp > (time_t)ai->reaction_time) {
            memmove(&ai->actions[i], &ai->actions[i + 1], (ai->action_count - i - 1) * sizeof(civ_tactical_action_t));
            ai->action_count--;
            i--;
        }
    }
    
    return result;
}

civ_result_t civ_tactical_ai_add_action(civ_tactical_ai_t* ai, const char* action_type, const char* target, civ_float_t urgency) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai || !action_type) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    urgency = CLAMP(urgency, 0.0f, 1.0f);
    
    /* Expand if needed */
    if (ai->action_count >= ai->action_capacity) {
        ai->action_capacity *= 2;
        ai->actions = (civ_tactical_action_t*)CIV_REALLOC(ai->actions,
                                                         ai->action_capacity * sizeof(civ_tactical_action_t));
    }
    
    if (ai->actions) {
        civ_tactical_action_t* action = &ai->actions[ai->action_count++];
        strncpy(action->action_type, action_type, sizeof(action->action_type) - 1);
        if (target) {
            strncpy(action->target, target, sizeof(action->target) - 1);
        }
        action->urgency = urgency;
        action->cost = 0.0f;
        action->expected_benefit = urgency;
        action->timestamp = time(NULL);
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_tactical_action_t* civ_tactical_ai_get_best_action(const civ_tactical_ai_t* ai) {
    if (!ai || ai->action_count == 0) return NULL;
    
    civ_tactical_action_t* best = NULL;
    civ_float_t best_utility = -1.0f;
    
    for (size_t i = 0; i < ai->action_count; i++) {
        civ_float_t utility = civ_tactical_ai_calculate_utility(&ai->actions[i]);
        if (utility > best_utility && ai->actions[i].urgency >= ai->decision_threshold) {
            best_utility = utility;
            best = &ai->actions[i];
        }
    }
    
    return best;
}

civ_float_t civ_tactical_ai_calculate_utility(const civ_tactical_action_t* action) {
    if (!action) return 0.0f;
    
    /* Utility = expected_benefit - cost, weighted by urgency */
    return (action->expected_benefit - action->cost) * action->urgency;
}

