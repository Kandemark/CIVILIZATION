/**
 * @file cultural_identity.h
 * @brief Cultural identity system
 */

#ifndef CIVILIZATION_CULTURAL_IDENTITY_H
#define CIVILIZATION_CULTURAL_IDENTITY_H

#include "../../common.h"
#include "../../types.h"

/* Cultural trait */
typedef struct {
    char name[STRING_SHORT_LEN];
    civ_float_t strength;  /* 0.0 to 1.0 */
    civ_float_t influence;
} civ_cultural_trait_t;

/* Cultural identity */
typedef struct {
    char id[STRING_SHORT_LEN];
    char name[STRING_MEDIUM_LEN];
    char description[STRING_MAX_LEN];
    
    civ_cultural_trait_t* traits;
    size_t trait_count;
    size_t trait_capacity;
    
    civ_float_t cohesion;  /* 0.0 to 1.0 */
    civ_float_t distinctiveness;  /* 0.0 to 1.0 */
    civ_float_t influence_radius;
    
    time_t creation_time;
    time_t last_update;
} civ_cultural_identity_t;

/* Cultural identity manager */
typedef struct {
    civ_cultural_identity_t* identities;
    size_t identity_count;
    size_t identity_capacity;
} civ_cultural_identity_manager_t;

/* Function declarations */
civ_cultural_identity_manager_t* civ_cultural_identity_manager_create(void);
void civ_cultural_identity_manager_destroy(civ_cultural_identity_manager_t* manager);
void civ_cultural_identity_manager_init(civ_cultural_identity_manager_t* manager);

civ_cultural_identity_t* civ_cultural_identity_create(const char* id, const char* name);
void civ_cultural_identity_destroy(civ_cultural_identity_t* identity);
civ_result_t civ_cultural_identity_add_trait(civ_cultural_identity_t* identity, const char* trait_name, civ_float_t strength);
civ_result_t civ_cultural_identity_update(civ_cultural_identity_t* identity, civ_float_t time_delta);
civ_float_t civ_cultural_identity_calculate_similarity(const civ_cultural_identity_t* a, const civ_cultural_identity_t* b);
civ_result_t civ_cultural_identity_manager_add(civ_cultural_identity_manager_t* manager, civ_cultural_identity_t* identity);
civ_cultural_identity_t* civ_cultural_identity_manager_find(const civ_cultural_identity_manager_t* manager, const char* id);

#endif /* CIVILIZATION_CULTURAL_IDENTITY_H */

