/**
 * @file conquest.h
 * @brief Conquest and warfare mechanics
 */

#ifndef CIVILIZATION_CONQUEST_H
#define CIVILIZATION_CONQUEST_H

#include "../../common.h"
#include "../../types.h"
#include "../world/territory.h"
#include "../world/map_generator.h"
#include "../culture/cultural_assimilation.h"

/* Conquest type */
typedef enum {
    CIV_CONQUEST_INVASION = 0,
    CIV_CONQUEST_SIEGE,
    CIV_CONQUEST_RAID,
    CIV_CONQUEST_ANNEXATION
} civ_conquest_type_t;

/* Plunder result */
typedef struct {
    civ_float_t gold;
    civ_float_t resources;
    civ_float_t artifacts;
    int32_t population_captured;
    civ_float_t knowledge_gained;
} civ_plunder_result_t;

/* Conquest event */
typedef struct {
    char attacker_id[STRING_SHORT_LEN];
    char defender_id[STRING_SHORT_LEN];
    char target_region_id[STRING_SHORT_LEN];
    
    civ_conquest_type_t type;
    civ_float_t progress;  /* 0.0 to 1.0 */
    civ_float_t attacker_strength;
    civ_float_t defender_strength;
    
    civ_plunder_result_t plunder;
    bool assimilation_enabled;
    civ_assimilation_type_t assimilation_type;
    
    time_t start_time;
    time_t last_update;
    bool   completed;          /* true when progress hit 1.0 */
    bool   territory_applied;  /* true after territory transfer done */
} civ_conquest_event_t;

/* Conquest system */
typedef struct {
    civ_conquest_event_t* conquests;
    size_t conquest_count;
    size_t conquest_capacity;
    
    civ_float_t base_conquest_rate;
    civ_float_t plunder_multiplier;
} civ_conquest_system_t;

/* Function declarations */
civ_conquest_system_t* civ_conquest_system_create(void);
void civ_conquest_system_destroy(civ_conquest_system_t* system);
void civ_conquest_system_init(civ_conquest_system_t* system);

civ_result_t civ_conquest_start(civ_conquest_system_t* system,
                                const char* attacker_id, const char* defender_id,
                                const char* target_region_id, civ_conquest_type_t type);
civ_result_t civ_conquest_update(civ_conquest_system_t* system, civ_float_t time_delta);

/* Transfer territory from defender to attacker when conquest completes.
   nation_manager is civ_nation_manager_t*, passed as void* to avoid circular include. */
int civ_conquest_transfer_territory(civ_conquest_system_t *system, civ_map_t *map,
                                     void *nation_manager);
civ_result_t civ_conquest_plunder(civ_conquest_event_t* conquest, civ_plunder_result_t* result);
civ_result_t civ_conquest_apply_assimilation(civ_conquest_event_t* conquest,
                                             civ_assimilation_tracker_t* assimilation_tracker);
civ_conquest_event_t* civ_conquest_find(const civ_conquest_system_t* system,
                                       const char* attacker_id, const char* defender_id);

#endif /* CIVILIZATION_CONQUEST_H */

