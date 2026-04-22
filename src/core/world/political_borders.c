/**
 * @file political_borders.c
 * @brief Real political borders from Natural Earth admin-0 country data
 */
#include "core/world/political_borders.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int       count;
  char    **names;
  uint32_t *colors;
  int16_t  *tile_country;
} civ_border_data_t;

static civ_border_data_t *s_borders = NULL;

static void borders_free_data(civ_border_data_t *bd) {
  if (!bd) return;
  for (int i = 0; i < bd->count; i++) free(bd->names[i]);
  free(bd->names);
  free(bd->colors);
  free(bd->tile_country);
  free(bd);
}

bool civ_political_borders_load(const char *filepath, int32_t map_w, int32_t map_h) {
  FILE *f = fopen(filepath, "rb");
  if (!f) { printf("[BORDERS] No file %s\n", filepath); return false; }

  civ_border_data_t *bd = (civ_border_data_t *)calloc(1, sizeof(civ_border_data_t));
  if (!bd) { fclose(f); return false; }

  uint32_t count;
  if (fread(&count, sizeof(count), 1, f) != 1) { borders_free_data(bd); fclose(f); return false; }
  bd->count = (int)count;
  printf("[BORDERS] Loading %d countries...\n", bd->count);

  bd->names   = (char **)calloc((size_t)count, sizeof(char *));
  bd->colors  = (uint32_t *)calloc((size_t)count, sizeof(uint32_t));
  if (!bd->names || !bd->colors) { borders_free_data(bd); fclose(f); return false; }

  for (int i = 0; i < bd->count; i++) {
    uint8_t nl;
    if (fread(&nl, 1, 1, f) != 1) break;
    bd->names[i] = (char *)malloc((size_t)nl + 1);
    if (!bd->names[i]) break;
    if (fread(bd->names[i], 1, nl, f) != nl) break;
    bd->names[i][nl] = '\0';
    if (fread(&bd->colors[i], 4, 1, f) != 1) break;
  }

  size_t tc = (size_t)map_w * map_h;
  bd->tile_country = (int16_t *)malloc(tc * sizeof(int16_t));
  if (!bd->tile_country) { borders_free_data(bd); fclose(f); return false; }
  if (fread(bd->tile_country, sizeof(int16_t), tc, f) != tc) {
    printf("[BORDERS] Short tile read\n");
    borders_free_data(bd); fclose(f); return false;
  }
  fclose(f);

  if (s_borders) borders_free_data(s_borders);
  s_borders = bd;
  printf("[BORDERS] %d countries loaded\n", bd->count);
  return true;
}

void civ_political_borders_apply(civ_map_t *map) {
  if (!s_borders || !map) return;
  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      int16_t cid = s_borders->tile_country[y * map->width + x];
      if (cid < 0 || cid >= s_borders->count) continue;
      if (!s_borders->names[cid]) continue;
      civ_map_tile_t *t = civ_map_get_tile(map, x, y);
      if (!t || t->land_use == CIV_LAND_USE_WATER) continue;
      snprintf(t->owner_id, STRING_SHORT_LEN, "%s", s_borders->names[cid]);
      t->political_color = s_borders->colors[cid];
    }
  }
  printf("[BORDERS] Applied to map\n");
}

const char *civ_political_borders_country_name(int16_t cid) {
  if (!s_borders || cid < 0 || cid >= s_borders->count) return NULL;
  return s_borders->names[cid];
}
uint32_t civ_political_borders_country_color(int16_t cid) {
  if (!s_borders || cid < 0 || cid >= s_borders->count) return 0xFFFF00;
  return s_borders->colors[cid];
}
int16_t civ_political_borders_tile_country(int32_t tx, int32_t ty, int32_t mw) {
  if (!s_borders || tx < 0 || ty < 0 || tx >= mw) return -1;
  return s_borders->tile_country[ty * mw + tx];
}
void civ_political_borders_free(void) {
  borders_free_data(s_borders);
  s_borders = NULL;
}
