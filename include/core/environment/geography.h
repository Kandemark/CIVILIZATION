/**
 * @file geography.h
 * @brief Geography and environment system
 */

#ifndef CIVILIZATION_GEOGRAPHY_H
#define CIVILIZATION_GEOGRAPHY_H

#include "../../common.h"
#include "../../types.h"

/* Land use type enumeration */
typedef enum {
    CIV_LAND_USE_FOREST = 0,
    CIV_LAND_USE_AGRICULTURE,
    CIV_LAND_USE_URBAN,
    CIV_LAND_USE_WETLAND,
    CIV_LAND_USE_GRASSLAND,
    CIV_LAND_USE_DESERT,
    CIV_LAND_USE_WATER,
    CIV_LAND_USE_TUNDRA
} civ_land_use_type_t;

/* Terrain type enumeration */
typedef enum {
    CIV_TERRAIN_MOUNTAIN = 0,
    CIV_TERRAIN_HILL,
    CIV_TERRAIN_PLAIN,
    CIV_TERRAIN_VALLEY,
    CIV_TERRAIN_PLATEAU,
    CIV_TERRAIN_COASTAL
} civ_terrain_type_t;

/* Geographic coordinate */
typedef struct {
    civ_float_t latitude;
    civ_float_t longitude;
} civ_coordinate_t;

/* Land patch structure */
typedef struct {
    civ_coordinate_t coordinate;
    civ_float_t area;
    civ_float_t elevation;
    civ_land_use_type_t land_use;
    civ_terrain_type_t terrain;
    civ_float_t soil_quality;
    civ_float_t vegetation_cover;
} civ_land_patch_t;

/* Geography system */
typedef struct {
    char region_name[STRING_MEDIUM_LEN];
    civ_coordinate_t bounds_sw;
    civ_coordinate_t bounds_ne;
    civ_land_patch_t* land_patches;
    size_t patch_count;
    size_t patch_capacity;
} civ_geography_t;

/* Function declarations */
civ_geography_t* civ_geography_create(const char* region_name, civ_coordinate_t sw, civ_coordinate_t ne);
void civ_geography_destroy(civ_geography_t* geo);
void civ_geography_init(civ_geography_t* geo, const char* region_name, civ_coordinate_t sw, civ_coordinate_t ne);

civ_result_t civ_geography_add_land_patch(civ_geography_t* geo, const civ_land_patch_t* patch);
civ_float_t civ_geography_calculate_distance(civ_coordinate_t a, civ_coordinate_t b);
civ_float_t civ_geography_get_agricultural_area(const civ_geography_t* geo);

#endif /* CIVILIZATION_GEOGRAPHY_H */

