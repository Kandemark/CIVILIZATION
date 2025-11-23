/**
 * @file territory.h
 * @brief Dynamic territory system (not tile-based)
 */

#ifndef CIVILIZATION_TERRITORY_H
#define CIVILIZATION_TERRITORY_H

#include "../../common.h"
#include "../../types.h"
#include "../environment/geography.h"

/* Territory point (for polygon-based territory) */
typedef struct {
    civ_float_t x;
    civ_float_t y;
} civ_territory_point_t;

/* Territory region */
typedef struct {
    char nation_id[STRING_SHORT_LEN];
    civ_territory_point_t* boundary_points;  /* Polygon boundary */
    size_t point_count;
    size_t point_capacity;
    
    civ_float_t area;
    civ_coordinate_t centroid;
    civ_float_t control_strength;  /* 0.0 to 1.0 - how well controlled */
    
    time_t acquisition_time;
} civ_territory_region_t;

/* Territory manager */
typedef struct {
    civ_territory_region_t* regions;
    size_t region_count;
    size_t region_capacity;
} civ_territory_manager_t;

/* Function declarations */
civ_territory_manager_t* civ_territory_manager_create(void);
void civ_territory_manager_destroy(civ_territory_manager_t* manager);
void civ_territory_manager_init(civ_territory_manager_t* manager);

civ_territory_region_t* civ_territory_region_create(const char* nation_id);
void civ_territory_region_destroy(civ_territory_region_t* region);
civ_result_t civ_territory_region_add_point(civ_territory_region_t* region, civ_float_t x, civ_float_t y);
civ_result_t civ_territory_region_calculate_area(civ_territory_region_t* region);
civ_result_t civ_territory_region_calculate_centroid(civ_territory_region_t* region);
bool civ_territory_region_contains_point(const civ_territory_region_t* region, civ_float_t x, civ_float_t y);
civ_result_t civ_territory_manager_add_region(civ_territory_manager_t* manager, civ_territory_region_t* region);
civ_territory_region_t* civ_territory_manager_find_region_at(civ_territory_manager_t* manager, civ_float_t x, civ_float_t y);
civ_result_t civ_territory_manager_update(civ_territory_manager_t* manager, civ_float_t time_delta);

#endif /* CIVILIZATION_TERRITORY_H */

