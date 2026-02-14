/**
 * @file map_generator.c
 * @brief Masterpiece Engine 17.1: Stabilized Reality.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "../../../include/common.h"
#include "../../../include/core/data/history_db.h"
#include "../../../include/core/world/map_generator.h"
#include "../../../include/utils/noise.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern civ_journal_t *g_journal;
float g_gen_progress = 0.0f; /* Global Source of Truth */

static civ_float_t clampf(civ_float_t v, civ_float_t min, civ_float_t max) {
  return v < min ? min : (v > max ? max : v);
}

civ_map_gen_params_t civ_map_default_params(void) {
  civ_map_gen_params_t p = {0};
  p.width = CIV_DEFAULT_MAP_WIDTH;
  p.height = CIV_DEFAULT_MAP_HEIGHT;
  p.sea_level = 0.35f;
  p.seed = (uint32_t)time(NULL);
  return p;
}

civ_result_t civ_map_genesis(civ_map_t *map) {
  if (!map)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null Map"};
  civ_map_gen_params_t p = civ_map_default_params();
  p.seed = map->seed;
  civ_journal_log(g_journal, CIV_JOURNAL_WORLD_GEN_START, "Engine 17.1 Genesis",
                  &map->seed, sizeof(uint32_t));
  return civ_map_generate(map, &p);
}

civ_result_t civ_map_generate(civ_map_t *map,
                              const civ_map_gen_params_t *params) {
  if (!map || !params)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null Params"};
  map->seed = params->seed;
  map->sea_level = 0.52f;
  map->land_tile_count = 0;
  map->river_tile_count = 0;

  /* MASTERPIECE ENGINE 18.5: FRACTAL REALITY (Realistic Continents) */
  for (int32_t y = 0; y < map->height; y++) {
    if ((y & 63) == 0) {
      g_gen_progress = (float)y / (float)map->height;
    }

    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
      if (!tile)
        continue;

      float fx = (float)x / (float)map->width;
      float fy = (float)y / (float)map->height;

      /* FBM Logic: Multi-layer noise for realism */
      float noise = 0.0f;
      float amplitude = 1.0f;
      float frequency = 1.8f;
      float persistence = 0.52f;
      float lacunarity = 2.05f;

      /* Base Continent Shape (6 octaves for detailed coastlines) */
      for (int i = 0; i < 6; i++) {
        noise += amplitude * (float)civ_noise_perlin(
                                 fx * frequency, fy * frequency, map->seed + i);
        amplitude *= persistence;
        frequency *= lacunarity;
      }

      /* Domain Warping (Realistic tectonic flow) */
      float qx = (float)civ_noise_perlin(fx * 4.0f + 0.0f, fy * 4.0f + 0.0f,
                                         map->seed + 10);
      float qy = (float)civ_noise_perlin(fx * 4.0f + 5.2f, fy * 4.0f + 1.3f,
                                         map->seed + 11);
      float warp = (float)civ_noise_perlin(
          fx * 4.0f + 4.0f * qx, fy * 4.0f + 4.0f * qy, map->seed + 12);

      float final_noise = noise * 0.5f + 0.5f;
      final_noise += warp * 0.15f; /* Add organic roughness */

      /* Continental Falloff (Masking the edge) */
      float d = sqrtf(powf(fx - 0.5f, 2) + powf(fy - 0.5f, 2)) * 2.0f;
      float mask =
          1.0f - powf(d, 4.0f); /* Sharper mask for island/continent feel */
      if (mask < 0)
        mask = 0;

      float elev = final_noise * mask;
      elev = powf(elev, 1.1f); /* Contrast boost */

      tile->elevation = clampf(elev, 0.001f, 0.999f);

      /* Temperature Modeling (Latitude based) */
      float lat_temp = 1.0f - fabsf(fy - 0.5f) * 2.0f;
      tile->temperature = clampf(
          lat_temp - (elev > 0.6f ? (elev - 0.6f) * 2.0f : 0.0f), 0.0f, 1.0f);

      /* Moisture Modeling (Perlin + Latitude) */
      tile->moisture =
          (float)civ_noise_perlin(fx * 3.0f, fy * 3.0f, map->seed + 20) * 0.5f +
          0.5f;
      tile->moisture *= (0.5f + lat_temp * 0.5f);

      /* Biome Logic */
      if (elev < map->sea_level) {
        tile->land_use = CIV_LAND_USE_WATER;
        tile->terrain = CIV_TERRAIN_PLAIN;
      } else {
        map->land_tile_count++;
        if (elev > 0.88f)
          tile->terrain = CIV_TERRAIN_MOUNTAIN;
        else if (elev > 0.75f)
          tile->terrain = CIV_TERRAIN_HILL;
        else
          tile->terrain = CIV_TERRAIN_PLAIN;

        if (tile->temperature < 0.2f) {
          tile->land_use = (tile->moisture < 0.3f) ? CIV_LAND_USE_TUNDRA
                                                   : CIV_LAND_USE_TUNDRA;
        } else if (tile->temperature > 0.7f && tile->moisture < 0.25f) {
          tile->land_use = CIV_LAND_USE_DESERT;
        } else if (tile->moisture > 0.7f) {
          tile->land_use = CIV_LAND_USE_FOREST;
        } else {
          tile->land_use = CIV_LAND_USE_GRASSLAND;
        }
      }
    }
  }

  /* Generate political influence */
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
      if (tile) {
        tile->political_influence =
            (float)civ_noise_perlin(x * 0.01f, y * 0.01f, map->seed + 20) *
                0.5f +
            0.5f;
      }
    }
  }

  /* Generate population density */
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
      if (tile) {
        tile->population_density =
            (float)civ_noise_perlin(x * 0.02f, y * 0.02f, map->seed + 30) *
                0.8f +
            0.2f;
      }
    }
  }

  /* Generate cultural influence */
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
      if (tile) {
        tile->cultural_influence =
            (float)civ_noise_perlin(x * 0.015f, y * 0.015f, map->seed + 40) *
                0.7f +
            0.3f;
      }
    }
  }

  /* Post-Processing: Hydrology & Ecology */
  civ_map_generate_rivers(map);
  civ_map_generate_resources(map);

  g_gen_progress = 1.0f;
  civ_journal_log(g_journal, CIV_JOURNAL_BIOME_FINALIZED,
                  "SATELLITE EVOLUTION COMPLETE", NULL, 0);
  return (civ_result_t){CIV_OK, "Genesis Perfected"};
}

civ_map_t *civ_map_create(int32_t width, int32_t height, uint32_t seed) {
  civ_map_t *m = malloc(sizeof(civ_map_t));
  if (m) {
    m->width = width;
    m->height = height;
    m->seed = seed;
    m->tiles = calloc((size_t)width * height, sizeof(civ_map_tile_t));
    if (!m->tiles) {
      free(m);
      return NULL;
    }
    for (int32_t y = 0; y < height; y++) {
      for (int32_t x = 0; x < width; x++) {
        civ_map_tile_t *t = &m->tiles[y * width + x];
        t->x = x;
        t->y = y;
      }
    }
  }
  return m;
}

void civ_map_destroy(civ_map_t *m) {
  if (m) {
    free(m->tiles);
    free(m);
  }
}
civ_map_tile_t *civ_map_get_tile(const civ_map_t *m, int32_t x, int32_t y) {
  if (!m || x < 0 || x >= m->width || y < 0 || y >= m->height)
    return NULL;
  return &m->tiles[y * m->width + x];
}
civ_result_t civ_map_calculate_stats(const civ_map_t *m, civ_map_stats_t *s) {
  if (!s)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};
  memset(s, 0, sizeof(*s));
  return (civ_result_t){CIV_OK, NULL};
}
bool civ_map_is_valid_position(const civ_map_t *m, int32_t x, int32_t y) {
  return m && x >= 0 && x < m->width && y >= 0 && y < m->height;
}
civ_map_tile_t *civ_map_get_adjacent_tile(const civ_map_t *m,
                                          const civ_map_tile_t *t, int32_t dx,
                                          int32_t dy) {
  if (!m || !t)
    return NULL;
  int32_t nx = (t->x + dx + m->width) % m->width;
  int32_t ny = t->y + dy;
  if (ny < 0 || ny >= m->height)
    return NULL;
  return civ_map_get_tile(m, nx, ny);
}
civ_result_t civ_map_validate(const civ_map_t *m) {
  return (m && m->tiles)
             ? (civ_result_t){CIV_OK, NULL}
             : (civ_result_t){CIV_ERROR_INVALID_STATE, "Broken Map"};
}
civ_result_t civ_map_simulate_erosion(civ_map_t *m, int i) {
  return (civ_result_t){CIV_OK, NULL};
}
civ_result_t civ_map_generate_rivers(civ_map_t *m) {
  if (!m)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null Map"};

  /* River Genesis: Start from high elevations (>0.75) and flow down */
  int river_count = 0;
  for (int32_t i = 0;
       i < (m->width * m->height) && river_count < CIV_MAX_RIVERS_PER_MAP;
       i++) {
    civ_map_tile_t *start = &m->tiles[rand() % (m->width * m->height)];
    if (start->elevation > 0.75f && !start->has_river) {
      civ_map_tile_t *curr = start;
      int len = 0;
      while (curr && len < CIV_MAX_RIVER_LENGTH) {
        curr->has_river = true;
        len++;
        if (curr->elevation < m->sea_level)
          break; /* Reached sea */

        /* Flow to lowest neighbor */
        civ_map_tile_t *lowest = NULL;
        float min_e = curr->elevation;
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0)
              continue;
            civ_map_tile_t *adj = civ_map_get_adjacent_tile(m, curr, dx, dy);
            if (adj && adj->elevation < min_e) {
              min_e = adj->elevation;
              lowest = adj;
            }
          }
        }
        if (!lowest)
          break; /* Puddle or local minimum */
        curr = lowest;
      }
      river_count++;
    }
  }
  return (civ_result_t){CIV_OK, "Rivers Flowing"};
}

civ_result_t civ_map_generate_resources(civ_map_t *m) {
  if (!m)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null Map"};
  for (int32_t i = 0; i < (m->width * m->height); i++) {
    civ_map_tile_t *t = &m->tiles[i];
    if (t->elevation >= m->sea_level) {
      /* 5% chance of resource placement */
      if ((rand() % 100) < 5) {
        t->has_resource = true;
        t->resources = 0.5f + (float)(rand() % 50) / 100.0f;
      }
    }
  }
  return (civ_result_t){CIV_OK, "Resources Scattered"};
}
civ_result_t civ_map_generate_terrain(civ_map_t *m) {
  return civ_map_genesis(m);
}
civ_result_t civ_map_generate_premium(civ_map_t *m,
                                      const civ_map_gen_params_t *p) {
  return civ_map_generate(m, p);
}
civ_float_t civ_map_tile_distance(const civ_map_tile_t *a,
                                  const civ_map_tile_t *b) {
  return 0.0f;
}
int32_t civ_map_count_terrain(const civ_map_t *m, civ_terrain_type_t t) {
  return 0;
}
int32_t civ_map_count_land_use(const civ_map_t *m, civ_land_use_type_t l) {
  return 0;
}
bool civ_map_init(civ_map_t *m, int32_t w, int32_t h, uint32_t s) {
  return false;
}
civ_result_t
civ_map_generate_terrain_with_params(civ_map_t *m,
                                     const civ_map_gen_params_t *p) {
  return civ_map_generate(m, p);
}