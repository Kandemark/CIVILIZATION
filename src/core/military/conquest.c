/**
 * @file conquest.c
 * @brief Implementation of conquest mechanics
 */

#include "../../../include/core/military/conquest.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* RNG for plunder */
static uint32_t plunder_rng_state = 0;
static uint32_t plunder_rng_next(void) {
    plunder_rng_state = plunder_rng_state * 1103515245 + 12345;
    return (plunder_rng_state / 65536) % 32768;
}
static civ_float_t plunder_rng_float(void) {
    return (civ_float_t)plunder_rng_next() / 32768.0f;
}

civ_conquest_system_t* civ_conquest_system_create(void) {
    civ_conquest_system_t* system = (civ_conquest_system_t*)CIV_MALLOC(sizeof(civ_conquest_system_t));
    if (!system) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate conquest system");
        return NULL;
    }
    
    civ_conquest_system_init(system);
    return system;
}

void civ_conquest_system_destroy(civ_conquest_system_t* system) {
    if (!system) return;
    CIV_FREE(system->conquests);
    CIV_FREE(system);
}

void civ_conquest_system_init(civ_conquest_system_t* system) {
    if (!system) return;
    
    memset(system, 0, sizeof(civ_conquest_system_t));
    system->base_conquest_rate = 0.05f;
    system->plunder_multiplier = 1.0f;
    system->conquest_capacity = 32;
    system->conquests = (civ_conquest_event_t*)CIV_CALLOC(system->conquest_capacity, sizeof(civ_conquest_event_t));
}

civ_result_t civ_conquest_start(civ_conquest_system_t* system,
                                const char* attacker_id, const char* defender_id,
                                const char* target_region_id, civ_conquest_type_t type) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!system || !attacker_id || !defender_id || !target_region_id) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Check if conquest already exists */
    for (size_t i = 0; i < system->conquest_count; i++) {
        if (strcmp(system->conquests[i].attacker_id, attacker_id) == 0 &&
            strcmp(system->conquests[i].defender_id, defender_id) == 0 &&
            strcmp(system->conquests[i].target_region_id, target_region_id) == 0) {
            result.error = CIV_ERROR_INVALID_STATE;
            result.message = "Conquest already in progress";
            return result;
        }
    }
    
    /* Expand if needed */
    if (system->conquest_count >= system->conquest_capacity) {
        system->conquest_capacity *= 2;
        system->conquests = (civ_conquest_event_t*)CIV_REALLOC(system->conquests,
                                                              system->conquest_capacity * sizeof(civ_conquest_event_t));
    }
    
    if (system->conquests) {
        civ_conquest_event_t* conquest = &system->conquests[system->conquest_count++];
        memset(conquest, 0, sizeof(civ_conquest_event_t));
        strncpy(conquest->attacker_id, attacker_id, sizeof(conquest->attacker_id) - 1);
        strncpy(conquest->defender_id, defender_id, sizeof(conquest->defender_id) - 1);
        strncpy(conquest->target_region_id, target_region_id, sizeof(conquest->target_region_id) - 1);
        conquest->type = type;
        conquest->progress = 0.0f;
        conquest->attacker_strength = 1.0f;  /* Would be calculated from military */
        conquest->defender_strength = 1.0f;
        conquest->assimilation_enabled = true;
        conquest->assimilation_type = (type == CIV_CONQUEST_INVASION || type == CIV_CONQUEST_ANNEXATION) ?
                                      CIV_ASSIMILATION_FORCED : CIV_ASSIMILATION_VOLUNTARY;
        conquest->start_time = time(NULL);
        conquest->last_update = conquest->start_time;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_conquest_update(civ_conquest_system_t* system, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!system) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    time_t now = time(NULL);
    
    for (size_t i = 0; i < system->conquest_count; i++) {
        civ_conquest_event_t* conquest = &system->conquests[i];
        
        /* Calculate conquest progress based on strength ratio */
        civ_float_t strength_ratio = conquest->attacker_strength / 
                                     MAX(conquest->defender_strength, 0.1f);
        civ_float_t progress_rate = system->base_conquest_rate * strength_ratio * time_delta;
        
        conquest->progress = CLAMP(conquest->progress + progress_rate, 0.0f, 1.0f);
        conquest->last_update = now;
        
        /* If conquest is complete, apply plunder and assimilation */
        if (conquest->progress >= 1.0f) {
            civ_plunder_result_t plunder_result = {0};
            civ_conquest_plunder(conquest, &plunder_result);
            conquest->plunder = plunder_result;
            
            /* Remove completed conquest */
            memmove(&system->conquests[i], &system->conquests[i + 1],
                   (system->conquest_count - i - 1) * sizeof(civ_conquest_event_t));
            system->conquest_count--;
            i--;
        }
    }
    
    return result;
}

civ_result_t civ_conquest_plunder(civ_conquest_event_t* conquest, civ_plunder_result_t* result) {
    civ_result_t result_code = {CIV_OK, NULL};
    
    if (!conquest || !result) {
        result_code.error = CIV_ERROR_NULL_POINTER;
        return result_code;
    }
    
    plunder_rng_state = (uint32_t)time(NULL);
    
    /* Calculate plunder based on conquest type and progress */
    civ_float_t plunder_factor = conquest->progress;
    
    switch (conquest->type) {
        case CIV_CONQUEST_RAID:
            plunder_factor *= 1.5f;  /* Raids focus on plunder */
            break;
        case CIV_CONQUEST_INVASION:
            plunder_factor *= 1.2f;
            break;
        case CIV_CONQUEST_SIEGE:
            plunder_factor *= 0.8f;  /* Sieges are slower */
            break;
        case CIV_CONQUEST_ANNEXATION:
            plunder_factor *= 0.5f;  /* Annexations preserve more */
            break;
    }
    
    result->gold = 1000.0f * plunder_factor * plunder_rng_float();
    result->resources = 500.0f * plunder_factor * plunder_rng_float();
    result->artifacts = 10.0f * plunder_factor * plunder_rng_float();
    result->population_captured = (int32_t)(1000 * plunder_factor * plunder_rng_float());
    result->knowledge_gained = 0.1f * plunder_factor;
    
    return result_code;
}

civ_result_t civ_conquest_apply_assimilation(civ_conquest_event_t* conquest,
                                             civ_assimilation_tracker_t* assimilation_tracker) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!conquest || !assimilation_tracker) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (!conquest->assimilation_enabled) {
        return result;
    }
    
    /* Add assimilation event */
    civ_assimilation_tracker_add_event(assimilation_tracker,
                                      conquest->attacker_id,
                                      conquest->defender_id,
                                      conquest->target_region_id,
                                      conquest->assimilation_type);
    
    return result;
}

civ_conquest_event_t* civ_conquest_find(const civ_conquest_system_t* system,
                                       const char* attacker_id, const char* defender_id) {
    if (!system || !attacker_id || !defender_id) return NULL;
    
    for (size_t i = 0; i < system->conquest_count; i++) {
        if (strcmp(system->conquests[i].attacker_id, attacker_id) == 0 &&
            strcmp(system->conquests[i].defender_id, defender_id) == 0) {
            return (civ_conquest_event_t*)&system->conquests[i];
        }
    }
    
    return NULL;
}

