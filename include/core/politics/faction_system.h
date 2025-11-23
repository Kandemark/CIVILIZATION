/**
 * @file faction_system.h
 * @brief Political faction system
 */

#ifndef CIVILIZATION_FACTION_SYSTEM_H
#define CIVILIZATION_FACTION_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Faction ideology */
typedef enum {
    CIV_FACTION_LIBERAL = 0,
    CIV_FACTION_CONSERVATIVE,
    CIV_FACTION_RADICAL,
    CIV_FACTION_MODERATE,
    CIV_FACTION_OTHER
} civ_faction_ideology_t;

/* Political faction */
typedef struct {
    char id[STRING_SHORT_LEN];
    char name[STRING_MEDIUM_LEN];
    
    civ_faction_ideology_t ideology;
    civ_float_t support;  /* 0.0 to 1.0 - percentage of population */
    civ_float_t influence; /* 0.0 to 1.0 */
    civ_float_t power;    /* 0.0 to 1.0 */
    
    civ_float_t loyalty;  /* 0.0 to 1.0 */
    civ_float_t ambition; /* 0.0 to 1.0 */
    
    time_t creation_time;
} civ_political_faction_t;

/* Faction system */
typedef struct {
    civ_political_faction_t* factions;
    size_t faction_count;
    size_t faction_capacity;
} civ_faction_system_t;

/* Function declarations */
civ_faction_system_t* civ_faction_system_create(void);
void civ_faction_system_destroy(civ_faction_system_t* system);
void civ_faction_system_init(civ_faction_system_t* system);

civ_political_faction_t* civ_faction_create(const char* id, const char* name, civ_faction_ideology_t ideology);
void civ_faction_destroy(civ_political_faction_t* faction);
civ_result_t civ_faction_system_add(civ_faction_system_t* system, civ_political_faction_t* faction);
civ_political_faction_t* civ_faction_system_find(const civ_faction_system_t* system, const char* id);
civ_result_t civ_faction_system_update(civ_faction_system_t* system, civ_float_t time_delta);
civ_float_t civ_faction_system_calculate_stability(const civ_faction_system_t* system);

#endif /* CIVILIZATION_FACTION_SYSTEM_H */

