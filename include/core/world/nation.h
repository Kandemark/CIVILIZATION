/**
 * @file nation.h
 * @brief Nation definitions with territory, government structure, subdivisions
 *
 * Nations are created data-driven from Natural Earth country data loaded
 * via nations_data and political_borders. Each nation has a full government
 * structure (dynamic political positions defined by its constitution),
 * administrative subdivisions, and territorial borders rendered on the map.
 */
#ifndef CIV_WORLD_NATION_H
#define CIV_WORLD_NATION_H

#include "../../common.h"
#include "../../types.h"
#include "../governance/government.h"
#include "map_generator.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_NATION_NAME_MAX       64
#define CIV_NATION_ID_MAX         32
#define CIV_NATION_CAPACITY_MAX   256
#define CIV_SUBDIVISION_MAX       16
#define CIV_NATION_DEFAULT_COUNT  8    /* fallback for procedural maps */

/* ── Territory region (lat/lon bounding box) ──────────────────────── */
typedef struct {
  float min_lon, max_lon;
  float min_lat, max_lat;
} civ_nation_region_t;

/* ── Economic profile (computed from territory + resources) ─────── */
typedef struct {
  float    gdp;                  /* total GDP in millions */
  float    gdp_per_capita;
  float    gdp_growth;           /* annualized growth rate */
  float    inflation;
  float    unemployment;
  float    tax_revenue;
  float    food_production;      /* kcal/year */
  float    food_consumption;     /* kcal/year */
  float    energy_output;        /* MWh/year */
  float    industrial_output;
  float    raw_materials_output;
  float    labor_force;
  float    avg_wage;
  int      owned_land_tiles;
  int      owned_water_tiles;
} civ_nation_economy_t;

/* ── Nation ─────────────────────────────────────────────────────────── */
typedef struct {
  char                    id[CIV_NATION_ID_MAX];
  char                    name[CIV_NATION_NAME_MAX];
  uint32_t                color;
  uint32_t                color_accent;

  /* Data-driven identity */
  uint32_t                data_id;         /* index into nations_data */
  char                    iso_a3[4];       /* ISO 3166-1 alpha-3 */
  char                    iso_a2[3];       /* ISO 3166-1 alpha-2 */

  /* Territory regions (legacy: used for procedural fallback) */
  civ_nation_region_t     regions[8];
  int                     region_count;

  /* Capital location */
  float                   capital_lon, capital_lat;

  /* Government — fully dynamic, constitution-defined */
  civ_government_t       *government;

  /* Constitution — national rules for all actions */
  void                   *constitution; /* civ_constitution_t — opaque */

  /* Subdivisions (provinces/states/regions) */
  struct {
    char     name[64];
    char     id[32];
    uint32_t color;
    civ_nation_region_t region;
  } subdivisions[CIV_SUBDIVISION_MAX];
  int subdivision_count;

  /* Economic state */
  civ_nation_economy_t           economy;

  /* Starting indices */
  int32_t tech_index;
  int32_t economic_index;
  int32_t military_index;
  int32_t cultural_index;

  /* Population */
  int64_t population;
  float   cost_of_living;   /* 1.0 = baseline */
  float   gdp_per_capita;
} civ_nation_t;

/* ── Resource profile (computed from owned tiles) ────────────────── */
typedef struct {
  uint16_t quantities[20];     /* one per CIV_RESOURCE_COUNT */
  uint8_t  best_quality[20];
  uint16_t total_resources;
  int      distinct_types;
} civ_nation_resource_profile_t;

/* ── Nation manager ────────────────────────────────────────────────── */
typedef struct {
  civ_nation_t  *nations;     /* heap-allocated dynamic array */
  int            count;
  int            capacity;
  int            player_nation_index;
} civ_nation_manager_t;

civ_nation_manager_t *civ_nation_manager_create(void);
void civ_nation_manager_destroy(civ_nation_manager_t *mgr);

/* Create all starting nations from borders data + nations_data */
void civ_nation_manager_init_from_data(civ_nation_manager_t *mgr,
                                       const void *nations_data,
                                       int map_w, int map_h,
                                       int default_count);

/* Legacy: 8 hardcoded nations for procedural fallback */
void civ_nation_manager_init_default(civ_nation_manager_t *mgr);

/* Claim territory tiles on the world map for a nation */
void civ_nation_claim_territory(civ_nation_t *nation, civ_map_t *map);
void civ_nation_claim_all_territories(civ_nation_manager_t *mgr, civ_map_t *map);

/* Claim territory from the borders tile map (pixel-accurate) */
void civ_nation_claim_from_borders(civ_nation_t *n, civ_map_t *map,
                                   int nation_idx, int map_w, int map_h);

/* Compute resource profile from owned tiles */
void civ_nation_calculate_resources(civ_nation_t *n, civ_map_t *map,
                                    const void *resource_map,
                                    civ_nation_resource_profile_t *out);

/* Get nation by ID or index */
civ_nation_t *civ_nation_get_by_id(civ_nation_manager_t *mgr, const char *id);
civ_nation_t *civ_nation_get_by_index(civ_nation_manager_t *mgr, int idx);

/* Check if a lon/lat point is inside a nation's territory */
bool civ_nation_contains_point(civ_nation_t *nation, float lon, float lat);
bool civ_nation_contains_tile(civ_nation_t *nation, int32_t tx, int32_t ty,
                              int32_t map_w, int32_t map_h);

/* Find nation owning a tile by checking owner_id on the tile itself */
civ_nation_t *civ_nation_find_owner(civ_nation_manager_t *mgr, civ_map_t *map,
                                    int32_t tx, int32_t ty);

/* Compute per-nation economic profile from territory + resources + population */
void civ_nation_compute_economy(civ_nation_t *n, civ_map_t *map,
                                const void *resource_map);

/* Compute economies for all nations and global aggregates */
void civ_nation_compute_all_economies(civ_nation_manager_t *mgr, civ_map_t *map,
                                       const void *resource_map,
                                       civ_nation_economy_t *global_out);

#ifdef __cplusplus
}
#endif
#endif
