/**
 * @file units.h
 * @brief Military units system
 */

#ifndef CIVILIZATION_UNITS_H
#define CIVILIZATION_UNITS_H

#include "../../common.h"
#include "../../types.h"

/* Unit type enumeration */
typedef enum {
    CIV_UNIT_TYPE_INFANTRY = 0,
    CIV_UNIT_TYPE_CAVALRY,
    CIV_UNIT_TYPE_ARCHER,
    CIV_UNIT_TYPE_ARTILLERY,
    CIV_UNIT_TYPE_ARMOR,
    CIV_UNIT_TYPE_SUPPORT,
    CIV_UNIT_TYPE_SPECIAL_FORCES
} civ_unit_type_t;

/* Unit structure */
typedef struct {
    char id[STRING_SHORT_LEN];
    char name[STRING_MEDIUM_LEN];
    civ_unit_type_t unit_type;
    civ_float_t combat_strength;
    civ_float_t movement_speed;
    civ_float_t supply_consumption;
    civ_float_t morale;
    civ_float_t experience;
    int32_t current_strength;
    int32_t max_strength;
} civ_unit_t;

/* Unit manager structure */
typedef struct {
    civ_unit_t* units;
    size_t unit_count;
    size_t unit_capacity;
} civ_unit_manager_t;

/* Function declarations */
civ_unit_manager_t* civ_unit_manager_create(void);
void civ_unit_manager_destroy(civ_unit_manager_t* um);
void civ_unit_manager_init(civ_unit_manager_t* um);

civ_unit_t* civ_unit_manager_create_unit(civ_unit_manager_t* um, civ_unit_type_t type, 
                                         const char* name, int32_t size);
void civ_unit_manager_recruit_units(civ_unit_manager_t* um, const char* nation_id, 
                                    civ_unit_type_t type, int32_t count, civ_float_t quality);
bool civ_unit_manager_update_strength(civ_unit_manager_t* um, const char* unit_id, 
                                     int32_t casualties, int32_t prisoners);

#endif /* CIVILIZATION_UNITS_H */

