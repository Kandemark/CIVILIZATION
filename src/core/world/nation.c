/**
 * @file nation.c
 * @brief Data-driven nation system from Natural Earth borders + nations_data
 */

#include "core/world/nation.h"
#include "core/constitution.h"
#include "core/world/nations_data.h"
#include "core/world/political_borders.h"
#include "core/world/resource_map.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ── Helper: point-in-region test ───────────────────────────────────── */
static bool point_in_region(float lon, float lat, civ_nation_region_t *r) {
  return lon >= r->min_lon && lon <= r->max_lon &&
         lat >= r->min_lat && lat <= r->max_lat;
}

/* ── Manager lifecycle ──────────────────────────────────────────────── */
civ_nation_manager_t *civ_nation_manager_create(void) {
  civ_nation_manager_t *mgr = calloc(1, sizeof(*mgr));
  if (!mgr) return NULL;
  mgr->capacity = CIV_NATION_CAPACITY_MAX;
  mgr->nations = calloc((size_t)mgr->capacity, sizeof(civ_nation_t));
  if (!mgr->nations) { free(mgr); return NULL; }
  return mgr;
}

void civ_nation_manager_destroy(civ_nation_manager_t *mgr) {
  if (!mgr) return;
  for (int i = 0; i < mgr->count; i++) {
    if (mgr->nations[i].government)
      civ_government_destroy(mgr->nations[i].government);
  }
  free(mgr->nations);
  free(mgr);
}

/* ── Minimal default government for data-driven nations ────────────── */
static void setup_default_government(civ_nation_t *n) {
  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Head of State", 0,
      0.35f, 0.25f, 0.10f, "varies", "varies", 1);
  civ_government_add_position(n->government, "Cabinet", 1,
      0.10f, 0.60f, 0.05f, "appointed", "indefinite", 15);
  civ_government_add_position(n->government, "Legislature", 1,
      0.10f, 0.55f, 0.05f, "elected", "4_years", 100);
  civ_government_add_position(n->government, "High Court", 1,
      0.05f, 0.05f, 0.65f, "appointed", "life", 7);
  civ_government_add_position(n->government, "Regional Governors", 2,
      0.15f, 0.15f, 0.05f, "varies", "varies", 10);
  civ_government_recompute_profile(n->government);
}

/* ── Data-driven initialization ────────────────────────────────────── */
void civ_nation_manager_init_from_data(civ_nation_manager_t *mgr,
                                        const void *nd_void,
                                        int map_w, int map_h,
                                        int default_count) {
  if (!mgr) return;
  const civ_nations_data_t *nd = (const civ_nations_data_t *)nd_void;

  int border_count = civ_political_borders_country_count();

  if (border_count > 0 && border_count <= mgr->capacity) {
    /* Create one nation per country in the borders data */
    int created = 0;
    for (int cid = 0; cid < border_count; cid++) {
      const char *cname = civ_political_borders_country_name((int16_t)cid);
      if (!cname || !cname[0]) continue;

      uint32_t color = civ_political_borders_country_color((int16_t)cid);

      civ_nation_t *n = &mgr->nations[created];
      memset(n, 0, sizeof(*n));

      /* Build a machine-friendly id from the name */
      snprintf(n->id, CIV_NATION_ID_MAX, "nation_%d", created);
      snprintf(n->name, CIV_NATION_NAME_MAX, "%s", cname);
      n->color = color;
      n->color_accent = color;

      /* Look up in nations_data for metadata */
      const civ_nation_data_t *data = NULL;
      if (nd) data = civ_nations_data_get_by_name(nd, cname);

      if (data) {
        n->data_id = data->id;
        snprintf(n->iso_a3, sizeof(n->iso_a3), "%s", data->iso_a3);
        snprintf(n->iso_a2, sizeof(n->iso_a2), "%s", data->iso_a2);
        n->capital_lon = data->capital_lon;
        n->capital_lat = data->capital_lat;
        n->population = (int64_t)data->population_est;
        n->gdp_per_capita = data->gdp_est_millions > 0
            ? (float)data->gdp_est_millions * 1000000.0f / (float)(data->population_est > 0 ? data->population_est : 1)
            : (float)(500 + (rand() % 9500));

        /* Starting indices derived from GDP and population */
        n->tech_index = (int32_t)(100 + (data->gdp_est_millions % 500));
        n->economic_index = (int32_t)(80 + (data->gdp_est_millions % 400));
        n->military_index = (int32_t)(50 + (data->population_est % 300));
        n->cultural_index = (int32_t)(60 + (data->population_est % 350));
      } else {
        /* Fallback for countries not in nations_data */
        n->data_id = (uint32_t)cid;
        n->population = 5000000 + (rand() % 5000000);
        n->gdp_per_capita = 500.0f + (float)(rand() % 9500);
        n->tech_index = 150;
        n->economic_index = 120;
        n->military_index = 100;
        n->cultural_index = 100;
      }

      n->cost_of_living = 0.7f + ((float)(rand() % 100) / 200.0f);
      n->region_count = 0;
      n->subdivision_count = 0;

      setup_default_government(n);
      created++;
    }
    mgr->count = created;
    mgr->player_nation_index = 0;
    printf("[NATION] Created %d nations from borders data\n", mgr->count);
  } else {
    /* Fall back to hardcoded 8 nations for procedural maps */
    civ_nation_manager_init_default(mgr);
  }
}

/* ── Hardcoded fallback (procedural maps without borders data) ─────── */

static void init_imperial(civ_nation_t *n) {
  strcpy(n->id, "imperial_dominion");
  strcpy(n->name, "Imperial Dominion");
  n->color = 0xCC2200; n->color_accent = 0xFF6644;
  n->tech_index = 280; n->economic_index = 320; n->military_index = 340;
  n->cultural_index = 220;
  n->regions[0] = (civ_nation_region_t){-10, 60, 35, 72};
  n->regions[1] = (civ_nation_region_t){-10, 40, 25, 55};
  n->region_count = 2;
  n->capital_lon = 12; n->capital_lat = 52;
  n->population = 45000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f);
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);
  strcpy(n->subdivisions[0].name, "Northern Province");
  strcpy(n->subdivisions[0].id, "north");
  n->subdivisions[0].region = (civ_nation_region_t){-10, 40, 55, 72};
  strcpy(n->subdivisions[1].name, "Southern Province");
  strcpy(n->subdivisions[1].id, "south");
  n->subdivisions[1].region = (civ_nation_region_t){-10, 40, 25, 55};
  n->subdivision_count = 2;
  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Supreme Chancellor", 0, 0.80f, 0.10f, 0.10f, "appointed", "life", 1);
  civ_government_add_position(n->government, "Council of Ministers", 1, 0.15f, 0.60f, 0.05f, "appointed_by_above", "indefinite", 15);
  civ_government_add_position(n->government, "Regional Governors", 2, 0.15f, 0.20f, 0.05f, "appointed_by_above", "5_years", 8);
  civ_government_add_position(n->government, "High Tribunal", 1, 0.05f, 0.05f, 0.70f, "appointed", "life", 7);
  civ_government_add_position(n->government, "Provincial Administrators", 3, 0.10f, 0.10f, 0.10f, "appointed_by_above", "3_years", 40);
  civ_government_recompute_profile(n->government);
}

static void init_mercantile(civ_nation_t *n) {
  strcpy(n->id, "mercantile_league");
  strcpy(n->name, "Mercantile League");
  n->color = 0xDAA520; n->color_accent = 0xFFD700;
  n->tech_index = 300; n->economic_index = 420; n->military_index = 180;
  n->cultural_index = 280;
  n->regions[0] = (civ_nation_region_t){-10, 45, 30, 48};
  n->regions[1] = (civ_nation_region_t){25, 55, 20, 42};
  n->region_count = 2;
  n->capital_lon = 29; n->capital_lat = 41;
  n->population = 32000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f);
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);
  strcpy(n->subdivisions[0].name, "Western Trade Ports");
  strcpy(n->subdivisions[0].id, "west");
  n->subdivisions[0].region = (civ_nation_region_t){-10, 20, 30, 48};
  strcpy(n->subdivisions[1].name, "Eastern Markets");
  strcpy(n->subdivisions[1].id, "east");
  n->subdivisions[1].region = (civ_nation_region_t){25, 55, 20, 42};
  n->subdivision_count = 2;
  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Trade Council Head", 0, 0.40f, 0.30f, 0.10f, "elected", "4_years", 1);
  civ_government_add_position(n->government, "Guild Assembly", 1, 0.15f, 0.60f, 0.05f, "elected", "3_years", 50);
  civ_government_add_position(n->government, "Port Authority", 2, 0.20f, 0.05f, 0.05f, "appointed_by_above", "indefinite", 12);
  civ_government_recompute_profile(n->government);
}

static void init_theocratic(civ_nation_t *n) {
  strcpy(n->id, "theocratic_order");
  strcpy(n->name, "Theocratic Order");
  n->color = 0x8B00CC; n->color_accent = 0xCC66FF;
  n->tech_index = 180; n->economic_index = 220; n->military_index = 240;
  n->cultural_index = 440;
  n->regions[0] = (civ_nation_region_t){40, 70, 25, 45};
  n->regions[1] = (civ_nation_region_t){50, 75, 35, 50};
  n->region_count = 2;
  n->capital_lon = 51; n->capital_lat = 35;
  n->population = 28000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f);
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);
  strcpy(n->subdivisions[0].name, "Holy District");
  strcpy(n->subdivisions[0].id, "holy");
  n->subdivisions[0].region = (civ_nation_region_t){40, 55, 30, 45};
  strcpy(n->subdivisions[1].name, "Eastern Sanctuary");
  strcpy(n->subdivisions[1].id, "east");
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

static void init_democratic(civ_nation_t *n) {
  strcpy(n->id, "democratic_federation");
  strcpy(n->name, "Democratic Federation");
  n->color = 0x2288FF; n->color_accent = 0x66BBFF;
  n->tech_index = 360; n->economic_index = 340; n->military_index = 160;
  n->cultural_index = 380;
  n->regions[0] = (civ_nation_region_t){-130, -65, 25, 60};
  n->regions[1] = (civ_nation_region_t){-135, -100, 45, 72};
  n->region_count = 2;
  n->capital_lon = -77; n->capital_lat = 39;
  n->population = 52000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f);
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);
  strcpy(n->subdivisions[0].name, "Eastern States");
  strcpy(n->subdivisions[0].id, "east");
  n->subdivisions[0].region = (civ_nation_region_t){-90, -65, 25, 50};
  strcpy(n->subdivisions[1].name, "Western Territories");
  strcpy(n->subdivisions[1].id, "west");
  n->subdivisions[1].region = (civ_nation_region_t){-130, -90, 25, 60};
  strcpy(n->subdivisions[2].name, "Northern Dominion");
  strcpy(n->subdivisions[2].id, "north");
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

static void init_technocratic(civ_nation_t *n) {
  strcpy(n->id, "technocratic_union");
  strcpy(n->name, "Technocratic Union");
  n->color = 0x00AACC; n->color_accent = 0x44EEFF;
  n->tech_index = 520; n->economic_index = 280; n->military_index = 140;
  n->cultural_index = 200;
  n->regions[0] = (civ_nation_region_t){100, 150, 20, 50};
  n->regions[1] = (civ_nation_region_t){125, 145, 30, 45};
  n->region_count = 2;
  n->capital_lon = 139; n->capital_lat = 36;
  n->population = 38000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f);
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);
  strcpy(n->subdivisions[0].name, "Research District");
  strcpy(n->subdivisions[0].id, "research");
  n->subdivisions[0].region = (civ_nation_region_t){135, 145, 34, 40};
  strcpy(n->subdivisions[1].name, "Industrial Corridor");
  strcpy(n->subdivisions[1].id, "industry");
  n->subdivisions[1].region = (civ_nation_region_t){125, 135, 30, 42};
  n->subdivision_count = 2;
  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Chief Scientist", 0, 0.50f, 0.15f, 0.10f, "appointed", "8_years", 1);
  civ_government_add_position(n->government, "Academy Senate", 1, 0.15f, 0.45f, 0.15f, "elected", "5_years", 60);
  civ_government_add_position(n->government, "Research Directors", 2, 0.15f, 0.10f, 0.10f, "appointed_by_above", "indefinite", 20);
  civ_government_recompute_profile(n->government);
}

static void init_martial(civ_nation_t *n) {
  strcpy(n->id, "martial_horde");
  strcpy(n->name, "Martial Horde");
  n->color = 0xCC0000; n->color_accent = 0xFF3333;
  n->tech_index = 140; n->economic_index = 160; n->military_index = 480;
  n->cultural_index = 140;
  n->regions[0] = (civ_nation_region_t){60, 100, 35, 55};
  n->regions[1] = (civ_nation_region_t){70, 90, 30, 50};
  n->region_count = 2;
  n->capital_lon = 76; n->capital_lat = 43;
  n->population = 18000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f);
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);
  strcpy(n->subdivisions[0].name, "Western Horde");
  strcpy(n->subdivisions[0].id, "west");
  n->subdivisions[0].region = (civ_nation_region_t){60, 80, 35, 55};
  strcpy(n->subdivisions[1].name, "Eastern Horde");
  strcpy(n->subdivisions[1].id, "east");
  n->subdivisions[1].region = (civ_nation_region_t){80, 100, 35, 55};
  n->subdivision_count = 2;
  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Supreme Commander", 0, 0.85f, 0.05f, 0.05f, "military_coup", "life", 1);
  civ_government_add_position(n->government, "War Council", 1, 0.10f, 0.15f, 0.05f, "appointed_by_above", "indefinite", 8);
  civ_government_recompute_profile(n->government);
}

static void init_agricultural(civ_nation_t *n) {
  strcpy(n->id, "agricultural_collective");
  strcpy(n->name, "Agricultural Collective");
  n->color = 0x228800; n->color_accent = 0x44DD44;
  n->tech_index = 200; n->economic_index = 260; n->military_index = 120;
  n->cultural_index = 260;
  n->regions[0] = (civ_nation_region_t){68, 100, 5, 35};
  n->regions[1] = (civ_nation_region_t){95, 120, -10, 10};
  n->region_count = 2;
  n->capital_lon = 77; n->capital_lat = 23;
  n->population = 65000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f);
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);
  strcpy(n->subdivisions[0].name, "Northern Plains");
  strcpy(n->subdivisions[0].id, "north");
  n->subdivisions[0].region = (civ_nation_region_t){68, 100, 20, 35};
  strcpy(n->subdivisions[1].name, "Southern Riverlands");
  strcpy(n->subdivisions[1].id, "south");
  n->subdivisions[1].region = (civ_nation_region_t){68, 100, 5, 20};
  n->subdivision_count = 2;
  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "First Farmer", 0, 0.40f, 0.30f, 0.10f, "elected", "4_years", 1);
  civ_government_add_position(n->government, "Land Council", 1, 0.15f, 0.50f, 0.10f, "elected", "3_years", 100);
  civ_government_add_position(n->government, "Village Assemblies", 2, 0.10f, 0.15f, 0.05f, "elected", "2_years", 500);
  civ_government_recompute_profile(n->government);
}

static void init_stewards(civ_nation_t *n) {
  strcpy(n->id, "stewards_wild");
  strcpy(n->name, "Stewards of the Wild");
  n->color = 0x228844; n->color_accent = 0x66CC88;
  n->tech_index = 220; n->economic_index = 180; n->military_index = 140;
  n->cultural_index = 400;
  n->regions[0] = (civ_nation_region_t){-80, -35, -40, 10};
  n->regions[1] = (civ_nation_region_t){-75, -40, -20, 5};
  n->region_count = 2;
  n->capital_lon = -47; n->capital_lat = -16;
  n->population = 22000000;
  n->cost_of_living = 0.7f + ((float)(rand()%100)/200.0f);
  n->gdp_per_capita = 500.0f + (float)(rand()%9500);
  strcpy(n->subdivisions[0].name, "Amazon Basin");
  strcpy(n->subdivisions[0].id, "amazon");
  n->subdivisions[0].region = (civ_nation_region_t){-75, -45, -20, 5};
  strcpy(n->subdivisions[1].name, "Southern Highlands");
  strcpy(n->subdivisions[1].id, "highlands");
  n->subdivisions[1].region = (civ_nation_region_t){-75, -35, -40, -20};
  n->subdivision_count = 2;
  n->government = civ_government_create(n->name);
  n->constitution = civ_national_constitution_create(n->id);
  civ_government_add_position(n->government, "Elder Speaker", 0, 0.25f, 0.40f, 0.15f, "elected", "3_years", 1);
  civ_government_add_position(n->government, "Circle of Wisdom", 1, 0.10f, 0.50f, 0.20f, "lottery", "2_years", 30);
  civ_government_add_position(n->government, "Region Keepers", 2, 0.20f, 0.10f, 0.10f, "elected", "indefinite", 10);
  civ_government_recompute_profile(n->government);
}

void civ_nation_manager_init_default(civ_nation_manager_t *mgr) {
  if (!mgr) return;
  typedef void (*init_fn)(civ_nation_t *);
  init_fn inits[] = {
      init_imperial, init_mercantile, init_theocratic,
      init_democratic, init_technocratic, init_martial,
      init_agricultural, init_stewards,
  };
  int n = (int)(sizeof(inits) / sizeof(inits[0]));
  if (n > mgr->capacity) n = mgr->capacity;
  mgr->count = n;
  for (int i = 0; i < n; i++) {
    inits[i](&mgr->nations[i]);
  }
  mgr->player_nation_index = 3;
}

/* ── Territory claiming ─────────────────────────────────────────────── */
void civ_nation_claim_territory(civ_nation_t *n, civ_map_t *map) {
  if (!n || !map) return;
  for (int32_t y = 0; y < map->height; y++) {
    float lat = 90.0f - (float)y / (float)(map->height - 1) * 180.0f;
    for (int32_t x = 0; x < map->width; x++) {
      float lon = (float)x / (float)(map->width - 1) * 360.0f - 180.0f;
      for (int r = 0; r < n->region_count; r++) {
        if (point_in_region(lon, lat, &n->regions[r])) {
          civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
          if (tile && tile->land_use != CIV_LAND_USE_WATER) {
            if (tile->owner_id[0] == '\0') {
              strncpy(tile->owner_id, n->id, STRING_SHORT_LEN - 1);
              tile->political_color = n->color;
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

void civ_nation_claim_from_borders(civ_nation_t *n, civ_map_t *map,
                                    int nation_idx, int map_w, int map_h) {
  if (!n || !map) return;
  for (int32_t y = 0; y < map_h; y++) {
    for (int32_t x = 0; x < map_w; x++) {
      int16_t cid = civ_political_borders_tile_country(x, y, map_w);
      if (cid == (int16_t)nation_idx) {
        civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
        if (tile && tile->land_use != CIV_LAND_USE_WATER) {
          strncpy(tile->owner_id, n->id, STRING_SHORT_LEN - 1);
          tile->political_color = n->color;
        }
      }
    }
  }
}

/* ── Resource accounting ───────────────────────────────────────────── */
void civ_nation_calculate_resources(civ_nation_t *n, civ_map_t *map,
                                     const void *rm_void,
                                     civ_nation_resource_profile_t *out) {
  if (!n || !map || !out) return;
  memset(out, 0, sizeof(*out));

  const civ_resource_map_t *rm = (const civ_resource_map_t *)rm_void;

  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
      if (!tile || tile->land_use == CIV_LAND_USE_WATER) continue;
      if (strcmp(tile->owner_id, n->id) != 0) continue;

      if (rm) {
        for (int t = 0; t < 20; t++) {
          if (civ_resource_map_has_type(rm, x, y, (civ_resource_type_t)t)) {
            uint16_t qty = civ_resource_map_get_quantity(rm, x, y, (civ_resource_type_t)t);
            uint8_t qual = civ_resource_map_get_quality(rm, x, y, (civ_resource_type_t)t);
            out->quantities[t] = (uint16_t)(out->quantities[t] + qty > 65535
                ? 65535 : out->quantities[t] + qty);
            if (qual > out->best_quality[t]) out->best_quality[t] = qual;
          }
        }
      }
    }
  }

  /* Compute summary stats */
  for (int t = 0; t < 20; t++) {
    out->total_resources = (uint16_t)(out->total_resources + out->quantities[t] > 65535
        ? 65535 : out->total_resources + out->quantities[t]);
    if (out->quantities[t] > 0) out->distinct_types++;
  }
}

/* ── Lookup ─────────────────────────────────────────────────────────── */
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

civ_nation_t *civ_nation_find_owner(civ_nation_manager_t *mgr, civ_map_t *map,
                                     int32_t tx, int32_t ty) {
  if (!mgr || !map) return NULL;
  civ_map_tile_t *tile = civ_map_get_tile(map, tx, ty);
  if (!tile || tile->owner_id[0] == '\0') return NULL;
  for (int i = 0; i < mgr->count; i++) {
    if (strcmp(mgr->nations[i].id, tile->owner_id) == 0)
      return &mgr->nations[i];
  }
  return NULL;
}

/* ── Per-nation economic computation ───────────────────────────────── */
void civ_nation_compute_economy(civ_nation_t *n, civ_map_t *map,
                                 const void *rm_void) {
  if (!n || !map) return;
  const civ_resource_map_t *rm = (const civ_resource_map_t *)rm_void;

  memset(&n->economy, 0, sizeof(n->economy));

  int land_tiles = 0, water_tiles = 0;
  float total_elevation = 0.0f;
  float total_fertility = 0.0f;

  /* Count owned tiles and sum resource quantities */
  civ_nation_resource_profile_t rp;
  civ_nation_calculate_resources(n, map, rm, &rp);

  for (int32_t y = 0; y < map->height; y++) {
    for (int32_t x = 0; x < map->width; x++) {
      civ_map_tile_t *t = civ_map_get_tile(map, x, y);
      if (!t) continue;
      if (strcmp(t->owner_id, n->id) != 0) continue;

      if (t->land_use == CIV_LAND_USE_WATER) {
        water_tiles++;
      } else {
        land_tiles++;
        total_elevation += t->elevation;
        total_fertility += t->fertility;
      }
    }
  }

  n->economy.owned_land_tiles = land_tiles;
  n->economy.owned_water_tiles = water_tiles;

  if (land_tiles == 0) return;

  float avg_elevation = total_elevation / (float)land_tiles;
  float avg_fertility = total_fertility / (float)land_tiles;
  int64_t pop = n->population > 0 ? n->population : 1000000;
  float tech_factor = 1.0f + (float)n->tech_index * 0.002f;
  float econ_factor = 1.0f + (float)n->economic_index * 0.002f;

  /* GDP: base from land + tech + resources */
  float gdp_base = (float)land_tiles * 0.5f * tech_factor * econ_factor;
  float resource_bonus = (float)rp.total_resources * 0.01f;
  float gdp = gdp_base + resource_bonus;
  if (gdp < 1.0f) gdp = 1.0f;

  n->economy.gdp = gdp;
  n->economy.gdp_per_capita = gdp * 1000000.0f / (float)pop;
  n->economy.gdp_growth = 0.01f + (float)n->economic_index * 0.0001f;
  n->economy.inflation = 0.02f;
  n->economy.unemployment = 0.04f + (1.0f / (econ_factor + 0.5f)) * 0.04f;
  n->economy.labor_force = (float)pop * 0.45f;
  n->economy.avg_wage = 20000.0f + gdp * 0.5f;

  /* Food: from arable land + fertility + agriculture resources */
  n->economy.food_production = (float)land_tiles * avg_fertility * 500.0f * tech_factor;
  n->economy.food_consumption = (float)pop * 2500.0f * 365.0f / 1000000.0f;

  /* Energy: from oil/gas/coal resources + tech */
  float energy_res = (float)(rp.quantities[CIV_RESOURCE_OIL] +
                              rp.quantities[CIV_RESOURCE_NATURAL_GAS] +
                              rp.quantities[CIV_RESOURCE_COAL]);
  n->economy.energy_output = (float)land_tiles * 0.1f * tech_factor + energy_res * 0.5f;
  if (n->economy.energy_output < 1.0f) n->economy.energy_output = 1.0f;

  /* Raw materials: from extraction resources */
  n->economy.raw_materials_output = (float)rp.total_resources * 10.0f * tech_factor;
  if (n->economy.raw_materials_output < 1.0f) n->economy.raw_materials_output = 1.0f;

  /* Industrial output: from raw materials + manufacturing + tech */
  n->economy.industrial_output = n->economy.raw_materials_output * 1.5f * econ_factor;

  /* Tax revenue: from GDP */
  n->economy.tax_revenue = gdp * 0.2f;
}

void civ_nation_compute_all_economies(civ_nation_manager_t *mgr, civ_map_t *map,
                                       const void *resource_map,
                                       civ_nation_economy_t *global_out) {
  if (!mgr || !map) return;

  if (global_out) memset(global_out, 0, sizeof(*global_out));

  float total_gdp = 0.0f;
  int nations_with_land = 0;

  for (int i = 0; i < mgr->count; i++) {
    civ_nation_compute_economy(&mgr->nations[i], map, resource_map);
    if (mgr->nations[i].economy.owned_land_tiles > 0) {
      total_gdp += mgr->nations[i].economy.gdp;
      nations_with_land++;
    }
  }

  if (global_out && nations_with_land > 0) {
    global_out->gdp = total_gdp;
    global_out->gdp_per_capita = total_gdp / (float)nations_with_land;
    global_out->inflation = 0.02f;
    global_out->unemployment = 0.05f;
  }
}
