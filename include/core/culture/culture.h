/**
 * @file culture.h
 * @brief Main culture system header
 */

#ifndef CIVILIZATION_CULTURE_H
#define CIVILIZATION_CULTURE_H

#include "../../common.h"
#include "../../types.h"
#include "cultural_identity.h"
#include "cultural_diffusion.h"
#include "cultural_assimilation.h"
#include "language_evolution.h"

/* Culture system */
typedef struct {
    civ_cultural_identity_manager_t* identity_manager;
    civ_cultural_diffusion_t* diffusion;
    civ_assimilation_tracker_t* assimilation_tracker;
    civ_language_evolution_t* language_evolution;
    civ_writing_system_manager_t* writing_system_manager;
} civ_culture_system_t;

/* Function declarations */
civ_culture_system_t* civ_culture_system_create(void);
void civ_culture_system_destroy(civ_culture_system_t* culture);
void civ_culture_system_init(civ_culture_system_t* culture);

civ_result_t civ_culture_system_update(civ_culture_system_t* culture, civ_float_t time_delta);

#endif /* CIVILIZATION_CULTURE_H */

