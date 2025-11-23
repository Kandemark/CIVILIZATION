/**
 * @file soft_metrics.c
 * @brief Implementation of soft metrics system
 */

#include "../../../include/core/abstracts/soft_metrics.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

civ_soft_metrics_manager_t* civ_soft_metrics_manager_create(void) {
    civ_soft_metrics_manager_t* sm = (civ_soft_metrics_manager_t*)CIV_MALLOC(sizeof(civ_soft_metrics_manager_t));
    if (!sm) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate soft metrics manager");
        return NULL;
    }
    
    civ_soft_metrics_manager_init(sm);
    return sm;
}

void civ_soft_metrics_manager_destroy(civ_soft_metrics_manager_t* sm) {
    if (!sm) return;
    
    CIV_FREE(sm->happiness_metrics.recent_changes);
    CIV_FREE(sm->prestige_system.international_relations);
    CIV_FREE(sm);
}

void civ_soft_metrics_manager_init(civ_soft_metrics_manager_t* sm) {
    if (!sm) return;
    
    memset(sm, 0, sizeof(civ_soft_metrics_manager_t));
    
    /* Initialize happiness metrics */
    sm->happiness_metrics.base_happiness = 0.5f;
    sm->happiness_metrics.stability = 0.5f;
    sm->happiness_metrics.loyalty = 0.5f;
    sm->happiness_metrics.change_capacity = 10;
    sm->happiness_metrics.recent_changes = (civ_float_t*)CIV_CALLOC(sm->happiness_metrics.change_capacity, sizeof(civ_float_t));
    
    /* Initialize legitimacy system */
    sm->legitimacy_system.legitimacy = 0.7f;
    sm->legitimacy_system.political_stability = 0.6f;
    sm->legitimacy_system.corruption_level = 0.3f;
    sm->legitimacy_system.government_approval = 0.5f;
    
    /* Initialize prestige system */
    sm->prestige_system.prestige = 0.5f;
    sm->prestige_system.cultural_influence = 0.4f;
    sm->prestige_system.technological_achievements = 0.3f;
    sm->prestige_system.military_prowess = 0.5f;
}

civ_float_t civ_happiness_metrics_get_overall(const civ_happiness_metrics_t* hm) {
    if (!hm) return 0.0f;
    
    civ_float_t base = hm->base_happiness;
    civ_float_t stability_factor = hm->stability * 0.3f;
    civ_float_t loyalty_factor = hm->loyalty * 0.2f;
    
    /* Consider recent changes (weighted average) */
    civ_float_t recent_impact = 0.0f;
    if (hm->recent_changes && hm->change_count > 0) {
        civ_float_t weights[] = {0.5f, 0.3f, 0.2f};
        size_t count = MIN(3, hm->change_count);
        size_t start = hm->change_count > 3 ? hm->change_count - 3 : 0;
        
        for (size_t i = 0; i < count; i++) {
            recent_impact += hm->recent_changes[start + i] * weights[i];
        }
    }
    
    civ_float_t overall = base + stability_factor + loyalty_factor + recent_impact * 0.2f;
    return CLAMP(overall, 0.0f, 1.0f);
}

civ_mood_t civ_happiness_metrics_get_mood(const civ_happiness_metrics_t* hm) {
    if (!hm) return CIV_MOOD_CONTENT;
    
    civ_float_t score = civ_happiness_metrics_get_overall(hm);
    
    if (score >= 0.9f) return CIV_MOOD_ECSTATIC;
    if (score >= 0.7f) return CIV_MOOD_HAPPY;
    if (score >= 0.5f) return CIV_MOOD_CONTENT;
    if (score >= 0.3f) return CIV_MOOD_DISCONTENT;
    if (score >= 0.1f) return CIV_MOOD_UNHAPPY;
    return CIV_MOOD_REBELLIOUS;
}

void civ_happiness_metrics_add_change(civ_happiness_metrics_t* hm, civ_float_t change) {
    if (!hm) return;
    
    /* Expand capacity if needed */
    if (hm->change_count >= hm->change_capacity) {
        hm->change_capacity *= 2;
        hm->recent_changes = (civ_float_t*)CIV_REALLOC(hm->recent_changes, 
                                                       hm->change_capacity * sizeof(civ_float_t));
    }
    
    if (hm->recent_changes) {
        hm->recent_changes[hm->change_count++] = change;
        
        /* Keep only last 10 changes */
        if (hm->change_count > 10) {
            memmove(hm->recent_changes, hm->recent_changes + 1, 
                    (hm->change_count - 1) * sizeof(civ_float_t));
            hm->change_count--;
        }
    }
}

civ_float_t civ_legitimacy_calculate_score(const civ_legitimacy_system_t* ls) {
    if (!ls) return 0.0f;
    
    civ_float_t stability_factor = ls->political_stability * 0.4f;
    civ_float_t corruption_factor = (1.0f - ls->corruption_level) * 0.3f;
    civ_float_t approval_factor = ls->government_approval * 0.3f;
    
    return CLAMP(stability_factor + corruption_factor + approval_factor, 0.0f, 1.0f);
}

void civ_soft_metrics_update_from_economy(civ_soft_metrics_manager_t* sm, const void* economic_data) {
    if (!sm || !economic_data) return;
    
    /* Simplified: extract GDP per capita and unemployment from economic data */
    /* In full implementation, would use proper struct */
    civ_float_t gdp_per_capita = 0.6f; /* Placeholder */
    civ_float_t unemployment = 0.3f; /* Placeholder */
    
    /* Update happiness based on economic conditions */
    civ_float_t economic_happiness = gdp_per_capita * 0.5f + (1.0f - unemployment) * 0.5f;
    civ_float_t change = (economic_happiness - sm->happiness_metrics.base_happiness) * 0.1f;
    civ_happiness_metrics_add_change(&sm->happiness_metrics, change);
    sm->happiness_metrics.base_happiness = CLAMP(sm->happiness_metrics.base_happiness + change, 0.0f, 1.0f);
    
    /* Update legitimacy based on economic performance */
    if (gdp_per_capita > 0.7f) {
        sm->legitimacy_system.government_approval = MIN(1.0f, sm->legitimacy_system.government_approval + 0.01f);
    } else if (gdp_per_capita < 0.4f) {
        sm->legitimacy_system.government_approval = MAX(0.0f, sm->legitimacy_system.government_approval - 0.01f);
    }
}

void civ_soft_metrics_update_from_events(civ_soft_metrics_manager_t* sm, const void* events, size_t event_count) {
    if (!sm || !events) return;
    
    /* Simplified event processing */
    /* In full implementation, would iterate through events and apply impacts */
    for (size_t i = 0; i < event_count; i++) {
        /* Process event impacts */
        /* Placeholder */
    }
}

char* civ_soft_metrics_to_dict(const civ_soft_metrics_manager_t* sm) {
    if (!sm) return NULL;
    
    char* json = (char*)CIV_MALLOC(512);
    if (!json) return NULL;
    
    civ_float_t happiness = civ_happiness_metrics_get_overall(&sm->happiness_metrics);
    civ_float_t legitimacy = civ_legitimacy_calculate_score(&sm->legitimacy_system);
    
    snprintf(json, 512,
        "{\"happiness\":%.3f,\"legitimacy\":%.3f,\"prestige\":%.3f,"
        "\"stability\":%.3f,\"corruption\":%.3f}",
        happiness, legitimacy, sm->prestige_system.prestige,
        sm->legitimacy_system.political_stability, sm->legitimacy_system.corruption_level);
    
    return json;
}

