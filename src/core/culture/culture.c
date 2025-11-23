/**
 * @file culture.c
 * @brief Implementation of main culture system
 */

#include "../../../include/core/culture/culture.h"
#include "../../../include/common.h"

civ_culture_system_t* civ_culture_system_create(void) {
    civ_culture_system_t* culture = (civ_culture_system_t*)CIV_MALLOC(sizeof(civ_culture_system_t));
    if (!culture) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate culture system");
        return NULL;
    }
    
    civ_culture_system_init(culture);
    return culture;
}

void civ_culture_system_destroy(civ_culture_system_t* culture) {
    if (!culture) return;
    
    if (culture->identity_manager) {
        civ_cultural_identity_manager_destroy(culture->identity_manager);
    }
    if (culture->diffusion) {
        civ_cultural_diffusion_destroy(culture->diffusion);
    }
    if (culture->assimilation_tracker) {
        civ_assimilation_tracker_destroy(culture->assimilation_tracker);
    }
    if (culture->language_evolution) {
        civ_language_evolution_destroy(culture->language_evolution);
    }
    if (culture->writing_system_manager) {
        civ_writing_system_manager_destroy(culture->writing_system_manager);
    }
    CIV_FREE(culture);
}

void civ_culture_system_init(civ_culture_system_t* culture) {
    if (!culture) return;
    
    memset(culture, 0, sizeof(civ_culture_system_t));
    culture->identity_manager = civ_cultural_identity_manager_create();
    culture->diffusion = civ_cultural_diffusion_create();
    culture->assimilation_tracker = civ_assimilation_tracker_create();
    culture->language_evolution = civ_language_evolution_create();
    culture->writing_system_manager = civ_writing_system_manager_create();
}

civ_result_t civ_culture_system_update(civ_culture_system_t* culture, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!culture) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update identity manager */
    if (culture->identity_manager) {
        for (size_t i = 0; i < culture->identity_manager->identity_count; i++) {
            civ_cultural_identity_update(&culture->identity_manager->identities[i], time_delta);
        }
    }
    
    /* Process cultural diffusion */
    if (culture->diffusion && culture->identity_manager) {
        civ_cultural_diffusion_process(culture->diffusion, culture->identity_manager, time_delta);
    }
    
    /* Update assimilation tracker */
    if (culture->assimilation_tracker && culture->identity_manager) {
        civ_assimilation_tracker_update(culture->assimilation_tracker, culture->identity_manager, time_delta);
    }
    
    /* Update language evolution */
    if (culture->language_evolution) {
        civ_language_evolution_update(culture->language_evolution, time_delta);
    }
    
    return result;
}

