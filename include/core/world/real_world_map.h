/**
 * @file real_world_map.h
 * @brief Real-world Earth map data loading for Dominion
 *
 * Reads a pre-computed land-water binary mask (equirectangular projection)
 * at the game's tile resolution and populates a civ_map_t.
 */
#ifndef CIV_WORLD_REAL_WORLD_MAP_H
#define CIV_WORLD_REAL_WORLD_MAP_H

#include "../../core/world/map_generator.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Earth map format header (written by tools/generate_earth_map.py)
 */
typedef struct {
  uint32_t magic;    /* "EARTH" = 0x48545241 */
  uint32_t version;  /* 1 */
  int32_t width;     /* e.g. 2048 */
  int32_t height;    /* e.g. 1024 */
  float sea_level;
  uint32_t seed;
  uint32_t reserved[8];
} civ_earth_map_header_t;

#define CIV_EARTH_MAP_MAGIC 0x48545241 /* "EARTH" in little-endian */

/**
 * @brief Load Earth map from binary file and populate a civ_map_t
 *
 * The file format:
 *   [civ_earth_map_header_t] [width*height bytes: 0=water, 1=land]
 *
 * Each pixel in the landmask corresponds to one tile in the map.
 * Elevation is derived from land/water + simple latitude-based variation.
 *
 * @param filepath Path to the .earth binary file
 * @param map Pre-allocated map (width/height must match file)
 * @return CIV_OK on success, error code on failure
 */
civ_result_t civ_earth_map_load(const char *filepath, civ_map_t *map);

/**
 * @brief Check if an Earth map data file exists and is valid
 * @param filepath Path to check
 * @return true if the file is a valid Earth map
 */
bool civ_earth_map_is_valid(const char *filepath);

/**
 * @brief Default path where the Earth map data is expected
 */
#define CIV_EARTH_MAP_DEFAULT_PATH "data/earth_2048x1024.earth"

#ifdef __cplusplus
}
#endif
#endif
