/**
 * @file demographics.c
 * @brief Implementation of demographics system
 */

#include "../../../include/core/population/demographics.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEFAULT_AGE_GROUPS 7

static void initialize_age_groups(civ_demographics_t* demo, int64_t initial_pop) {
    /* Standard age groups: 0-14, 15-24, 25-34, 35-44, 45-54, 55-64, 65+ */
    int32_t age_ranges[][2] = {
        {0, 14}, {15, 24}, {25, 34}, {35, 44}, {45, 54}, {55, 64}, {65, 100}
    };
    
    civ_float_t fertility_rates[] = {0.0f, 0.3f, 0.4f, 0.3f, 0.1f, 0.0f, 0.0f};
    civ_float_t mortality_rates[] = {0.01f, 0.005f, 0.01f, 0.02f, 0.04f, 0.08f, 0.15f};
    
    demo->age_group_count = DEFAULT_AGE_GROUPS;
    demo->age_groups = (civ_age_group_t*)CIV_CALLOC(demo->age_group_count, sizeof(civ_age_group_t));
    
    if (!demo->age_groups) return;
    
    /* Distribute population across age groups (pyramid distribution) */
    civ_float_t distribution[] = {0.25f, 0.20f, 0.18f, 0.15f, 0.12f, 0.07f, 0.03f};
    
    for (size_t i = 0; i < demo->age_group_count; i++) {
        demo->age_groups[i].min_age = age_ranges[i][0];
        demo->age_groups[i].max_age = age_ranges[i][1];
        demo->age_groups[i].count = (int32_t)(initial_pop * distribution[i]);
        demo->age_groups[i].fertility_rate = fertility_rates[i];
        demo->age_groups[i].mortality_rate = mortality_rates[i];
    }
}

civ_demographics_t* civ_demographics_create(int64_t initial_population) {
    civ_demographics_t* demo = (civ_demographics_t*)CIV_MALLOC(sizeof(civ_demographics_t));
    if (!demo) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate demographics");
        return NULL;
    }
    
    civ_demographics_init(demo, initial_population);
    return demo;
}

void civ_demographics_destroy(civ_demographics_t* demo) {
    if (!demo) return;
    
    CIV_FREE(demo->age_groups);
    CIV_FREE(demo->ethnic_composition);
    CIV_FREE(demo);
}

void civ_demographics_init(civ_demographics_t* demo, int64_t initial_population) {
    if (!demo) return;
    
    memset(demo, 0, sizeof(civ_demographics_t));
    demo->total_population = initial_population;
    demo->gender_ratio = 0.5f;
    demo->urban_ratio = 0.6f;
    demo->rural_ratio = 0.4f;
    
    initialize_age_groups(demo, initial_population);
    
    /* Initialize ethnic composition (simplified - 1 default group) */
    demo->ethnic_group_count = 1;
    demo->ethnic_composition = (civ_float_t*)CIV_CALLOC(1, sizeof(civ_float_t));
    if (demo->ethnic_composition) {
        demo->ethnic_composition[0] = 1.0f;
    }
}

void civ_demographics_update(civ_demographics_t* demo, int32_t births, int32_t deaths, civ_float_t time_delta) {
    if (!demo) return;
    
    /* Update total population */
    demo->total_population += births - deaths;
    demo->total_population = MAX(0, demo->total_population);
    
    /* Add births to youngest age group */
    if (demo->age_groups && demo->age_group_count > 0) {
        demo->age_groups[0].count += births;
    }
    
    /* Apply mortality to all age groups */
    for (size_t i = 0; i < demo->age_group_count; i++) {
        int32_t age_deaths = (int32_t)(demo->age_groups[i].count * demo->age_groups[i].mortality_rate * time_delta);
        demo->age_groups[i].count = MAX(0, demo->age_groups[i].count - age_deaths);
    }
    
    /* Age population (simplified - move people between age groups) */
    /* This is a simplified model - in reality would need more sophisticated aging */
    if (time_delta >= 1.0f) { /* Only age if significant time passed */
        /* Move people up age groups periodically */
        /* Simplified: just maintain distribution */
    }
}

int64_t civ_demographics_get_total(const civ_demographics_t* demo) {
    if (!demo) return 0;
    return demo->total_population;
}

civ_float_t civ_demographics_get_workforce_ratio(const civ_demographics_t* demo) {
    if (!demo || !demo->age_groups) return 0.0f;
    
    /* Workforce is typically ages 15-64 */
    int64_t workforce = 0;
    int64_t total = 0;
    
    for (size_t i = 0; i < demo->age_group_count; i++) {
        total += demo->age_groups[i].count;
        if (demo->age_groups[i].min_age >= 15 && demo->age_groups[i].max_age <= 64) {
            workforce += demo->age_groups[i].count;
        }
    }
    
    return total > 0 ? (civ_float_t)workforce / (civ_float_t)total : 0.0f;
}

