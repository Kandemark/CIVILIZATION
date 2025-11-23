/**
 * @file population_manager.c
 * @brief Implementation of population manager
 */

#include "../../../include/core/population/population_manager.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

civ_population_manager_t* civ_population_manager_create(void) {
    civ_population_manager_t* pm = (civ_population_manager_t*)CIV_MALLOC(sizeof(civ_population_manager_t));
    if (!pm) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate population manager");
        return NULL;
    }
    
    civ_population_manager_init(pm);
    return pm;
}

void civ_population_manager_destroy(civ_population_manager_t* pm) {
    if (!pm) return;
    
    if (pm->demographics) {
        civ_demographics_destroy(pm->demographics);
    }
    
    if (pm->region_ids) {
        for (size_t i = 0; i < pm->region_count; i++) {
            CIV_FREE(pm->region_ids[i]);
        }
        CIV_FREE(pm->region_ids);
    }
    
    CIV_FREE(pm);
}

void civ_population_manager_init(civ_population_manager_t* pm) {
    if (!pm) return;
    
    memset(pm, 0, sizeof(civ_population_manager_t));
    
    pm->birth_rate = 0.025f;  /* 2.5% per year */
    pm->death_rate = 0.015f;  /* 1.5% per year */
    pm->migration_rate = 0.0f;
    pm->growth_rate = pm->birth_rate - pm->death_rate;
    
    pm->education_quality = 0.5f;
    pm->health_index = 0.6f;
    pm->satisfaction = 0.5f;
    
    /* Initialize default demographics */
    pm->demographics = civ_demographics_create(1000);
    
    pm->region_capacity = 10;
    pm->region_ids = (char**)CIV_CALLOC(pm->region_capacity, sizeof(char*));
}

void civ_population_manager_update(civ_population_manager_t* pm, civ_float_t time_delta, 
                                   const void* economic_conditions) {
    if (!pm || !pm->demographics) return;
    
    /* Calculate births and deaths based on rates */
    int64_t current_pop = civ_demographics_get_total(pm->demographics);
    
    /* Adjust rates based on economic conditions */
    civ_float_t economic_modifier = 1.0f;
    if (economic_conditions) {
        /* Simplified: better economy = lower death rate, stable birth rate */
        /* In full implementation, would extract from economic_conditions struct */
        economic_modifier = 0.9f; /* Placeholder */
    }
    
    /* Calculate vital events */
    int32_t births = (int32_t)(current_pop * pm->birth_rate * time_delta);
    int32_t deaths = (int32_t)(current_pop * pm->death_rate * time_delta * economic_modifier);
    
    /* Apply health and education modifiers */
    civ_float_t health_modifier = 1.0f - (1.0f - pm->health_index) * 0.3f;
    deaths = (int32_t)(deaths * health_modifier);
    
    civ_float_t education_modifier = 1.0f + pm->education_quality * 0.1f;
    births = (int32_t)(births * education_modifier);
    
    /* Update demographics */
    civ_demographics_update(pm->demographics, births, deaths, time_delta);
    
    /* Update growth rate */
    int64_t new_pop = civ_demographics_get_total(pm->demographics);
    if (current_pop > 0 && time_delta > 0) {
        pm->growth_rate = ((civ_float_t)(new_pop - current_pop) / (civ_float_t)current_pop) / time_delta;
    }
}

void civ_population_manager_initialize_region(civ_population_manager_t* pm, const char* region_id, 
                                               int64_t initial_population) {
    if (!pm || !region_id) return;
    
    /* Check if region already exists */
    for (size_t i = 0; i < pm->region_count; i++) {
        if (strcmp(pm->region_ids[i], region_id) == 0) {
            return; /* Region already exists */
        }
    }
    
    /* Expand capacity if needed */
    if (pm->region_count >= pm->region_capacity) {
        pm->region_capacity *= 2;
        pm->region_ids = (char**)CIV_REALLOC(pm->region_ids, pm->region_capacity * sizeof(char*));
    }
    
    /* Add region */
    size_t id_len = strlen(region_id) + 1;
    pm->region_ids[pm->region_count] = (char*)CIV_MALLOC(id_len);
    if (pm->region_ids[pm->region_count]) {
        strcpy(pm->region_ids[pm->region_count], region_id);
        pm->region_count++;
    }
    
    /* Initialize demographics for this region (simplified - using main demographics) */
    if (!pm->demographics) {
        pm->demographics = civ_demographics_create(initial_population);
    }
}

int64_t civ_population_manager_get_total(civ_population_manager_t* pm) {
    if (!pm || !pm->demographics) return 0;
    return civ_demographics_get_total(pm->demographics);
}

civ_float_t civ_population_manager_get_growth_rate(const civ_population_manager_t* pm) {
    if (!pm) return 0.0f;
    return pm->growth_rate;
}

char* civ_population_manager_to_dict(const civ_population_manager_t* pm) {
    if (!pm) return NULL;
    
    char* json = (char*)CIV_MALLOC(512);
    if (!json) return NULL;
    
    int64_t total = civ_population_manager_get_total(pm);
    snprintf(json, 512,
        "{\"total_population\":%lld,\"birth_rate\":%.3f,\"death_rate\":%.3f,"
        "\"growth_rate\":%.3f,\"education_quality\":%.3f,\"health_index\":%.3f}",
        (long long)total, pm->birth_rate, pm->death_rate, pm->growth_rate,
        pm->education_quality, pm->health_index);
    
    return json;
}

civ_result_t civ_population_manager_from_dict(civ_population_manager_t* pm, const char* json) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!pm || !json) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Simple JSON parsing (in production, use proper JSON library) */
    /* For now, just reinitialize */
    civ_population_manager_init(pm);
    
    return result;
}

