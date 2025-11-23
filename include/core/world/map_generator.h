/**
 * @file map_generator.h
 * @brief 2D world map generation system
 */

#ifndef CIVILIZATION_MAP_GENERATOR_H
#define CIVILIZATION_MAP_GENERATOR_H

#include "../../common.h"
#include "../../types.h"
#include "../environment/geography.h"

/* Map tile */
typedef struct {
    civ_terrain_type_t terrain;
    civ_land_use_type_t land_use;
    civ_float_t elevation;
    civ_float_t fertility;
    civ_float_t resources;
    int32_t x;
    int32_t y;
} civ_map_tile_t;

/* 2D Map */
typedef struct {
    civ_map_tile_t* tiles;
    int32_t width;
    int32_t height;
    civ_float_t sea_level;
    uint32_t seed;
} civ_map_t;

/* Map generation parameters */
typedef struct {
    int32_t width;
    int32_t height;
    civ_float_t sea_level;
    civ_float_t land_ratio;  /* 0.0 to 1.0 */
    uint32_t seed;
    bool generate_rivers;
    bool generate_mountains;
} civ_map_gen_params_t;

/* Function declarations */
civ_map_t* civ_map_create(int32_t width, int32_t height, uint32_t seed);
void civ_map_destroy(civ_map_t* map);
void civ_map_init(civ_map_t* map, int32_t width, int32_t height, uint32_t seed);

civ_result_t civ_map_generate(civ_map_t* map, const civ_map_gen_params_t* params);
civ_map_tile_t* civ_map_get_tile(const civ_map_t* map, int32_t x, int32_t y);
civ_result_t civ_map_generate_terrain(civ_map_t* map);
civ_result_t civ_map_generate_rivers(civ_map_t* map);
civ_result_t civ_map_generate_resources(civ_map_t* map);

#endif /* CIVILIZATION_MAP_GENERATOR_H */

