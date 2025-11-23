/**
 * @file cultural_display.c
 * @brief Implementation of cultural visualization
 */

#include "../../../include/core/visualization/cultural_display.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

civ_cultural_display_t* civ_cultural_display_create(void) {
    civ_cultural_display_t* display = (civ_cultural_display_t*)CIV_MALLOC(sizeof(civ_cultural_display_t));
    if (!display) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate cultural display");
        return NULL;
    }
    
    civ_cultural_display_init(display);
    return display;
}

void civ_cultural_display_destroy(civ_cultural_display_t* display) {
    if (!display) return;
    CIV_FREE(display->assimilation_displays);
    CIV_FREE(display->culture_displays);
    CIV_FREE(display);
}

void civ_cultural_display_init(civ_cultural_display_t* display) {
    if (!display) return;
    
    memset(display, 0, sizeof(civ_cultural_display_t));
    display->assimilation_display_capacity = 50;
    display->culture_display_capacity = 50;
    display->assimilation_displays = (civ_assimilation_display_t*)CIV_CALLOC(display->assimilation_display_capacity, sizeof(civ_assimilation_display_t));
    display->culture_displays = (civ_culture_display_t*)CIV_CALLOC(display->culture_display_capacity, sizeof(civ_culture_display_t));
}

civ_result_t civ_cultural_display_update(civ_cultural_display_t* display,
                                        civ_assimilation_tracker_t* assimilation_tracker,
                                        civ_cultural_identity_manager_t* identity_manager) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!display) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update assimilation displays */
    display->assimilation_display_count = 0;
    if (assimilation_tracker) {
        for (size_t i = 0; i < assimilation_tracker->event_count && 
             display->assimilation_display_count < display->assimilation_display_capacity; i++) {
            civ_assimilation_event_t* event = &assimilation_tracker->events[i];
            civ_assimilation_display_t* disp = &display->assimilation_displays[display->assimilation_display_count++];
            
            strncpy(disp->source_culture, event->source_culture_id, sizeof(disp->source_culture) - 1);
            strncpy(disp->target_culture, event->target_culture_id, sizeof(disp->target_culture) - 1);
            strncpy(disp->region, event->region_id, sizeof(disp->region) - 1);
            disp->type = event->type;
            disp->progress = event->progress;
            disp->adoption_level = event->adoption_level;
            disp->population_affected = event->population_affected;
            
            /* Determine status */
            if (event->progress < 0.1f) {
                strcpy(disp->status, "spreading");
            } else if (event->progress < 0.5f) {
                strcpy(disp->status, event->type == CIV_ASSIMILATION_FORCED ? "imposing" : "adopting");
            } else if (event->progress < 1.0f) {
                strcpy(disp->status, "integrating");
            } else {
                strcpy(disp->status, "complete");
            }
        }
    }
    
    /* Update culture displays */
    display->culture_display_count = 0;
    if (identity_manager) {
        for (size_t i = 0; i < identity_manager->identity_count &&
             display->culture_display_count < display->culture_display_capacity; i++) {
            civ_cultural_identity_t* identity = &identity_manager->identities[i];
            civ_culture_display_t* disp = &display->culture_displays[display->culture_display_count++];
            
            strncpy(disp->culture_id, identity->id, sizeof(disp->culture_id) - 1);
            strncpy(disp->culture_name, identity->name, sizeof(disp->culture_name) - 1);
            disp->influence = identity->influence_radius;
            disp->cohesion = identity->cohesion;
            disp->trait_count = identity->trait_count;
            disp->regions_influenced = 0; /* Would be calculated from map */
        }
    }
    
    return result;
}

civ_result_t civ_cultural_display_get_assimilation_info(const civ_cultural_display_t* display,
                                                        const char* source_id, const char* target_id,
                                                        civ_assimilation_display_t* out) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!display || !source_id || !target_id || !out) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    for (size_t i = 0; i < display->assimilation_display_count; i++) {
        if (strcmp(display->assimilation_displays[i].source_culture, source_id) == 0 &&
            strcmp(display->assimilation_displays[i].target_culture, target_id) == 0) {
            *out = display->assimilation_displays[i];
            return result;
        }
    }
    
    result.error = CIV_ERROR_NOT_FOUND;
    return result;
}

civ_result_t civ_cultural_display_get_culture_info(const civ_cultural_display_t* display,
                                                  const char* culture_id,
                                                  civ_culture_display_t* out) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!display || !culture_id || !out) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    for (size_t i = 0; i < display->culture_display_count; i++) {
        if (strcmp(display->culture_displays[i].culture_id, culture_id) == 0) {
            *out = display->culture_displays[i];
            return result;
        }
    }
    
    result.error = CIV_ERROR_NOT_FOUND;
    return result;
}

char* civ_cultural_display_format_assimilation_report(const civ_cultural_display_t* display) {
    if (!display) return NULL;
    
    char* report = (char*)CIV_MALLOC(4096);
    if (!report) return NULL;
    
    size_t offset = 0;
    offset += snprintf(report + offset, 4096 - offset, "Cultural Assimilation Report\n");
    offset += snprintf(report + offset, 4096 - offset, "============================\n\n");
    
    for (size_t i = 0; i < display->assimilation_display_count && offset < 4000; i++) {
        const civ_assimilation_display_t* disp = &display->assimilation_displays[i];
        offset += snprintf(report + offset, 4096 - offset,
                          "%s -> %s (%s)\n",
                          disp->source_culture, disp->target_culture, disp->region);
        offset += snprintf(report + offset, 4096 - offset,
                          "  Type: %s | Progress: %.1f%% | Adoption: %.1f%%\n",
                          disp->type == CIV_ASSIMILATION_FORCED ? "Forced" : "Voluntary",
                          disp->progress * 100.0f, disp->adoption_level * 100.0f);
        offset += snprintf(report + offset, 4096 - offset,
                          "  Status: %s | Population: %d\n\n",
                          disp->status, disp->population_affected);
    }
    
    return report;
}

