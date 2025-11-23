/**
 * @file faction_system.c
 * @brief Implementation of faction system
 */

#include "../../../include/core/politics/faction_system.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

civ_faction_system_t* civ_faction_system_create(void) {
    civ_faction_system_t* system = (civ_faction_system_t*)CIV_MALLOC(sizeof(civ_faction_system_t));
    if (!system) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate faction system");
        return NULL;
    }
    
    civ_faction_system_init(system);
    return system;
}

void civ_faction_system_destroy(civ_faction_system_t* system) {
    if (!system) return;
    
    for (size_t i = 0; i < system->faction_count; i++) {
        civ_faction_destroy(&system->factions[i]);
    }
    CIV_FREE(system->factions);
    CIV_FREE(system);
}

void civ_faction_system_init(civ_faction_system_t* system) {
    if (!system) return;
    
    memset(system, 0, sizeof(civ_faction_system_t));
    system->faction_capacity = 16;
    system->factions = (civ_political_faction_t*)CIV_CALLOC(system->faction_capacity, sizeof(civ_political_faction_t));
}

civ_political_faction_t* civ_faction_create(const char* id, const char* name, civ_faction_ideology_t ideology) {
    if (!id || !name) return NULL;
    
    civ_political_faction_t* faction = (civ_political_faction_t*)CIV_MALLOC(sizeof(civ_political_faction_t));
    if (!faction) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate faction");
        return NULL;
    }
    
    memset(faction, 0, sizeof(civ_political_faction_t));
    strncpy(faction->id, id, sizeof(faction->id) - 1);
    strncpy(faction->name, name, sizeof(faction->name) - 1);
    faction->ideology = ideology;
    faction->support = 0.1f;
    faction->influence = 0.1f;
    faction->power = 0.1f;
    faction->loyalty = 0.5f;
    faction->ambition = 0.5f;
    faction->creation_time = time(NULL);
    
    return faction;
}

void civ_faction_destroy(civ_political_faction_t* faction) {
    if (!faction) return;
    /* Nothing to free for now */
}

civ_result_t civ_faction_system_add(civ_faction_system_t* system, civ_political_faction_t* faction) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!system || !faction) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (system->faction_count >= system->faction_capacity) {
        system->faction_capacity *= 2;
        system->factions = (civ_political_faction_t*)CIV_REALLOC(system->factions,
                                                                system->faction_capacity * sizeof(civ_political_faction_t));
    }
    
    if (system->factions) {
        system->factions[system->faction_count++] = *faction;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_political_faction_t* civ_faction_system_find(const civ_faction_system_t* system, const char* id) {
    if (!system || !id) return NULL;
    
    for (size_t i = 0; i < system->faction_count; i++) {
        if (strcmp(system->factions[i].id, id) == 0) {
            return (civ_political_faction_t*)&system->factions[i];
        }
    }
    
    return NULL;
}

civ_result_t civ_faction_system_update(civ_faction_system_t* system, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!system) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update faction dynamics */
    for (size_t i = 0; i < system->faction_count; i++) {
        civ_political_faction_t* faction = &system->factions[i];
        
        /* Power changes based on support and influence */
        civ_float_t power_change = (faction->support * faction->influence - faction->power) * time_delta * 0.1f;
        faction->power = CLAMP(faction->power + power_change, 0.0f, 1.0f);
        
        /* Influence changes based on power */
        civ_float_t influence_change = (faction->power - faction->influence) * time_delta * 0.05f;
        faction->influence = CLAMP(faction->influence + influence_change, 0.0f, 1.0f);
    }
    
    return result;
}

civ_float_t civ_faction_system_calculate_stability(const civ_faction_system_t* system) {
    if (!system || system->faction_count == 0) return 1.0f;
    
    /* Calculate stability based on faction balance */
    civ_float_t total_power = 0.0f;
    civ_float_t max_power = 0.0f;
    
    for (size_t i = 0; i < system->faction_count; i++) {
        total_power += system->factions[i].power;
        if (system->factions[i].power > max_power) {
            max_power = system->factions[i].power;
        }
    }
    
    if (total_power == 0.0f) return 1.0f;
    
    /* Stability is higher when power is more evenly distributed */
    civ_float_t balance = 1.0f - (max_power / total_power);
    return CLAMP(balance, 0.0f, 1.0f);
}

