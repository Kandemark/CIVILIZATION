/**
 * @file map_generator.h
 * @brief 2D world map generation system using procedural content generation
 */

#ifndef CIVILIZATION_MAP_GENERATOR_H
#define CIVILIZATION_MAP_GENERATOR_H

#include "../../common.h"
#include "../../types.h"
#include "../environment/geography.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Constants & Configuration -------------------------------------------------
 */

/** Minimum allowed map dimensions */
#define CIV_MIN_MAP_WIDTH 16
#define CIV_MIN_MAP_HEIGHT 16
#define CIV_MAX_MAP_WIDTH 4096
#define CIV_MAX_MAP_HEIGHT 4096

/** Default generation values */
#define CIV_DEFAULT_SEA_LEVEL 0.35f
#define CIV_DEFAULT_LAND_RATIO 0.65f
#define CIV_DEFAULT_MAP_WIDTH 100
#define CIV_DEFAULT_MAP_HEIGHT 100

/** River generation limits */
#define CIV_MAX_RIVERS_PER_MAP 50
#define CIV_MAX_RIVER_LENGTH 500

/* ========================================================================== */
/* Core Data Types -----------------------------------------------------------
 */

/**
 * @brief Represents a single tile on the map with all environmental properties
 */
typedef struct {
  /* Position */
  int32_t x;
  int32_t y;

  /* Core environmental properties */
  civ_float_t elevation;   /**< 0.0 (deep ocean) to 1.0 (highest peak) */
  civ_float_t moisture;    /**< 0.0 (arid) to 1.0 (saturated) */
  civ_float_t temperature; /**< 0.0 (cold) to 1.0 (hot) */

  /* Terrain classification */
  civ_terrain_type_t terrain;   /**< Physical terrain type */
  civ_land_use_type_t land_use; /**< Biome/land use classification */

  /* Resource and fertility */
  civ_float_t vegetation_density; /**< 0.0 (barren) to 1.0 (dense) */
  civ_float_t fertility; /**< 0.0 (infertile) to 1.0 (highly fertile) */
  civ_float_t resources; /**< 0.0 (none) to 1.0 (abundant) */

  /* Flags */
  bool has_river;    /**< True if tile contains a river */
  bool has_resource; /**< True if tile has natural resources */

} civ_map_tile_t;

/**
 * @brief Complete 2D map containing all tiles and generation metadata
 */
typedef struct {
  civ_map_tile_t *tiles; /**< 1D array of tiles in row-major order */
  int32_t width;         /**< Map width in tiles */
  int32_t height;        /**< Map height in tiles */

  /* Generation parameters */
  civ_float_t sea_level; /**< Elevation threshold for ocean/land */
  uint32_t seed;         /**< Seed used for generation */

  /* Statistics */
  int32_t land_tile_count;     /**< Number of land tiles */
  int32_t river_tile_count;    /**< Number of river tiles */
  int32_t mountain_tile_count; /**< Number of mountain tiles */

} civ_map_t;

/**
 * @brief Parameters controlling map generation behavior
 */
typedef struct {
  /* Dimensions */
  int32_t width;
  int32_t height;

  /* Environmental parameters */
  civ_float_t sea_level;  /**< 0.1 to 0.9, default 0.35 */
  civ_float_t land_ratio; /**< 0.0 to 1.0, target land percentage */
  civ_float_t
      mountain_ratio; /**< 0.0 to 0.3, percentage of land as mountains */

  /* Generation flags */
  bool generate_rivers;    /**< Enable river generation */
  bool generate_mountains; /**< Enable mountain generation */
  bool generate_resources; /**< Enable resource placement */
  bool smooth_terrain;     /**< Apply terrain smoothing */

  /* Advanced parameters */
  uint32_t seed;           /**< Random seed (0 for random) */
  civ_float_t noise_scale; /**< Noise frequency scaling */
  int32_t noise_octaves;   /**< Number of noise octaves */

} civ_map_gen_params_t;

/**
 * @brief Statistics about generated map content
 */
typedef struct {
  int32_t total_tiles;
  int32_t land_tiles;
  int32_t water_tiles;
  int32_t river_tiles;
  int32_t mountain_tiles;
  int32_t forest_tiles;
  int32_t desert_tiles;

  civ_float_t avg_temperature;
  civ_float_t avg_moisture;
  civ_float_t avg_fertility;

} civ_map_stats_t;

/* ========================================================================== */
/* Core Map Management -------------------------------------------------------
 */

/**
 * @brief Create a new map with specified dimensions
 * @param width Map width in tiles (min CIV_MIN_MAP_WIDTH)
 * @param height Map height in tiles (min CIV_MIN_MAP_HEIGHT)
 * @param seed Random seed for generation
 * @return New map instance or NULL on failure
 */
civ_map_t *civ_map_create(int32_t width, int32_t height, uint32_t seed);

/**
 * @brief Destroy a map and free all resources
 * @param map Map to destroy
 */
void civ_map_destroy(civ_map_t *map);

/**
 * @brief Initialize an existing map structure
 * @param map Map to initialize
 * @param width Map width in tiles
 * @param height Map height in tiles
 * @param seed Random seed
 * @return true if initialization successful
 */
bool civ_map_init(civ_map_t *map, int32_t width, int32_t height, uint32_t seed);

/**
 * @brief Get default generation parameters
 * @return Default parameters structure
 */
civ_map_gen_params_t civ_map_default_params(void);

/* ========================================================================== */
/* Tile Access & Queries -----------------------------------------------------
 */

/**
 * @brief Get tile at specified coordinates
 * @param map Map instance
 * @param x X coordinate (0 to width-1)
 * @param y Y coordinate (0 to height-1)
 * @return Tile pointer or NULL if coordinates are invalid
 */
civ_map_tile_t *civ_map_get_tile(const civ_map_t *map, int32_t x, int32_t y);

/**
 * @brief Check if coordinates are within map bounds
 * @param map Map instance
 * @param x X coordinate
 * @param y Y coordinate
 * @return true if coordinates are valid
 */
bool civ_map_is_valid_position(const civ_map_t *map, int32_t x, int32_t y);

/**
 * @brief Get adjacent tile in specified direction
 * @param map Map instance
 * @param tile Source tile
 * @param dx X direction (-1, 0, 1)
 * @param dy Y direction (-1, 0, 1)
 * @return Adjacent tile or NULL if out of bounds
 */
civ_map_tile_t *civ_map_get_adjacent_tile(const civ_map_t *map,
                                          const civ_map_tile_t *tile,
                                          int32_t dx, int32_t dy);

/**
 * @brief Calculate distance between two tiles
 * @param a First tile
 * @param b Second tile
 * @return Euclidean distance
 */
civ_float_t civ_map_tile_distance(const civ_map_tile_t *a,
                                  const civ_map_tile_t *b);

/* ========================================================================== */
/* Map Generation ------------------------------------------------------------
 */

/**
 * @brief Generate complete map using specified parameters
 * @param map Map to generate (must be initialized)
 * @param params Generation parameters
 * @return Result indicating success or failure
 */
civ_result_t civ_map_generate(civ_map_t *map,
                              const civ_map_gen_params_t *params);

/**
 * @brief Generate base terrain (elevation, moisture, temperature)
 * @param map Map to generate terrain for
 * @return Result indicating success or failure
 */
civ_result_t civ_map_generate_terrain(civ_map_t *map);

/**
 * @brief Generate base terrain with specific parameters
 * @param map Map to generate terrain for
 * @param params Generation parameters
 * @return Result indicating success or failure
 */
civ_result_t
civ_map_generate_terrain_with_params(civ_map_t *map,
                                     const civ_map_gen_params_t *params);

/**
 * @brief Generate rivers flowing from mountains to oceans
 * @param map Map to generate rivers for
 * @return Result indicating success or failure
 */
civ_result_t civ_map_generate_rivers(civ_map_t *map);

/**
 * @brief Generate natural resources based on terrain and biomes
 * @param map Map to generate resources for
 * @return Result indicating success or failure
 */
civ_result_t civ_map_generate_resources(civ_map_t *map);

/**
 * @brief Apply smoothing to terrain for more natural appearance
 * @param map Map to smooth
 * @param iterations Number of smoothing passes
 * @return Result indicating success or failure
 */
civ_result_t civ_map_smooth_terrain(civ_map_t *map, int32_t iterations);

/* ========================================================================== */
/* Map Analysis & Statistics -------------------------------------------------
 */

/**
 * @brief Calculate statistics about map content
 * @param map Map to analyze
 * @param stats Output statistics structure
 * @return Result indicating success or failure
 */
civ_result_t civ_map_calculate_stats(const civ_map_t *map,
                                     civ_map_stats_t *stats);

/**
 * @brief Validate map consistency and data integrity
 * @param map Map to validate
 * @return Result indicating validation result
 */
civ_result_t civ_map_validate(const civ_map_t *map);

/**
 * @brief Count tiles of specific terrain type
 * @param map Map to search
 * @param terrain Terrain type to count
 * @return Number of matching tiles
 */
int32_t civ_map_count_terrain(const civ_map_t *map, civ_terrain_type_t terrain);

/**
 * @brief Count tiles of specific land use type
 * @param map Map to search
 * @param land_use Land use type to count
 * @return Number of matching tiles
 */
int32_t civ_map_count_land_use(const civ_map_t *map,
                               civ_land_use_type_t land_use);

#ifdef __cplusplus
}
#endif

#endif /* CIVILIZATION_MAP_GENERATOR_H */