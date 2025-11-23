/**
 * @file population_manager.h
 * @brief Population manager system
 */

#ifndef CIVILIZATION_POPULATION_MANAGER_H
#define CIVILIZATION_POPULATION_MANAGER_H

#include "../../common.h"
#include "../../types.h"
#include "demographics.h"

/* Population manager structure */
typedef struct {
    civ_demographics_t* demographics;
    civ_float_t birth_rate;
    civ_float_t death_rate;
    civ_float_t migration_rate;
    civ_float_t growth_rate;
    
    /* Subsystems */
    civ_float_t education_quality;
    civ_float_t health_index;
    civ_float_t satisfaction;
    
    /* Regional data */
    char** region_ids;
    size_t region_count;
    size_t region_capacity;
} civ_population_manager_t;

/* Function declarations */
civ_population_manager_t* civ_population_manager_create(void);
void civ_population_manager_destroy(civ_population_manager_t* pm);
void civ_population_manager_init(civ_population_manager_t* pm);

void civ_population_manager_update(civ_population_manager_t* pm, civ_float_t time_delta, 
                                   const void* economic_conditions);
void civ_population_manager_initialize_region(civ_population_manager_t* pm, const char* region_id, 
                                             int64_t initial_population);

int64_t civ_population_manager_get_total(civ_population_manager_t* pm);
civ_float_t civ_population_manager_get_growth_rate(const civ_population_manager_t* pm);

/* Serialization */
char* civ_population_manager_to_dict(const civ_population_manager_t* pm);
civ_result_t civ_population_manager_from_dict(civ_population_manager_t* pm, const char* json);

#endif /* CIVILIZATION_POPULATION_MANAGER_H */

