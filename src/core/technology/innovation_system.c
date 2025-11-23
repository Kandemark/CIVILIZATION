/**
 * @file innovation_system.c
 * @brief Implementation of innovation system
 */

#include "../../../include/core/technology/innovation_system.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

civ_innovation_system_t* civ_innovation_system_create(void) {
    civ_innovation_system_t* is = (civ_innovation_system_t*)CIV_MALLOC(sizeof(civ_innovation_system_t));
    if (!is) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate innovation system");
        return NULL;
    }
    
    civ_innovation_system_init(is);
    return is;
}

void civ_innovation_system_destroy(civ_innovation_system_t* is) {
    if (!is) return;
    
    if (is->technologies) {
        for (size_t i = 0; i < is->tech_count; i++) {
            CIV_FREE(is->technologies[i].prerequisites);
        }
        CIV_FREE(is->technologies);
    }
    
    CIV_FREE(is->researched_techs);
    CIV_FREE(is->current_research);
    CIV_FREE(is);
}

void civ_innovation_system_init(civ_innovation_system_t* is) {
    if (!is) return;
    
    memset(is, 0, sizeof(civ_innovation_system_t));
    
    is->tech_level = 0.0f;  /* Start at 0, no predefined minimum */
    is->research_budget = 100.0f;
    is->tech_capacity = 50;
    is->technologies = (civ_technology_node_t*)CIV_CALLOC(is->tech_capacity, sizeof(civ_technology_node_t));
    is->researched_capacity = 50;
    is->researched_techs = (civ_id_t*)CIV_CALLOC(is->researched_capacity, sizeof(civ_id_t));
}

void civ_innovation_system_update(civ_innovation_system_t* is, civ_float_t time_delta) {
    if (!is || !is->current_research) return;
    
    /* Find current research technology */
    civ_technology_node_t* current_tech = NULL;
    for (size_t i = 0; i < is->tech_count; i++) {
        if (strcmp(is->technologies[i].id, is->current_research) == 0) {
            current_tech = &is->technologies[i];
            break;
        }
    }
    
    if (!current_tech || current_tech->researched) return;
    
    /* Calculate research progress */
    civ_float_t research_rate = is->research_budget * 0.1f * time_delta;
    current_tech->progress += research_rate;
    
    /* Check if research is complete */
    if (current_tech->progress >= current_tech->base_research_cost) {
        current_tech->researched = true;
        current_tech->progress = current_tech->base_research_cost;
        
        /* Add to researched list */
        if (is->researched_count < is->researched_capacity) {
            /* Convert tech_id to civ_id_t (simplified) */
            is->researched_techs[is->researched_count++] = (civ_id_t)is->researched_count;
        }
        
        /* Update tech level */
        is->tech_level += 0.1f;
        
        /* Clear current research */
        CIV_FREE(is->current_research);
        is->current_research = NULL;
        
        civ_log(CIV_LOG_INFO, "Technology researched: %s", current_tech->name);
    }
}

civ_result_t civ_innovation_system_research_tech(civ_innovation_system_t* is, const char* tech_id) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!is || !tech_id) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Find technology */
    civ_technology_node_t* tech = NULL;
    for (size_t i = 0; i < is->tech_count; i++) {
        if (strcmp(is->technologies[i].id, tech_id) == 0) {
            tech = &is->technologies[i];
            break;
        }
    }
    
    if (!tech) {
        result.error = CIV_ERROR_NOT_FOUND;
        result.message = "Technology not found";
        return result;
    }
    
    if (tech->researched) {
        result.error = CIV_ERROR_INVALID_STATE;
        result.message = "Technology already researched";
        return result;
    }
    
    /* Check prerequisites */
    for (size_t i = 0; i < tech->prerequisite_count; i++) {
        bool found = false;
        for (size_t j = 0; j < is->researched_count; j++) {
            if (tech->prerequisites[i] == is->researched_techs[j]) {
                found = true;
                break;
            }
        }
        if (!found) {
            result.error = CIV_ERROR_INVALID_STATE;
            result.message = "Prerequisites not met";
            return result;
        }
    }
    
    /* Set as current research */
    if (is->current_research) {
        CIV_FREE(is->current_research);
    }
    is->current_research = (char*)CIV_MALLOC(strlen(tech_id) + 1);
    if (is->current_research) {
        strcpy(is->current_research, tech_id);
    }
    
    return result;
}

civ_float_t civ_innovation_system_get_tech_level(const civ_innovation_system_t* is) {
    if (!is) return 0.0f;
    return is->tech_level;
}

void civ_innovation_system_set_research_budget(civ_innovation_system_t* is, civ_float_t budget) {
    if (!is) return;
    is->research_budget = MAX(0.0f, budget);
}

