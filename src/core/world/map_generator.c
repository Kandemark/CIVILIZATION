/**
 * @file map_generator.c
 * @brief Implementation of 2D map generation
 */

#include "../../../include/core/world/map_generator.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Simple PRNG for map generation */
static uint32_t map_rng_state = 0;

static uint32_t map_rng_next(void) {
    map_rng_state = map_rng_state * 1103515245 + 12345;
    return (map_rng_state / 65536) % 32768;
}

static void map_rng_seed(uint32_t seed) {
    map_rng_state = seed;
}

static civ_float_t map_rng_float(void) {
    return (civ_float_t)map_rng_next() / 32768.0f;
}

civ_map_t* civ_map_create(int32_t width, int32_t height, uint32_t seed) {
    civ_map_t* map = (civ_map_t*)CIV_MALLOC(sizeof(civ_map_t));
    if (!map) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate map");
        return NULL;
    }
    
    civ_map_init(map, width, height, seed);
    return map;
}

void civ_map_destroy(civ_map_t* map) {
    if (!map) return;
    CIV_FREE(map->tiles);
    CIV_FREE(map);
}

void civ_map_init(civ_map_t* map, int32_t width, int32_t height, uint32_t seed) {
    if (!map) return;
    
    memset(map, 0, sizeof(civ_map_t));
    map->width = width > 0 ? width : 100;
    map->height = height > 0 ? height : 100;
    map->sea_level = 0.3f;
    map->seed = seed;
    
    size_t tile_count = (size_t)map->width * (size_t)map->height;
    map->tiles = (civ_map_tile_t*)CIV_CALLOC(tile_count, sizeof(civ_map_tile_t));
    
    if (map->tiles) {
        /* Initialize tile positions */
        for (int32_t y = 0; y < map->height; y++) {
            for (int32_t x = 0; x < map->width; x++) {
                civ_map_tile_t* tile = &map->tiles[y * map->width + x];
                tile->x = x;
                tile->y = y;
                tile->terrain = CIV_TERRAIN_PLAIN;
                tile->land_use = CIV_LAND_USE_GRASSLAND;
            }
        }
    }
}

civ_result_t civ_map_generate(civ_map_t* map, const civ_map_gen_params_t* params) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!map || !params) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    map_rng_seed(params->seed);
    
    /* Generate terrain */
    civ_map_generate_terrain(map);
    
    /* Generate rivers if requested */
    if (params->generate_rivers) {
        civ_map_generate_rivers(map);
    }
    
    /* Generate resources */
    civ_map_generate_resources(map);
    
    return result;
}

civ_map_tile_t* civ_map_get_tile(const civ_map_t* map, int32_t x, int32_t y) {
    if (!map || x < 0 || y < 0 || x >= map->width || y >= map->height) {
        return NULL;
    }
    
    return &map->tiles[y * map->width + x];
}

civ_result_t civ_map_generate_terrain(civ_map_t* map) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!map) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Generate elevation using simple noise */
    for (int32_t y = 0; y < map->height; y++) {
        for (int32_t x = 0; x < map->width; x++) {
            civ_map_tile_t* tile = civ_map_get_tile(map, x, y);
            if (!tile) continue;
            
            /* Simple noise-based elevation */
            civ_float_t noise = map_rng_float();
            tile->elevation = noise;
            
            /* Determine terrain based on elevation */
            if (tile->elevation < map->sea_level) {
                tile->terrain = CIV_TERRAIN_PLAIN;  /* Ocean represented as water land use */
                tile->land_use = CIV_LAND_USE_WATER;
            } else if (tile->elevation < map->sea_level + 0.1f) {
                tile->terrain = CIV_TERRAIN_COASTAL;
            } else if (tile->elevation < 0.6f) {
                tile->terrain = CIV_TERRAIN_PLAIN;
            } else if (tile->elevation < 0.8f) {
                tile->terrain = CIV_TERRAIN_HILL;
            } else {
                tile->terrain = CIV_TERRAIN_MOUNTAIN;
            }
            
            /* Fertility based on elevation and terrain */
            if (tile->terrain == CIV_TERRAIN_PLAIN) {
                tile->fertility = 0.7f + map_rng_float() * 0.3f;
            } else if (tile->terrain == CIV_TERRAIN_HILL) {
                tile->fertility = 0.4f + map_rng_float() * 0.3f;
            } else {
                tile->fertility = map_rng_float() * 0.3f;
            }
        }
    }
    
    return result;
}

civ_result_t civ_map_generate_rivers(civ_map_t* map) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!map) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Generate a few rivers */
    int32_t river_count = 3 + (int32_t)(map_rng_float() * 5);
    
    for (int32_t i = 0; i < river_count; i++) {
        /* Start river from high elevation */
        int32_t start_x = (int32_t)(map_rng_float() * map->width);
        int32_t start_y = (int32_t)(map_rng_float() * map->height);
        
        civ_map_tile_t* tile = civ_map_get_tile(map, start_x, start_y);
        if (tile && tile->elevation > map->sea_level + 0.3f) {
            /* Flow river downhill */
            int32_t x = start_x;
            int32_t y = start_y;
            
            for (int32_t step = 0; step < 50; step++) {
                tile = civ_map_get_tile(map, x, y);
                if (!tile || tile->elevation <= map->sea_level) break;
                
                /* Mark as river (use water land use) */
                if (tile->land_use != CIV_LAND_USE_WATER) {
                    tile->land_use = CIV_LAND_USE_WATER;  /* Use water land use for rivers */
                }
                
                /* Find lowest neighbor */
                civ_float_t lowest = tile->elevation;
                int32_t next_x = x;
                int32_t next_y = y;
                
                for (int32_t dy = -1; dy <= 1; dy++) {
                    for (int32_t dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        civ_map_tile_t* neighbor = civ_map_get_tile(map, x + dx, y + dy);
                        if (neighbor && neighbor->elevation < lowest) {
                            lowest = neighbor->elevation;
                            next_x = x + dx;
                            next_y = y + dy;
                        }
                    }
                }
                
                if (next_x == x && next_y == y) break; /* No lower neighbor */
                x = next_x;
                y = next_y;
            }
        }
    }
    
    return result;
}

civ_result_t civ_map_generate_resources(civ_map_t* map) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!map) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Distribute resources randomly */
    for (int32_t y = 0; y < map->height; y++) {
        for (int32_t x = 0; x < map->width; x++) {
            civ_map_tile_t* tile = civ_map_get_tile(map, x, y);
            if (!tile || tile->land_use == CIV_LAND_USE_WATER) continue;
            
            if (map_rng_float() < 0.1f) { /* 10% chance */
                tile->resources = 0.5f + map_rng_float() * 0.5f;
            }
        }
    }
    
    return result;
}

