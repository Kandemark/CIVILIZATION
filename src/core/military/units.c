/**
 * @file units.c
 * @brief Implementation of military units
 */

#include "../../../include/core/military/units.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static civ_float_t get_base_strength(civ_unit_type_t type) {
    switch (type) {
        case CIV_UNIT_TYPE_INFANTRY: return 10.0f;
        case CIV_UNIT_TYPE_ARCHER: return 12.0f;
        case CIV_UNIT_TYPE_CAVALRY: return 20.0f;
        case CIV_UNIT_TYPE_ARTILLERY: return 25.0f;
        case CIV_UNIT_TYPE_ARMOR: return 30.0f;
        case CIV_UNIT_TYPE_SUPPORT: return 8.0f;
        case CIV_UNIT_TYPE_SPECIAL_FORCES: return 35.0f;
        default: return 10.0f;
    }
}

static civ_float_t get_movement_speed(civ_unit_type_t type) {
    switch (type) {
        case CIV_UNIT_TYPE_INFANTRY: return 5.0f;
        case CIV_UNIT_TYPE_ARCHER: return 4.0f;
        case CIV_UNIT_TYPE_CAVALRY: return 8.0f;
        case CIV_UNIT_TYPE_ARTILLERY: return 2.0f;
        case CIV_UNIT_TYPE_ARMOR: return 6.0f;
        case CIV_UNIT_TYPE_SUPPORT: return 3.0f;
        case CIV_UNIT_TYPE_SPECIAL_FORCES: return 7.0f;
        default: return 5.0f;
    }
}

civ_unit_manager_t* civ_unit_manager_create(void) {
    civ_unit_manager_t* um = (civ_unit_manager_t*)CIV_MALLOC(sizeof(civ_unit_manager_t));
    if (!um) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate unit manager");
        return NULL;
    }
    
    civ_unit_manager_init(um);
    return um;
}

void civ_unit_manager_destroy(civ_unit_manager_t* um) {
    if (!um) return;
    CIV_FREE(um->units);
    CIV_FREE(um);
}

void civ_unit_manager_init(civ_unit_manager_t* um) {
    if (!um) return;
    
    memset(um, 0, sizeof(civ_unit_manager_t));
    um->unit_capacity = 100;
    um->units = (civ_unit_t*)CIV_CALLOC(um->unit_capacity, sizeof(civ_unit_t));
}

civ_unit_t* civ_unit_manager_create_unit(civ_unit_manager_t* um, civ_unit_type_t type, 
                                         const char* name, int32_t size) {
    if (!um || !name) return NULL;
    
    if (um->unit_count >= um->unit_capacity) {
        um->unit_capacity *= 2;
        um->units = (civ_unit_t*)CIV_REALLOC(um->units, um->unit_capacity * sizeof(civ_unit_t));
    }
    
    civ_unit_t* unit = &um->units[um->unit_count++];
    memset(unit, 0, sizeof(civ_unit_t));
    
    snprintf(unit->id, sizeof(unit->id), "unit_%zu", um->unit_count);
    strncpy(unit->name, name, sizeof(unit->name) - 1);
    unit->unit_type = type;
    unit->combat_strength = get_base_strength(type);
    unit->movement_speed = get_movement_speed(type);
    unit->supply_consumption = 1.0f;
    unit->morale = 0.6f;
    unit->experience = 0.0f;
    unit->current_strength = size;
    unit->max_strength = size;
    
    return unit;
}

void civ_unit_manager_recruit_units(civ_unit_manager_t* um, const char* nation_id, 
                                    civ_unit_type_t type, int32_t count, civ_float_t quality) {
    if (!um || !nation_id) return;
    
    char name[STRING_MEDIUM_LEN];
    snprintf(name, sizeof(name), "%s_%d", nation_id, type);
    
    civ_unit_t* unit = civ_unit_manager_create_unit(um, type, name, count);
    if (unit) {
        unit->combat_strength *= quality;
        unit->morale = quality;
    }
}

bool civ_unit_manager_update_strength(civ_unit_manager_t* um, const char* unit_id, 
                                     int32_t casualties, int32_t prisoners) {
    if (!um || !unit_id) return false;
    
    for (size_t i = 0; i < um->unit_count; i++) {
        if (strcmp(um->units[i].id, unit_id) == 0) {
            civ_unit_t* unit = &um->units[i];
            int32_t total_losses = casualties + prisoners;
            unit->current_strength = MAX(0, unit->current_strength - total_losses);
            
            /* Update morale based on casualties */
            if (unit->max_strength > 0) {
                civ_float_t casualty_ratio = (civ_float_t)total_losses / (civ_float_t)unit->max_strength;
                unit->morale = MAX(0.1f, unit->morale - casualty_ratio * 0.3f);
                unit->experience = MIN(1.0f, unit->experience + casualty_ratio * 0.1f);
            }
            
            return true;
        }
    }
    
    return false;
}

