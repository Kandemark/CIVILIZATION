/**
 * @file nation.c
 * @brief Nation definitions with real-world-approximate territories
 *
 * 8 starting nations with territory regions, full government structures
 * with dynamic political positions, administrative subdivisions, and
 * starting indices. Political borders are rendered where adjacent tiles
 * have different nation owners.
 */
#include "../../../include/core/world/nation.h"
#include "../../../include/core/constitution.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ── Helper: point-in-region test ───────────────────────────────────── */
static bool point_in_region(float lon, float lat, civ_nation_region_t *r) {
  return lon >= r->min_lon && lon <= r->max_lon &&
         lat >= r->min_lat && lat <= r->max_lat;
}

/* ── Default nations ─────────────────────────────────────────────────── */
static void init_nation_imperial(civ_nation_t *n) {
  strcpy(n->id, "imperial_dominion");
  strcpy(n->name, "Imperial Dominion");
  n->color = 0xCC2200; n->color_accent = 0xFF6644;
  n->tech_index = 280; n->economic_index = 320; n->military_index = 340;
  n->cultural_index = 220;
  /* Europe + Russia */
  n->regions[0] = (civ_nation_region_t){-10, 60, 35, 72};
  n->regions[1] = (civ_nation_region_t){-10, 40, 25, 55};
  n->region_count = 2;
  n->capital_lon = 12; n->capital_lat = 52;
  n->population = 45000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f); /* 0.7-1.2 */
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);

  /* Subdivisions */
  strcpy(n->subdivisions[0].name, "Northern Province"); strcpy(n->subdivisions[0].id, "north");
  n->subdivisions[0].region = (civ_nation_region_t){-10, 40, 55, 72};
  strcpy(n->subdivisions[1].name, "Southern Province"); strcpy(n->subdivisions[1].id, "south");
  n->subdivisions[1].region = (civ_nation_region_t){-10, 40, 25, 55};
  n->subdivision_count = 2;

  /* Government structure */
  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Supreme Chancellor", 0, 0.80f, 0.10f, 0.10f, "appointed", "life", 1);
  civ_government_add_position(n->government, "Council of Ministers", 1, 0.15f, 0.60f, 0.05f, "appointed_by_above", "indefinite", 15);
  civ_government_add_position(n->government, "Regional Governors", 2, 0.15f, 0.20f, 0.05f, "appointed_by_above", "5_years", 8);
  civ_government_add_position(n->government, "High Tribunal", 1, 0.05f, 0.05f, 0.70f, "appointed", "life", 7);
  civ_government_add_position(n->government, "Provincial Administrators", 3, 0.10f, 0.10f, 0.10f, "appointed_by_above", "3_years", 40);
  civ_government_recompute_profile(n->government);
}

static void init_nation_mercantile(civ_nation_t *n) {
  n->constitution = civ_national_constitution_create(n->id);
  strcpy(n->id, "mercantile_league");
  strcpy(n->name, "Mercantile League");
  n->color = 0xDAA520; n->color_accent = 0xFFD700;
  n->tech_index = 300; n->economic_index = 420; n->military_index = 180;
  n->cultural_index = 280;
  /* Mediterranean + North Africa + Middle East */
  n->regions[0] = (civ_nation_region_t){-10, 45, 30, 48};
  n->regions[1] = (civ_nation_region_t){25, 55, 20, 42};
  n->region_count = 2;
  n->capital_lon = 29; n->capital_lat = 41;
  n->population = 32000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f); /* 0.7-1.2 */
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);

  strcpy(n->subdivisions[0].name, "Western Trade Ports"); strcpy(n->subdivisions[0].id, "west");
  n->subdivisions[0].region = (civ_nation_region_t){-10, 20, 30, 48};
  strcpy(n->subdivisions[1].name, "Eastern Markets"); strcpy(n->subdivisions[1].id, "east");
  n->subdivisions[1].region = (civ_nation_region_t){25, 55, 20, 42};
  n->subdivision_count = 2;

  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Trade Council Head", 0, 0.40f, 0.30f, 0.10f, "elected", "4_years", 1);
  civ_government_add_position(n->government, "Guild Assembly", 1, 0.15f, 0.60f, 0.05f, "elected", "3_years", 50);
  civ_government_add_position(n->government, "Port Authority", 2, 0.20f, 0.05f, 0.05f, "appointed_by_above", "indefinite", 12);
  civ_government_recompute_profile(n->government);
}

static void init_nation_theocratic(civ_nation_t *n) {
  n->constitution = civ_national_constitution_create(n->id);
  strcpy(n->id, "theocratic_order");
  strcpy(n->name, "Theocratic Order");
  n->color = 0x8B00CC; n->color_accent = 0xCC66FF;
  n->tech_index = 180; n->economic_index = 220; n->military_index = 240;
  n->cultural_index = 440;
  /* Middle East + Persia + Central Asia */
  n->regions[0] = (civ_nation_region_t){40, 70, 25, 45};
  n->regions[1] = (civ_nation_region_t){50, 75, 35, 50};
  n->region_count = 2;
  n->capital_lon = 51; n->capital_lat = 35;
  n->population = 28000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f); /* 0.7-1.2 */
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);

  strcpy(n->subdivisions[0].name, "Holy District"); strcpy(n->subdivisions[0].id, "holy");
  n->subdivisions[0].region = (civ_nation_region_t){40, 55, 30, 45};
  strcpy(n->subdivisions[1].name, "Eastern Sanctuary"); strcpy(n->subdivisions[1].id, "east");
  n->subdivisions[1].region = (civ_nation_region_t){55, 75, 25, 50};
  n->subdivision_count = 2;

  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "High Oracle", 0, 0.60f, 0.20f, 0.10f, "hereditary", "life", 1);
  civ_government_add_position(n->government, "Council of Elders", 1, 0.15f, 0.40f, 0.15f, "appointed", "life", 12);
  civ_government_add_position(n->government, "Temple Stewards", 2, 0.10f, 0.10f, 0.20f, "appointed_by_above", "indefinite", 30);
  civ_government_add_position(n->government, "Doctrinal Court", 1, 0.05f, 0.05f, 0.70f, "appointed", "life", 5);
  civ_government_recompute_profile(n->government);
}

static void init_nation_democratic(civ_nation_t *n) {
  n->constitution = civ_national_constitution_create(n->id);
  strcpy(n->id, "democratic_federation");
  strcpy(n->name, "Democratic Federation");
  n->color = 0x2288FF; n->color_accent = 0x66BBFF;
  n->tech_index = 360; n->economic_index = 340; n->military_index = 160;
  n->cultural_index = 380;
  /* North America */
  n->regions[0] = (civ_nation_region_t){-130, -65, 25, 60};
  n->regions[1] = (civ_nation_region_t){-135, -100, 45, 72};
  n->region_count = 2;
  n->capital_lon = -77; n->capital_lat = 39;
  n->population = 52000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f); /* 0.7-1.2 */
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);

  strcpy(n->subdivisions[0].name, "Eastern States"); strcpy(n->subdivisions[0].id, "east");
  n->subdivisions[0].region = (civ_nation_region_t){-90, -65, 25, 50};
  strcpy(n->subdivisions[1].name, "Western Territories"); strcpy(n->subdivisions[1].id, "west");
  n->subdivisions[1].region = (civ_nation_region_t){-130, -90, 25, 60};
  strcpy(n->subdivisions[2].name, "Northern Dominion"); strcpy(n->subdivisions[2].id, "north");
  n->subdivisions[2].region = (civ_nation_region_t){-135, -100, 50, 72};
  n->subdivision_count = 3;

  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "President", 0, 0.30f, 0.10f, 0.05f, "elected", "5_years", 1);
  civ_government_add_position(n->government, "Congress", 1, 0.10f, 0.65f, 0.05f, "elected", "3_years", 200);
  civ_government_add_position(n->government, "Supreme Court", 1, 0.05f, 0.05f, 0.60f, "appointed", "life", 9);
  civ_government_add_position(n->government, "State Governors", 2, 0.15f, 0.10f, 0.05f, "elected", "4_years", 15);
  civ_government_recompute_profile(n->government);
}

static void init_nation_technocratic(civ_nation_t *n) {
  n->constitution = civ_national_constitution_create(n->id);
  strcpy(n->id, "technocratic_union");
  strcpy(n->name, "Technocratic Union");
  n->color = 0x00AACC; n->color_accent = 0x44EEFF;
  n->tech_index = 520; n->economic_index = 280; n->military_index = 140;
  n->cultural_index = 200;
  /* East Asia */
  n->regions[0] = (civ_nation_region_t){100, 150, 20, 50};
  n->regions[1] = (civ_nation_region_t){125, 145, 30, 45};
  n->region_count = 2;
  n->capital_lon = 139; n->capital_lat = 36;
  n->population = 38000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f); /* 0.7-1.2 */
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);

  strcpy(n->subdivisions[0].name, "Research District"); strcpy(n->subdivisions[0].id, "research");
  n->subdivisions[0].region = (civ_nation_region_t){135, 145, 34, 40};
  strcpy(n->subdivisions[1].name, "Industrial Corridor"); strcpy(n->subdivisions[1].id, "industry");
  n->subdivisions[1].region = (civ_nation_region_t){125, 135, 30, 42};
  n->subdivision_count = 2;

  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Chief Scientist", 0, 0.50f, 0.15f, 0.10f, "appointed", "8_years", 1);
  civ_government_add_position(n->government, "Academy Senate", 1, 0.15f, 0.45f, 0.15f, "elected", "5_years", 60);
  civ_government_add_position(n->government, "Research Directors", 2, 0.15f, 0.10f, 0.10f, "appointed_by_above", "indefinite", 20);
  civ_government_recompute_profile(n->government);
}

static void init_nation_martial(civ_nation_t *n) {
  n->constitution = civ_national_constitution_create(n->id);
  strcpy(n->id, "martial_horde");
  strcpy(n->name, "Martial Horde");
  n->color = 0xCC0000; n->color_accent = 0xFF3333;
  n->tech_index = 140; n->economic_index = 160; n->military_index = 480;
  n->cultural_index = 140;
  /* Central Asia + Steppe */
  n->regions[0] = (civ_nation_region_t){60, 100, 35, 55};
  n->regions[1] = (civ_nation_region_t){70, 90, 30, 50};
  n->region_count = 2;
  n->capital_lon = 76; n->capital_lat = 43;
  n->population = 18000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f); /* 0.7-1.2 */
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);

  strcpy(n->subdivisions[0].name, "Western Horde"); strcpy(n->subdivisions[0].id, "west");
  n->subdivisions[0].region = (civ_nation_region_t){60, 80, 35, 55};
  strcpy(n->subdivisions[1].name, "Eastern Horde"); strcpy(n->subdivisions[1].id, "east");
  n->subdivisions[1].region = (civ_nation_region_t){80, 100, 35, 55};
  n->subdivision_count = 2;

  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Supreme Commander", 0, 0.85f, 0.05f, 0.05f, "military_coup", "life", 1);
  civ_government_add_position(n->government, "War Council", 1, 0.10f, 0.15f, 0.05f, "appointed_by_above", "indefinite", 8);
  civ_government_recompute_profile(n->government);
}

static void init_nation_agricultural(civ_nation_t *n) {
  n->constitution = civ_national_constitution_create(n->id);
  strcpy(n->id, "agricultural_collective");
  strcpy(n->name, "Agricultural Collective");
  n->color = 0x228800; n->color_accent = 0x44DD44;
  n->tech_index = 200; n->economic_index = 260; n->military_index = 120;
  n->cultural_index = 260;
  /* South Asia + SE Asia */
  n->regions[0] = (civ_nation_region_t){68, 100, 5, 35};
  n->regions[1] = (civ_nation_region_t){95, 120, -10, 10};
  n->region_count = 2;
  n->capital_lon = 77; n->capital_lat = 23;
  n->population = 65000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f); /* 0.7-1.2 */
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);

  strcpy(n->subdivisions[0].name, "Northern Plains"); strcpy(n->subdivisions[0].id, "north");
  n->subdivisions[0].region = (civ_nation_region_t){68, 100, 20, 35};
  strcpy(n->subdivisions[1].name, "Southern Riverlands"); strcpy(n->subdivisions[1].id, "south");
  n->subdivisions[1].region = (civ_nation_region_t){68, 100, 5, 20};
  n->subdivision_count = 2;

  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "First Farmer", 0, 0.40f, 0.30f, 0.10f, "elected", "4_years", 1);
  civ_government_add_position(n->government, "Land Council", 1, 0.15f, 0.50f, 0.10f, "elected", "3_years", 100);
  civ_government_add_position(n->government, "Village Assemblies", 2, 0.10f, 0.15f, 0.05f, "elected", "2_years", 500);
  civ_government_recompute_profile(n->government);
}

static void init_nation_stewards(civ_nation_t *n) {
  n->constitution = civ_national_constitution_create(n->id);
  strcpy(n->id, "stewards_wild");
  strcpy(n->name, "Stewards of the Wild");
  n->color = 0x228844; n->color_accent = 0x66CC88;
  n->tech_index = 220; n->economic_index = 180; n->military_index = 140;
  n->cultural_index = 400;
  /* South America */
  n->regions[0] = (civ_nation_region_t){-80, -35, -40, 10};
  n->regions[1] = (civ_nation_region_t){-75, -40, -20, 5};
  n->region_count = 2;
  n->capital_lon = -47; n->capital_lat = -16;
  n->population = 22000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f); /* 0.7-1.2 */
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);

  strcpy(n->subdivisions[0].name, "Amazon Basin"); strcpy(n->subdivisions[0].id, "amazon");
  n->subdivisions[0].region = (civ_nation_region_t){-75, -45, -20, 5};
  strcpy(n->subdivisions[1].name, "Southern Highlands"); strcpy(n->subdivisions[1].id, "highlands");
  n->subdivisions[1].region = (civ_nation_region_t){-75, -35, -40, -20};
  n->subdivision_count = 2;

  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Elder Speaker", 0, 0.25f, 0.40f, 0.15f, "elected", "3_years", 1);
  civ_government_add_position(n->government, "Circle of Wisdom", 1, 0.10f, 0.50f, 0.20f, "lottery", "2_years", 30);
  civ_government_add_position(n->government, "Region Keepers", 2, 0.20f, 0.10f, 0.10f, "elected", "indefinite", 10);
  civ_government_recompute_profile(n->government);
}

/* ── Manager ─────────────────────────────────────────────────────────── */
civ_nation_manager_t *civ_nation_manager_create(void) {
  civ_nation_manager_t *mgr = (civ_nation_manager_t *)malloc(sizeof(civ_nation_manager_t));
  if (!mgr) return NULL;
  memset(mgr, 0, sizeof(*mgr));
  return mgr;
}

void civ_nation_manager_destroy(civ_nation_manager_t *mgr) {
  if (!mgr) return;
  for (int i = 0; i < mgr->count; i++)
    if (mgr->nations[i].government)
      civ_government_destroy(mgr->nations[i].government);
  free(mgr);
}

void civ_nation_manager_init_default(civ_nation_manager_t *mgr) {
  if (!mgr) return;

  typedef void (*init_fn)(civ_nation_t *);
  init_fn inits[] = {
      init_nation_imperial, init_nation_mercantile, init_nation_theocratic,
      init_nation_democratic, init_nation_technocratic, init_nation_martial,
      init_nation_agricultural, init_nation_stewards,
  };

  mgr->count = CIV_NATION_COUNT;
  for (int i = 0; i < CIV_NATION_COUNT; i++) {
    inits[i](&mgr->nations[i]);
  }
  mgr->player_nation_index = 3; /* Democratic Federation as default player nation */
}

/* ── Territory claiming ─────────────────────────────────────────────── */
void civ_nation_claim_territory(civ_nation_t *n, civ_map_t *map) {
  if (!n || !map) return;

  for (int32_t y = 0; y < map->height; y++) {
    float lat = 90.0f - (float)y / (float)(map->height - 1) * 180.0f;
    for (int32_t x = 0; x < map->width; x++) {
      float lon = (float)x / (float)(map->width - 1) * 360.0f - 180.0f;

      /* Check if this tile is in any of the nation's regions */
      for (int r = 0; r < n->region_count; r++) {
        if (point_in_region(lon, lat, &n->regions[r])) {
          civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
          if (tile && tile->land_use != CIV_LAND_USE_WATER) {
            /* Only claim if unclaimed */
            if (tile->owner_id[0] == '\0') {
              strncpy(tile->owner_id, n->id, STRING_SHORT_LEN - 1);
            }
            /* Also check subdivision membership */
            for (int s = 0; s < n->subdivision_count; s++) {
              if (point_in_region(lon, lat, &n->subdivisions[s].region)) {
                strncpy(tile->owner_id, n->subdivisions[s].id, STRING_SHORT_LEN - 1);
              }
            }
          }
          break;
        }
      }
    }
  }
}

void civ_nation_claim_all_territories(civ_nation_manager_t *mgr, civ_map_t *map) {
  if (!mgr || !map) return;
  for (int i = 0; i < mgr->count; i++)
    civ_nation_claim_territory(&mgr->nations[i], map);
}

/* ── Query ───────────────────────────────────────────────────────────── */
civ_nation_t *civ_nation_get_by_id(civ_nation_manager_t *mgr, const char *id) {
  if (!mgr || !id) return NULL;
  for (int i = 0; i < mgr->count; i++)
    if (strcmp(mgr->nations[i].id, id) == 0) return &mgr->nations[i];
  return NULL;
}

civ_nation_t *civ_nation_get_by_index(civ_nation_manager_t *mgr, int idx) {
  if (!mgr || idx < 0 || idx >= mgr->count) return NULL;
  return &mgr->nations[idx];
}

bool civ_nation_contains_point(civ_nation_t *n, float lon, float lat) {
  if (!n) return false;
  for (int r = 0; r < n->region_count; r++)
    if (point_in_region(lon, lat, &n->regions[r])) return true;
  return false;
}

bool civ_nation_contains_tile(civ_nation_t *n, int32_t tx, int32_t ty,
                              int32_t map_w, int32_t map_h) {
  if (!n || map_w <= 0 || map_h <= 0) return false;
  float lon = (float)tx / (float)(map_w - 1) * 360.0f - 180.0f;
  float lat = 90.0f - (float)ty / (float)(map_h - 1) * 180.0f;
  return civ_nation_contains_point(n, lon, lat);
}
