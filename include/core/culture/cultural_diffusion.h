/**
 * @file cultural_diffusion.h
 * @brief Cultural diffusion system
 */

#ifndef CIVILIZATION_CULTURAL_DIFFUSION_H
#define CIVILIZATION_CULTURAL_DIFFUSION_H

#include "../../common.h"
#include "../../types.h"
#include "cultural_identity.h"

/* Diffusion event */
typedef struct {
    char source_id[STRING_SHORT_LEN];
    char target_id[STRING_SHORT_LEN];
    char trait_name[STRING_SHORT_LEN];
    civ_float_t intensity;
    civ_float_t distance;
    time_t timestamp;
} civ_cultural_diffusion_event_t;

/* Cultural diffusion system */
typedef struct {
    civ_cultural_diffusion_event_t* events;
    size_t event_count;
    size_t event_capacity;
    
    civ_float_t base_diffusion_rate;
    civ_float_t distance_decay;
    civ_float_t resistance_factor;
} civ_cultural_diffusion_t;

/* Function declarations */
civ_cultural_diffusion_t* civ_cultural_diffusion_create(void);
void civ_cultural_diffusion_destroy(civ_cultural_diffusion_t* diffusion);
void civ_cultural_diffusion_init(civ_cultural_diffusion_t* diffusion);

civ_result_t civ_cultural_diffusion_process(civ_cultural_diffusion_t* diffusion,
                                            civ_cultural_identity_manager_t* manager,
                                            civ_float_t time_delta);
civ_result_t civ_cultural_diffusion_diffuse_trait(civ_cultural_diffusion_t* diffusion,
                                                  const civ_cultural_identity_t* source,
                                                  civ_cultural_identity_t* target,
                                                  const char* trait_name,
                                                  civ_float_t distance);
civ_float_t civ_cultural_diffusion_calculate_rate(civ_cultural_diffusion_t* diffusion,
                                                   civ_float_t source_strength,
                                                   civ_float_t target_resistance,
                                                   civ_float_t distance);

#endif /* CIVILIZATION_CULTURAL_DIFFUSION_H */

