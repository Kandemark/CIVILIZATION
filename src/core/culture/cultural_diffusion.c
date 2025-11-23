/**
 * @file cultural_diffusion.c
 * @brief Implementation of cultural diffusion system
 */

#include "../../../include/core/culture/cultural_diffusion.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

civ_cultural_diffusion_t* civ_cultural_diffusion_create(void) {
    civ_cultural_diffusion_t* diffusion = (civ_cultural_diffusion_t*)CIV_MALLOC(sizeof(civ_cultural_diffusion_t));
    if (!diffusion) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate cultural diffusion");
        return NULL;
    }
    
    civ_cultural_diffusion_init(diffusion);
    return diffusion;
}

void civ_cultural_diffusion_destroy(civ_cultural_diffusion_t* diffusion) {
    if (!diffusion) return;
    CIV_FREE(diffusion->events);
    CIV_FREE(diffusion);
}

void civ_cultural_diffusion_init(civ_cultural_diffusion_t* diffusion) {
    if (!diffusion) return;
    
    memset(diffusion, 0, sizeof(civ_cultural_diffusion_t));
    diffusion->base_diffusion_rate = 0.01f;
    diffusion->distance_decay = 0.1f;
    diffusion->resistance_factor = 0.5f;
    diffusion->event_capacity = 100;
    diffusion->events = (civ_cultural_diffusion_event_t*)CIV_CALLOC(diffusion->event_capacity, sizeof(civ_cultural_diffusion_event_t));
}

civ_result_t civ_cultural_diffusion_process(civ_cultural_diffusion_t* diffusion,
                                            civ_cultural_identity_manager_t* manager,
                                            civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!diffusion || !manager) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Process diffusion between all identity pairs */
    for (size_t i = 0; i < manager->identity_count; i++) {
        for (size_t j = i + 1; j < manager->identity_count; j++) {
            civ_cultural_identity_t* source = &manager->identities[i];
            civ_cultural_identity_t* target = &manager->identities[j];
            
            /* Calculate distance (simplified - would use actual geographic distance) */
            civ_float_t distance = fabsf(source->influence_radius - target->influence_radius);
            
            /* Diffuse traits from source to target */
            for (size_t k = 0; k < source->trait_count; k++) {
                const char* trait_name = source->traits[k].name;
                civ_float_t source_strength = source->traits[k].strength;
                
                /* Find or create trait in target */
                bool found = false;
                for (size_t l = 0; l < target->trait_count; l++) {
                    if (strcmp(target->traits[l].name, trait_name) == 0) {
                        found = true;
                        /* Apply diffusion */
                        civ_float_t rate = civ_cultural_diffusion_calculate_rate(diffusion, source_strength,
                                                                               1.0f - target->cohesion, distance);
                        target->traits[l].strength = CLAMP(target->traits[l].strength + rate * time_delta, 0.0f, 1.0f);
                        break;
                    }
                }
                
                if (!found && source_strength > 0.3f) {
                    /* Create new trait in target if strong enough */
                    civ_cultural_identity_add_trait(target, trait_name, source_strength * 0.1f);
                }
            }
        }
    }
    
    return result;
}

civ_result_t civ_cultural_diffusion_diffuse_trait(civ_cultural_diffusion_t* diffusion,
                                                  const civ_cultural_identity_t* source,
                                                  civ_cultural_identity_t* target,
                                                  const char* trait_name,
                                                  civ_float_t distance) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!diffusion || !source || !target || !trait_name) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Find trait in source */
    civ_float_t source_strength = 0.0f;
    for (size_t i = 0; i < source->trait_count; i++) {
        if (strcmp(source->traits[i].name, trait_name) == 0) {
            source_strength = source->traits[i].strength;
            break;
        }
    }
    
    if (source_strength <= 0.0f) {
        result.error = CIV_ERROR_NOT_FOUND;
        return result;
    }
    
    /* Calculate diffusion rate */
    civ_float_t rate = civ_cultural_diffusion_calculate_rate(diffusion, source_strength,
                                                            1.0f - target->cohesion, distance);
    
    /* Find or create trait in target */
    bool found = false;
    for (size_t i = 0; i < target->trait_count; i++) {
        if (strcmp(target->traits[i].name, trait_name) == 0) {
            found = true;
            target->traits[i].strength = CLAMP(target->traits[i].strength + rate, 0.0f, 1.0f);
            break;
        }
    }
    
    if (!found) {
        civ_cultural_identity_add_trait(target, trait_name, rate);
    }
    
    /* Record event */
    if (diffusion->event_count < diffusion->event_capacity) {
        civ_cultural_diffusion_event_t* event = &diffusion->events[diffusion->event_count++];
        strncpy(event->source_id, source->id, sizeof(event->source_id) - 1);
        strncpy(event->target_id, target->id, sizeof(event->target_id) - 1);
        strncpy(event->trait_name, trait_name, sizeof(event->trait_name) - 1);
        event->intensity = rate;
        event->distance = distance;
        event->timestamp = time(NULL);
    }
    
    return result;
}

civ_float_t civ_cultural_diffusion_calculate_rate(civ_cultural_diffusion_t* diffusion,
                                                   civ_float_t source_strength,
                                                   civ_float_t target_resistance,
                                                   civ_float_t distance) {
    if (!diffusion) return 0.0f;
    
    /* Calculate diffusion rate: base_rate * strength * (1 - resistance) * distance_decay */
    civ_float_t distance_factor = expf(-diffusion->distance_decay * distance);
    civ_float_t resistance_factor = 1.0f - (target_resistance * diffusion->resistance_factor);
    
    return diffusion->base_diffusion_rate * source_strength * resistance_factor * distance_factor;
}

