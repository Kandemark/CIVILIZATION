/**
 * @file ai_system.c
 * @brief Implementation of main AI system
 */

#include "../../../include/core/ai/ai_system.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_ai_system_t* civ_ai_system_create(void) {
    civ_ai_system_t* ai_system = (civ_ai_system_t*)CIV_MALLOC(sizeof(civ_ai_system_t));
    if (!ai_system) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate AI system");
        return NULL;
    }
    
    civ_ai_system_init(ai_system);
    return ai_system;
}

void civ_ai_system_destroy(civ_ai_system_t* ai_system) {
    if (!ai_system) return;
    
    for (size_t i = 0; i < ai_system->strategic_count; i++) {
        civ_strategic_ai_destroy(ai_system->strategic_ais[i]);
    }
    for (size_t i = 0; i < ai_system->tactical_count; i++) {
        civ_tactical_ai_destroy(ai_system->tactical_ais[i]);
    }
    CIV_FREE(ai_system->strategic_ais);
    CIV_FREE(ai_system->tactical_ais);
    CIV_FREE(ai_system);
}

void civ_ai_system_init(civ_ai_system_t* ai_system) {
    if (!ai_system) return;
    
    memset(ai_system, 0, sizeof(civ_ai_system_t));
    ai_system->strategic_capacity = 16;
    ai_system->tactical_capacity = 16;
    ai_system->strategic_ais = (civ_strategic_ai_t**)CIV_CALLOC(ai_system->strategic_capacity, sizeof(civ_strategic_ai_t*));
    ai_system->tactical_ais = (civ_tactical_ai_t**)CIV_CALLOC(ai_system->tactical_capacity, sizeof(civ_tactical_ai_t*));
}

civ_result_t civ_ai_system_update(civ_ai_system_t* ai_system, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai_system) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update strategic AIs */
    for (size_t i = 0; i < ai_system->strategic_count; i++) {
        civ_strategic_ai_plan(ai_system->strategic_ais[i], time_delta);
    }
    
    /* Update tactical AIs */
    for (size_t i = 0; i < ai_system->tactical_count; i++) {
        civ_tactical_ai_react(ai_system->tactical_ais[i], time_delta);
    }
    
    return result;
}

civ_result_t civ_ai_system_add_strategic(civ_ai_system_t* ai_system, civ_strategic_ai_t* ai) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai_system || !ai) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (ai_system->strategic_count >= ai_system->strategic_capacity) {
        ai_system->strategic_capacity *= 2;
        ai_system->strategic_ais = (civ_strategic_ai_t**)CIV_REALLOC(ai_system->strategic_ais,
                                                                     ai_system->strategic_capacity * sizeof(civ_strategic_ai_t*));
    }
    
    if (ai_system->strategic_ais) {
        ai_system->strategic_ais[ai_system->strategic_count++] = ai;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_ai_system_add_tactical(civ_ai_system_t* ai_system, civ_tactical_ai_t* ai) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ai_system || !ai) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (ai_system->tactical_count >= ai_system->tactical_capacity) {
        ai_system->tactical_capacity *= 2;
        ai_system->tactical_ais = (civ_tactical_ai_t**)CIV_REALLOC(ai_system->tactical_ais,
                                                                    ai_system->tactical_capacity * sizeof(civ_tactical_ai_t*));
    }
    
    if (ai_system->tactical_ais) {
        ai_system->tactical_ais[ai_system->tactical_count++] = ai;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

