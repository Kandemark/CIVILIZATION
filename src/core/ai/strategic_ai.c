/**
 * @file strategic_ai.c
 * @brief Implementation of strategic AI
 */

#include "../../../include/core/ai/strategic_ai.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_strategic_ai_t* civ_strategic_ai_create(const char* id, const char* name) {
    civ_strategic_ai_t* ai = (civ_strategic_ai_t*)CIV_MALLOC(sizeof(civ_strategic_ai_t));
    if (!ai) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate strategic AI");
        return NULL;
    }
    
    civ_strategic_ai_init(ai, id, name);
    return ai;
}

void civ_strategic_ai_destroy(civ_strategic_ai_t* ai) {
    if (!ai) return;
    
    if (ai->base_ai) {
        civ_base_ai_destroy(ai->base_ai);
    }
    CIV_FREE(ai->goals);
    CIV_FREE(ai);
}

void civ_strategic_ai_init(civ_strategic_ai_t* ai, const char* id, const char* name) {
    if (!ai || !id || !name) return;
    
    memset(ai, 0, sizeof(civ_strategic_ai_t));
    ai->base_ai = civ_base_ai_create(id, name);
    ai->planning_horizon = 10.0f;  /* 10 years */
    ai->risk_tolerance = 0.5f;
    ai->goal_capacity = 16;
    ai->goals = (civ_strategic_goal_t*)CIV_CALLOC(ai->goal_capacity, sizeof(civ_strategic_goal_t));
}

civ_result_t civ_strategic_ai_plan(civ_strategic_ai_t* ai, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update base AI thinking */
    if (ai->base_ai) {
        civ_base_ai_think(ai->base_ai, time_delta);
    }
    
    /* Update goal progress */
    time_t now = time(NULL);
    for (size_t i = 0; i < ai->goal_count; i++) {
        civ_strategic_goal_t* goal = &ai->goals[i];
        
        /* Remove completed or expired goals */
        if (goal->progress >= 1.0f || (goal->deadline > 0 && now > goal->deadline)) {
            /* Remove goal */
            memmove(&ai->goals[i], &ai->goals[i + 1], (ai->goal_count - i - 1) * sizeof(civ_strategic_goal_t));
            ai->goal_count--;
            i--;
        }
    }
    
    return result;
}

civ_result_t civ_strategic_ai_add_goal(civ_strategic_ai_t* ai, const char* goal_type, const char* description, civ_float_t priority) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai || !goal_type) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    priority = CLAMP(priority, 0.0f, 1.0f);
    
    /* Expand if needed */
    if (ai->goal_count >= ai->goal_capacity) {
        ai->goal_capacity *= 2;
        ai->goals = (civ_strategic_goal_t*)CIV_REALLOC(ai->goals,
                                                      ai->goal_capacity * sizeof(civ_strategic_goal_t));
    }
    
    if (ai->goals) {
        civ_strategic_goal_t* goal = &ai->goals[ai->goal_count++];
        strncpy(goal->goal_type, goal_type, sizeof(goal->goal_type) - 1);
        if (description) {
            strncpy(goal->description, description, sizeof(goal->description) - 1);
        }
        goal->priority = priority;
        goal->progress = 0.0f;
        goal->deadline = 0;  /* No deadline by default */
        goal->created = time(NULL);
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_strategic_goal_t* civ_strategic_ai_get_priority_goal(const civ_strategic_ai_t* ai) {
    if (!ai || ai->goal_count == 0) return NULL;
    
    civ_strategic_goal_t* best = &ai->goals[0];
    for (size_t i = 1; i < ai->goal_count; i++) {
        if (ai->goals[i].priority > best->priority) {
            best = &ai->goals[i];
        }
    }
    
    return best;
}

civ_result_t civ_strategic_ai_update_goal(civ_strategic_ai_t* ai, size_t goal_index, civ_float_t progress) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (goal_index >= ai->goal_count) {
        result.error = CIV_ERROR_INVALID_ARGUMENT;
        return result;
    }
    
    ai->goals[goal_index].progress = CLAMP(progress, 0.0f, 1.0f);
    return result;
}

