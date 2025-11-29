/**
 * @file map_generator.c
 * @brief Implementation of 2D map generation using Perlin noise and biomes
 */

#include "../../../include/core/world/map_generator.h"
#include "../../../include/common.h"
#include "../../../include/utils/noise.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* -------------------------------------------------------------------------- */
/* Internal Constants & Types ------------------------------------------------
 */

#define MIN_MAP_WIDTH 16
#define MIN_MAP_HEIGHT 16
#define MAX_MAP_WIDTH 4096
#define MAX_MAP_HEIGHT 4096
#define MAX_RIVER_ATTEMPTS 50
#define MAX_RIVER_LENGTH 200
#define SMOOTHING_ITERATIONS 2
#define DEFAULT_NOISE_SCALE 0.02f
#define DEFAULT_NOISE_OCTAVES 6

typedef struct {
  int32_t x, y;
} civ_point_t;

/* -------------------------------------------------------------------------- */
/* Internal Helpers ----------------------------------------------------------
 */

static civ_float_t map_rng_float(uint32_t *seed) {
  *seed = *seed * 1103515245 + 12345;
  return (civ_float_t)((*seed >> 16) & 0x7FFF) / 32768.0f;
}

static int32_t map_rng_int(uint32_t *seed, int32_t min, int32_t max) {
  return min + (int32_t)(map_rng_float(seed) * (max - min));
}

static civ_float_t clamp(civ_float_t value, civ_float_t min, civ_float_t max) {
  return value < min ? min : (value > max ? max : value);
}

static civ_float_t smoothstep(civ_float_t edge0, civ_float_t edge1,
                              civ_float_t x) {
  x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return x * x * (3.0f - 2.0f * x);
}

static civ_float_t lerp(civ_float_t a, civ_float_t b, civ_float_t t) {
  return a + t * (b - a);
}

static civ_map_tile_t *get_tile_safe(const civ_map_t *map, int32_t x,
                                     int32_t y) {
  if (x < 0 || x >= map->width || y < 0 || y >= map->height)
    return NULL;
  return &map->tiles[y * map->width + x];
}

static civ_bool_t is_valid_river_source(const civ_map_tile_t *tile,
                                        civ_float_t min_elevation) {
  return tile && tile->elevation > min_elevation && tile->moisture > 0.4f &&
         tile->land_use != CIV_LAND_USE_WATER;
}

static civ_float_t calculate_distance(int32_t x1, int32_t y1, int32_t x2,
                                      int32_t y2) {
  civ_float_t dx = (civ_float_t)(x1 - x2);
  civ_float_t dy = (civ_float_t)(y1 - y2);
  return sqrt(dx * dx + dy * dy);
}

/* -------------------------------------------------------------------------- */
/* Map Creation & Destruction ------------------------------------------------
 */

civ_map_t *civ_map_create(int32_t width, int32_t height, uint32_t seed) {
  if (width < MIN_MAP_WIDTH || height < MIN_MAP_HEIGHT) {
    civ_log(CIV_LOG_ERROR, "Map dimensions too small: %dx%d", width, height);
    return NULL;
  }

  if (width > MAX_MAP_WIDTH || height > MAX_MAP_HEIGHT) {
    civ_log(CIV_LOG_ERROR, "Map dimensions too large: %dx%d", width, height);
    return NULL;
  }

  civ_map_t *map = (civ_map_t *)CIV_MALLOC(sizeof(civ_map_t));
  if (!map) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate map structure");
    return NULL;
  }

  if (!civ_map_init(map, width, height, seed)) {
    CIV_FREE(map);
    return NULL;
  }

  return map;
}

void civ_map_destroy(civ_map_t *map) {
  if (!map)
    return;
  CIV_FREE(map->tiles);
  CIV_FREE(map);
}

bool civ_map_init(civ_map_t *map, int32_t width, int32_t height,
                  uint32_t seed) {
  if (!map)
    return false;

  memset(map, 0, sizeof(civ_map_t));
  map->width = (int32_t)clamp((civ_float_t)width, (civ_float_t)MIN_MAP_WIDTH,
                              (civ_float_t)MAX_MAP_WIDTH);
  map->height = (int32_t)clamp((civ_float_t)height, (civ_float_t)MIN_MAP_HEIGHT,
                               (civ_float_t)MAX_MAP_HEIGHT);
  map->sea_level = CIV_DEFAULT_SEA_LEVEL;
  map->seed = seed;

  size_t tile_count = (size_t)map->width * (size_t)map->height;
  if (tile_count > SIZE_MAX / sizeof(civ_map_tile_t)) {
    civ_log(CIV_LOG_ERROR, "Map too large: %zux%zu tiles", (size_t)map->width,
            (size_t)map->height);
    return false;
  }

  map->tiles = (civ_map_tile_t *)CIV_CALLOC(tile_count, sizeof(civ_map_tile_t));
  if (!map->tiles) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate %zu map tiles", tile_count);
    return false;
  }

  // Initialize tiles with default values
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = &map->tiles[y * map->width + x];
      tile->x = x;
      tile->y = y;
      tile->terrain = CIV_TERRAIN_PLAIN;
      tile->land_use = CIV_LAND_USE_GRASSLAND;
      tile->vegetation_density = 0.0f;
      tile->fertility = 0.0f;
      tile->resources = 0.0f;
      tile->has_river = false;
      tile->has_resource = false;
    }
  }

  return true;
}

civ_map_gen_params_t civ_map_default_params(void) {
  return (civ_map_gen_params_t){.width = CIV_DEFAULT_MAP_WIDTH,
                                .height = CIV_DEFAULT_MAP_HEIGHT,
                                .sea_level = CIV_DEFAULT_SEA_LEVEL,
                                .land_ratio = CIV_DEFAULT_LAND_RATIO,
                                .mountain_ratio = 0.15f,
                                .generate_rivers = true,
                                .generate_mountains = true,
                                .generate_resources = true,
                                .smooth_terrain = true,
                                .seed = (uint32_t)time(NULL),
                                .noise_scale = DEFAULT_NOISE_SCALE,
                                .noise_octaves = DEFAULT_NOISE_OCTAVES};
}

/* -------------------------------------------------------------------------- */
/* Tile Access & Queries -----------------------------------------------------
 */

civ_map_tile_t *civ_map_get_tile(const civ_map_t *map, int32_t x, int32_t y) {
  return get_tile_safe(map, x, y);
}

bool civ_map_is_valid_position(const civ_map_t *map, int32_t x, int32_t y) {
  return x >= 0 && x < map->width && y >= 0 && y < map->height;
}

civ_map_tile_t *civ_map_get_adjacent_tile(const civ_map_t *map,
                                          const civ_map_tile_t *tile,
                                          int32_t dx, int32_t dy) {
  if (!map || !tile)
    return NULL;
  return get_tile_safe(map, tile->x + dx, tile->y + dy);
}

civ_float_t civ_map_tile_distance(const civ_map_tile_t *a,
                                  const civ_map_tile_t *b) {
  if (!a || !b)
    return 0.0f;
  return calculate_distance(a->x, a->y, b->x, b->y);
}

/* -------------------------------------------------------------------------- */
/* Terrain Generation --------------------------------------------------------
 */

static void generate_elevation(civ_map_t *map,
                               const civ_map_gen_params_t *params) {
  const int octaves = params->noise_octaves;
  const civ_float_t persistence = 0.5f;
  const civ_float_t scale = params->noise_scale;

  // Generate base elevation with multiple octaves
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);

      // Base elevation from Perlin noise
      civ_float_t elevation =
          civ_noise_octave((civ_float_t)x, (civ_float_t)y, octaves, persistence,
                           scale, map->seed);

      // Apply curve to create more distinct land masses
      elevation = pow(elevation, 1.5);

      tile->elevation = clamp(elevation, 0.0f, 1.0f);
    }
  }
}

static void generate_moisture(civ_map_t *map) {
  const civ_float_t scale = 0.03f;

  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);

      // Base moisture from noise
      civ_float_t moisture = civ_noise_octave((civ_float_t)x, (civ_float_t)y, 4,
                                              0.5f, scale, map->seed + 12345);

      // Reduce moisture near coasts
      if (tile->elevation < map->sea_level + 0.1f) {
        moisture *= 0.7f;
      }

      tile->moisture = clamp(moisture, 0.0f, 1.0f);
    }
  }
}

static void generate_temperature(civ_map_t *map) {
  const civ_float_t equator_y = map->height / 2.0f;
  const civ_float_t latitude_effect = 1.2f; // Strength of latitude effect

  for (int32_t y = 0; y < map->height; y++) {
    // Latitude effect: colder near poles, hotter near equator
    civ_float_t pole_factor = fabs((civ_float_t)y - equator_y) / equator_y;
    civ_float_t base_temp = 1.0f - pow(pole_factor, latitude_effect);

    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);

      // Temperature noise for variation
      civ_float_t temp_noise =
          civ_noise_perlin(x * 0.03f, y * 0.03f, map->seed + 54321);

      // Elevation effect: higher = colder
      civ_float_t elevation_effect =
          tile->elevation > map->sea_level
              ? (tile->elevation - map->sea_level) * 0.3f
              : 0.0f;

      tile->temperature = base_temp + (temp_noise * 0.15f) - elevation_effect;
      tile->temperature = clamp(tile->temperature, 0.0f, 1.0f);
    }
  }
}

static void assign_terrain_type(civ_map_t *map, civ_map_tile_t *tile,
                                const civ_map_gen_params_t *params) {
  const civ_float_t elevation = tile->elevation;

  if (elevation < map->sea_level) {
    tile->terrain = CIV_TERRAIN_PLAIN; // Ocean floor
  } else if (elevation > 0.85f && params->generate_mountains) {
    tile->terrain = CIV_TERRAIN_MOUNTAIN;
    map->mountain_tile_count++;
  } else if (elevation > 0.70f) {
    tile->terrain = CIV_TERRAIN_HILL;
  } else if (elevation < map->sea_level + 0.05f) {
    tile->terrain = CIV_TERRAIN_COASTAL;
  } else {
    tile->terrain = CIV_TERRAIN_PLAIN;
  }

  if (tile->elevation >= map->sea_level) {
    map->land_tile_count++;
  }
}

static void assign_biome(civ_map_t *map, civ_map_tile_t *tile) {
  const civ_float_t temp = tile->temperature;
  const civ_float_t moisture = tile->moisture;

  // Enhanced biome determination with smoother transitions
  if (temp < 0.15f) {
    // Polar/Subpolar - very cold
    if (moisture < 0.3f) {
      tile->land_use = CIV_LAND_USE_TUNDRA;
      tile->vegetation_density = 0.05f;
    } else {
      tile->land_use = CIV_LAND_USE_TUNDRA;
      tile->vegetation_density = 0.15f;
    }
  } else if (temp < 0.35f) {
    // Cold temperate
    if (moisture < 0.25f) {
      tile->land_use = CIV_LAND_USE_GRASSLAND; // Cold desert/steppe
      tile->vegetation_density = 0.1f;
    } else if (moisture < 0.6f) {
      tile->land_use = CIV_LAND_USE_GRASSLAND;
      tile->vegetation_density = 0.4f;
    } else {
      tile->land_use = CIV_LAND_USE_FOREST; // Boreal forest
      tile->vegetation_density = 0.7f;
    }
  } else if (temp < 0.65f) {
    // Temperate
    if (moisture < 0.25f) {
      tile->land_use = CIV_LAND_USE_GRASSLAND; // Steppe
      tile->vegetation_density = 0.2f;
    } else if (moisture < 0.5f) {
      tile->land_use = CIV_LAND_USE_GRASSLAND;
      tile->vegetation_density = 0.5f;
    } else {
      tile->land_use = CIV_LAND_USE_FOREST; // Temperate forest
      tile->vegetation_density = 0.8f;
    }
  } else {
    // Hot/Tropical
    if (moisture < 0.2f) {
      tile->land_use = CIV_LAND_USE_DESERT;
      tile->vegetation_density = 0.05f;
    } else if (moisture < 0.45f) {
      tile->land_use = CIV_LAND_USE_GRASSLAND; // Savanna
      tile->vegetation_density = 0.4f;
    } else if (moisture < 0.75f) {
      tile->land_use = CIV_LAND_USE_FOREST; // Tropical forest
      tile->vegetation_density = 0.8f;
    } else {
      tile->land_use = CIV_LAND_USE_FOREST; // Rainforest
      tile->vegetation_density = 0.95f;
    }
  }
}

static void calculate_fertility(civ_map_t *map, civ_map_tile_t *tile) {
  // Base fertility from environmental factors
  civ_float_t base_fertility = tile->moisture * 0.6f + tile->temperature * 0.2f;

  // Terrain modifiers
  switch (tile->terrain) {
  case CIV_TERRAIN_MOUNTAIN:
    base_fertility *= 0.1f;
    break;
  case CIV_TERRAIN_HILL:
    base_fertility *= 0.7f;
    break;
  case CIV_TERRAIN_COASTAL:
    base_fertility *= 1.2f; // Coastal areas can be fertile
    break;
  default:
    break;
  }

  // Biome-specific adjustments
  switch (tile->land_use) {
  case CIV_LAND_USE_DESERT:
  case CIV_LAND_USE_TUNDRA:
    base_fertility *= 0.2f;
    break;
  case CIV_LAND_USE_FOREST:
    base_fertility *= 1.3f;
    break;
  case CIV_LAND_USE_WETLAND:
    base_fertility *= 1.5f;
    break;
  default:
    break;
  }

  tile->fertility = clamp(base_fertility, 0.0f, 1.0f);
}

civ_result_t civ_map_smooth_terrain(civ_map_t *map, int32_t iterations) {
  if (!map)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  civ_float_t *temp_elevation = (civ_float_t *)CIV_MALLOC(
      (size_t)(map->width * map->height) * sizeof(civ_float_t));
  if (!temp_elevation) {
    return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY,
                          "Failed to allocate smoothing buffer"};
  }

  for (int32_t iter = 0; iter < iterations; iter++) {
    // Copy current elevations to temp buffer
    for (int32_t i = 0; i < map->width * map->height; i++) {
      temp_elevation[i] = map->tiles[i].elevation;
    }

    // Apply smoothing kernel
    for (int32_t y = 1; y < map->height - 1; y++) {
      for (int32_t x = 1; x < map->width - 1; x++) {
        civ_float_t sum = 0.0f;
        int32_t count = 0;

        // 3x3 smoothing kernel
        for (int32_t dy = -1; dy <= 1; dy++) {
          for (int32_t dx = -1; dx <= 1; dx++) {
            civ_map_tile_t *neighbor = get_tile_safe(map, x + dx, y + dy);
            if (neighbor) {
              sum += temp_elevation[(y + dy) * map->width + (x + dx)];
              count++;
            }
          }
        }

        civ_map_tile_t *tile = get_tile_safe(map, x, y);
        if (tile && count > 0) {
          tile->elevation = sum / count;
        }
      }
    }
  }

  CIV_FREE(temp_elevation);
  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_map_generate_terrain(civ_map_t *map) {
  if (!map)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  civ_map_gen_params_t default_params = civ_map_default_params();
  return civ_map_generate_terrain_with_params(map, &default_params);
}

civ_result_t
civ_map_generate_terrain_with_params(civ_map_t *map,
                                     const civ_map_gen_params_t *params) {
  if (!map || !params)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  // Reset counters
  map->land_tile_count = 0;
  map->mountain_tile_count = 0;
  map->river_tile_count = 0;

  generate_elevation(map, params);
  generate_moisture(map);
  generate_temperature(map);

  // Apply smoothing if requested
  if (params->smooth_terrain) {
    civ_map_smooth_terrain(map, SMOOTHING_ITERATIONS);
  }

  // Assign terrain types and biomes
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);

      if (tile->elevation < map->sea_level) {
        // Ocean
        tile->terrain = CIV_TERRAIN_PLAIN;
        tile->land_use = CIV_LAND_USE_WATER;
        tile->vegetation_density = 0.0f;
        tile->fertility = 0.0f;
        tile->has_river = false;
        continue;
      }

      assign_terrain_type(map, tile, params);
      assign_biome(map, tile);
      calculate_fertility(map, tile);
    }
  }

  return (civ_result_t){CIV_OK, NULL};
}

/* -------------------------------------------------------------------------- */
/* River Generation ----------------------------------------------------------
 */

static bool find_river_source(const civ_map_t *map, uint32_t *seed,
                              civ_point_t *source) {
  for (int32_t attempt = 0; attempt < MAX_RIVER_ATTEMPTS; attempt++) {
    source->x = (int32_t)(map_rng_float(seed) * map->width);
    source->y = (int32_t)(map_rng_float(seed) * map->height);

    civ_map_tile_t *tile = get_tile_safe(map, source->x, source->y);
    if (is_valid_river_source(tile, 0.6f)) {
      return true;
    }
  }
  return false;
}

static void flow_river(civ_map_t *map, civ_point_t start) {
  civ_point_t current = start;
  civ_map_tile_t *curr_tile = get_tile_safe(map, current.x, current.y);

  if (!curr_tile)
    return;

  for (int32_t step = 0; step < MAX_RIVER_LENGTH; step++) {
    if (!curr_tile || curr_tile->elevation < map->sea_level) {
      break; // Reached ocean
    }

    // Mark as river
    if (!curr_tile->has_river) {
      curr_tile->has_river = true;
      curr_tile->land_use = CIV_LAND_USE_WETLAND;
      curr_tile->fertility = clamp(curr_tile->fertility + 0.3f, 0.0f, 1.0f);
      map->river_tile_count++;
    }

    // Find lowest neighbor (prefer cardinal directions)
    civ_float_t min_elev = curr_tile->elevation;
    civ_point_t next = current;
    static const civ_point_t directions[] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

    for (int i = 0; i < 4; i++) {
      civ_point_t test = {current.x + directions[i].x,
                          current.y + directions[i].y};
      civ_map_tile_t *neighbor = get_tile_safe(map, test.x, test.y);

      if (neighbor && neighbor->elevation < min_elev) {
        min_elev = neighbor->elevation;
        next = test;
      }
    }

    // Stop if no downhill path found
    if (next.x == current.x && next.y == current.y)
      break;

    current = next;
    curr_tile = get_tile_safe(map, current.x, current.y);
  }
}

civ_result_t civ_map_generate_rivers(civ_map_t *map) {
  if (!map)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  uint32_t rng_seed = map->seed;
  int32_t river_count = 10 + (int32_t)(map_rng_float(&rng_seed) * 10);
  river_count = (int32_t)clamp((civ_float_t)river_count, 1.0f,
                               (civ_float_t)CIV_MAX_RIVERS_PER_MAP);

  int32_t rivers_placed = 0;
  for (int32_t i = 0; i < river_count; i++) {
    civ_point_t source;
    if (find_river_source(map, &rng_seed, &source)) {
      flow_river(map, source);
      rivers_placed++;
    }
  }

  civ_log(CIV_LOG_INFO, "Generated %d rivers with %d river tiles",
          rivers_placed, map->river_tile_count);

  return (civ_result_t){CIV_OK, NULL};
}

/* -------------------------------------------------------------------------- */
/* Resource Generation -------------------------------------------------------
 */

civ_result_t civ_map_generate_resources(civ_map_t *map) {
  if (!map)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  uint32_t rng_seed = map->seed + 999;
  int32_t resources_placed = 0;

  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);
      if (!tile || tile->elevation < map->sea_level)
        continue;

      // Base resource chance
      civ_float_t chance = 0.05f;

      // Increase chance based on terrain and biome
      if (tile->land_use == CIV_LAND_USE_FOREST)
        chance = 0.15f;
      if (tile->terrain == CIV_TERRAIN_HILL)
        chance = 0.20f;
      if (tile->terrain == CIV_TERRAIN_MOUNTAIN)
        chance = 0.25f;

      if (map_rng_float(&rng_seed) < chance) {
        tile->resources = 0.5f + map_rng_float(&rng_seed) * 0.5f;
        tile->has_resource = true;
        resources_placed++;
      }
    }
  }

  civ_log(CIV_LOG_INFO, "Placed %d resource deposits", resources_placed);

  return (civ_result_t){CIV_OK, NULL};
}

/* -------------------------------------------------------------------------- */
/* Main Generation Interface -------------------------------------------------
 */

civ_result_t civ_map_generate(civ_map_t *map,
                              const civ_map_gen_params_t *params) {
  if (!map || !params)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  // Update map parameters
  map->seed = params->seed;
  if (params->sea_level > 0) {
    map->sea_level = clamp(params->sea_level, 0.1f, 0.9f);
  }

  // Generate terrain
  CIV_RESULT_CHECK(civ_map_generate_terrain_with_params(map, params));

  // Generate rivers if enabled
  if (params->generate_rivers) {
    CIV_RESULT_CHECK(civ_map_generate_rivers(map));
  }

  // Generate resources if enabled
  if (params->generate_resources) {
    CIV_RESULT_CHECK(civ_map_generate_resources(map));
  }

  civ_log(CIV_LOG_INFO,
          "Map generation complete: %dx%d, %d land tiles, %d mountains",
          map->width, map->height, map->land_tile_count,
          map->mountain_tile_count);

  return (civ_result_t){CIV_OK, NULL};
}

/* -------------------------------------------------------------------------- */
/* Map Analysis & Statistics -------------------------------------------------
 */

civ_result_t civ_map_calculate_stats(const civ_map_t *map,
                                     civ_map_stats_t *stats) {
  if (!map || !stats)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  memset(stats, 0, sizeof(civ_map_stats_t));

  stats->total_tiles = map->width * map->height;
  stats->land_tiles = map->land_tile_count;
  stats->water_tiles = stats->total_tiles - stats->land_tiles;
  stats->river_tiles = map->river_tile_count;
  stats->mountain_tiles = map->mountain_tile_count;

  civ_float_t temp_sum = 0.0f;
  civ_float_t moisture_sum = 0.0f;
  civ_float_t fertility_sum = 0.0f;

  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);
      if (!tile)
        continue;

      temp_sum += tile->temperature;
      moisture_sum += tile->moisture;
      fertility_sum += tile->fertility;

      switch (tile->land_use) {
      case CIV_LAND_USE_FOREST:
        stats->forest_tiles++;
        break;
      case CIV_LAND_USE_DESERT:
        stats->desert_tiles++;
        break;
      default:
        break;
      }
    }
  }

  stats->avg_temperature = temp_sum / stats->total_tiles;
  stats->avg_moisture = moisture_sum / stats->total_tiles;
  stats->avg_fertility = fertility_sum / stats->total_tiles;

  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_map_validate(const civ_map_t *map) {
  if (!map)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};
  if (!map->tiles)
    return (civ_result_t){CIV_ERROR_INVALID_DATA, "Map tiles are NULL"};
  if (map->width < MIN_MAP_WIDTH || map->height < MIN_MAP_HEIGHT) {
    return (civ_result_t){CIV_ERROR_INVALID_DATA, "Map dimensions too small"};
  }
  if (map->width > MAX_MAP_WIDTH || map->height > MAX_MAP_HEIGHT) {
    return (civ_result_t){CIV_ERROR_INVALID_DATA, "Map dimensions too large"};
  }

  // Validate tile data ranges
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);
      if (!tile)
        continue;

      if (tile->x != x || tile->y != y) {
        return (civ_result_t){CIV_ERROR_INVALID_DATA, "Tile position mismatch"};
      }

      if (tile->elevation < 0.0f || tile->elevation > 1.0f ||
          tile->moisture < 0.0f || tile->moisture > 1.0f ||
          tile->temperature < 0.0f || tile->temperature > 1.0f) {
        return (civ_result_t){CIV_ERROR_INVALID_DATA,
                              "Tile value out of range"};
      }
    }
  }

  return (civ_result_t){CIV_OK, NULL};
}

int32_t civ_map_count_terrain(const civ_map_t *map,
                              civ_terrain_type_t terrain) {
  if (!map)
    return 0;

  int32_t count = 0;
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);
      if (tile && tile->terrain == terrain) {
        count++;
      }
    }
  }
  return count;
}

int32_t civ_map_count_land_use(const civ_map_t *map,
                               civ_land_use_type_t land_use) {
  if (!map)
    return 0;

  int32_t count = 0;
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = get_tile_safe(map, x, y);
      if (tile && tile->land_use == land_use) {
        count++;
      }
    }
  }
  return count;
}