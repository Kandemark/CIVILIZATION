/**
 * @file demographics.h
 * @brief Population demographics system
 */

#ifndef CIVILIZATION_DEMOGRAPHICS_H
#define CIVILIZATION_DEMOGRAPHICS_H

#include "../../common.h"
#include "../../types.h"

/* Age group structure */
typedef struct {
    int32_t min_age;
    int32_t max_age;
    int32_t count;
    civ_float_t fertility_rate;
    civ_float_t mortality_rate;
} civ_age_group_t;

/* Demographics structure */
typedef struct {
    int64_t total_population;
    civ_age_group_t* age_groups;
    size_t age_group_count;
    civ_float_t gender_ratio;
    civ_float_t urban_ratio;
    civ_float_t rural_ratio;
    
    /* Ethnic composition (simplified) */
    civ_float_t* ethnic_composition;
    size_t ethnic_group_count;
} civ_demographics_t;

/* Function declarations */
civ_demographics_t* civ_demographics_create(int64_t initial_population);
void civ_demographics_destroy(civ_demographics_t* demo);
void civ_demographics_init(civ_demographics_t* demo, int64_t initial_population);
void civ_demographics_update(civ_demographics_t* demo, int32_t births, int32_t deaths, civ_float_t time_delta);
int64_t civ_demographics_get_total(const civ_demographics_t* demo);
civ_float_t civ_demographics_get_workforce_ratio(const civ_demographics_t* demo);

#endif /* CIVILIZATION_DEMOGRAPHICS_H */

