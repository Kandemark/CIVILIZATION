/**
 * @file map_generator.c
 * @brief Deterministic atlas-style global map generation.
 */

#include "../../../include/common.h"
#include "../../../include/core/data/history_db.h"
#include "../../../include/core/world/map_generator.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

extern civ_journal_t *g_journal;
float g_gen_progress = 0.0f; /* Global Source of Truth */

static civ_float_t clampf(civ_float_t v, civ_float_t min, civ_float_t max) {
  return v < min ? min : (v > max ? max : v);
}

static civ_float_t atlas_shape_signal(civ_float_t nx, civ_float_t ny) {
  /* Deterministic pseudo-continents (non-earthlike) */
  civ_float_t wave_a = sinf(nx * 6.2831853f * 1.7f) * 0.22f;
  civ_float_t wave_b = cosf(ny * 6.2831853f * 2.3f) * 0.18f;

  civ_float_t c1 = expf(-((nx - 0.28f) * (nx - 0.28f) * 18.0f +
                          (ny - 0.58f) * (ny - 0.58f) * 26.0f));
  civ_float_t c2 = expf(-((nx - 0.72f) * (nx - 0.72f) * 22.0f +
                          (ny - 0.42f) * (ny - 0.42f) * 19.0f));
  civ_float_t c3 = expf(-((nx - 0.55f) * (nx - 0.55f) * 30.0f +
                          (ny - 0.78f) * (ny - 0.78f) * 24.0f));

  return 0.46f + wave_a + wave_b + (c1 + c2 + c3) * 0.55f;
}

civ_map_gen_params_t civ_map_default_params(void) {
  civ_map_gen_params_t p = {0};
  p.width = CIV_DEFAULT_MAP_WIDTH;
  p.height = CIV_DEFAULT_MAP_HEIGHT;
  p.sea_level = CIV_DEFAULT_SEA_LEVEL;
  p.seed = CIV_GLOBAL_MAP_SEED;
  p.generate_rivers = false;
  p.generate_mountains = false;
  p.generate_resources = false;
  return p;
}

civ_result_t civ_map_genesis(civ_map_t *map) {
  if (!map)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null Map"};

  civ_map_gen_params_t p = civ_map_default_params();
  civ_journal_log(g_journal, CIV_JOURNAL_WORLD_GEN_START,
                  "Global atlas generation started", &p.seed,
                  sizeof(uint32_t));
  return civ_map_generate(map, &p);
}

civ_result_t civ_map_generate(civ_map_t *map,
                              const civ_map_gen_params_t *params) {
  if (!map || !params)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null Params"};

  /* Single global atlas definition: no per-run randomization */
  map->seed = CIV_GLOBAL_MAP_SEED;
  map->sea_level = clampf(params->sea_level, 0.2f, 0.8f);
  map->land_tile_count = 0;
  map->river_tile_count = 0;
  map->mountain_tile_count = 0;

  const civ_float_t polar_band = 0.08f;

  for (int32_t y = 0; y < map->height; y++) {
    if ((y & 63) == 0) {
      g_gen_progress = (float)y / (float)map->height;
    }

    civ_float_t ny = (civ_float_t)y / (civ_float_t)(map->height - 1);
    bool is_polar = (ny <= polar_band || ny >= (1.0f - polar_band));

    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
      if (!tile)
        continue;

      civ_float_t nx = (civ_float_t)x / (civ_float_t)(map->width - 1);
      civ_float_t shape = atlas_shape_signal(nx, ny);
      bool is_land = (!is_polar && shape >= map->sea_level);

      tile->x = x;
      tile->y = y;
      tile->elevation = is_land ? 0.65f : 0.1f;
      tile->temperature = 0.0f;
      tile->moisture = 0.0f;
      tile->has_river = false;
      tile->has_resource = false;
      tile->resources = 0.0f;
      tile->fertility = is_land ? 0.5f : 0.0f;
      tile->vegetation_density = is_land ? 0.3f : 0.0f;
      tile->is_explored = false;
      tile->is_visible = false;
      tile->owner_id[0] = '\0';

      tile->terrain = is_land ? CIV_TERRAIN_PLAIN : CIV_TERRAIN_COASTAL;
      tile->land_use = is_land ? CIV_LAND_USE_GRASSLAND : CIV_LAND_USE_WATER;

      /* Atlas overlays kept simple; political borders are dynamic elsewhere. */
      tile->political_influence = is_land ? 0.5f : 0.0f;
      tile->population_density = is_land ? 0.35f : 0.0f;
      tile->cultural_influence = is_land ? 0.4f : 0.0f;

      if (is_land) {
        map->land_tile_count++;
      }
    }
  }

  g_gen_progress = 1.0f;
  civ_journal_log(g_journal, CIV_JOURNAL_BIOME_FINALIZED,
                  "Global atlas generation complete", NULL, 0);
  return (civ_result_t){CIV_OK, "Global atlas generated"};
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
  (void)m;
  (void)i;
  return (civ_result_t){CIV_OK, "Erosion disabled for atlas mode"};
}

civ_result_t civ_map_generate_rivers(civ_map_t *m) {
  (void)m;
  return (civ_result_t){CIV_OK, "Rivers disabled for atlas mode"};
}

civ_result_t civ_map_generate_resources(civ_map_t *m) {
  (void)m;
  return (civ_result_t){CIV_OK, "Resources disabled for atlas mode"};
}

civ_result_t civ_map_generate_terrain(civ_map_t *m) { return civ_map_genesis(m); }

civ_result_t civ_map_generate_premium(civ_map_t *m,
                                      const civ_map_gen_params_t *p) {
  return civ_map_generate(m, p);
}

civ_float_t civ_map_tile_distance(const civ_map_tile_t *a,
                                  const civ_map_tile_t *b) {
  (void)a;
  (void)b;
  return 0.0f;
}

int32_t civ_map_count_terrain(const civ_map_t *m, civ_terrain_type_t t) {
  (void)m;
  (void)t;
  return 0;
}

int32_t civ_map_count_land_use(const civ_map_t *m, civ_land_use_type_t l) {
  (void)m;
  (void)l;
  return 0;
}

bool civ_map_init(civ_map_t *m, int32_t w, int32_t h, uint32_t s) {
  (void)m;
  (void)w;
  (void)h;
  (void)s;
  return false;
}

civ_result_t
civ_map_generate_terrain_with_params(civ_map_t *m,
                                     const civ_map_gen_params_t *p) {
  return civ_map_generate(m, p);
}
