#include "core/world/real_world_map.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

civ_result_t civ_earth_map_load(const char *filepath, civ_map_t *map) {
  if (!filepath || !map) return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null arg"};

  FILE *f = fopen(filepath, "rb");
  if (!f) return (civ_result_t){CIV_ERROR_IO, "Cannot open Earth map file"};

  civ_earth_map_header_t hdr;
  if (fread(&hdr, sizeof(hdr), 1, f) != 1) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_IO, "Failed to read Earth map header"};
  }

  if (hdr.magic != CIV_EARTH_MAP_MAGIC) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_INVALID_DATA, "Not a valid Earth map file"};
  }

  if (hdr.width != map->width || hdr.height != map->height) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_INVALID_DATA,
                          "Earth map dimensions don't match game map"};
  }

  map->sea_level = hdr.sea_level;
  map->seed = hdr.seed;

  /* Read landmask: one byte per tile (packed for efficiency) */
  size_t tile_count = (size_t)hdr.width * hdr.height;
  uint8_t *landmask = (uint8_t *)malloc(tile_count);
  if (!landmask) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, NULL};
  }

  if (fread(landmask, 1, tile_count, f) != tile_count) {
    free(landmask);
    fclose(f);
    return (civ_result_t){CIV_ERROR_IO, "Earth map data truncated"};
  }
  fclose(f);

  /* Populate tiles from landmask */
  float polar_band = (float)hdr.height * 0.08f;
  map->land_tile_count = 0;

  for (int32_t y = 0; y < hdr.height; y++) {
    float ny = (float)y / (float)(hdr.height - 1);
    float lat_factor = 1.0f - fabsf(ny - 0.5f) * 2.0f; /* 1 at equator, 0 at poles */

    for (int32_t x = 0; x < hdr.width; x++) {
      civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
      if (!tile) continue;

      uint8_t mask_val = landmask[y * hdr.width + x];
      bool is_land = (mask_val > 0);
      bool is_polar = (y < (int32_t)polar_band || y >= hdr.height - (int32_t)polar_band);

      tile->x = x;
      tile->y = y;
      tile->elevation = is_land ? 0.4f + 0.3f * lat_factor : 0.05f + 0.1f * (1.0f - lat_factor);
      tile->moisture = is_land ? 0.3f + 0.4f * lat_factor : 1.0f;
      tile->temperature = 1.0f - fabsf(ny - 0.5f) * 2.0f;
      tile->terrain = is_land ? CIV_TERRAIN_PLAIN : CIV_TERRAIN_COASTAL;
      tile->land_use = is_land ? CIV_LAND_USE_GRASSLAND : CIV_LAND_USE_WATER;
      tile->fertility = is_land ? 0.3f + 0.4f * lat_factor : 0.0f;
      tile->vegetation_density = is_land ? 0.2f + 0.5f * lat_factor : 0.0f;
      tile->has_river = false;
      tile->has_resource = false;
      tile->resources = is_land ? 0.3f : 0.0f;
      tile->is_explored = true;   /* Earth geography is known */
      tile->is_visible = false;    /* tactical visibility per unit */
      tile->owner_id[0] = '\0';
      tile->political_influence = is_land ? 0.3f : 0.0f;
      tile->population_density = is_land ? 0.2f : 0.0f;
      tile->cultural_influence = is_land ? 0.2f : 0.0f;

      if (is_land) map->land_tile_count++;
    }
  }

  free(landmask);
  return (civ_result_t){CIV_OK, "Earth map loaded"};
}

bool civ_earth_map_is_valid(const char *filepath) {
  if (!filepath) return false;
  FILE *f = fopen(filepath, "rb");
  if (!f) return false;

  civ_earth_map_header_t hdr;
  bool valid = (fread(&hdr, sizeof(hdr), 1, f) == 1 &&
                hdr.magic == CIV_EARTH_MAP_MAGIC);
  fclose(f);
  return valid;
}
