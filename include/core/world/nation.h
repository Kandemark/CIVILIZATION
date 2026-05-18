/**
 * @file nation.h
 * @brief Nation definitions with territory, government structure, subdivisions
 *
 * Nations claim territory on the Earth map. Each nation has a full
 * government structure (dynamic political positions defined by its
 * constitution), administrative subdivisions, and territorial borders
 * that are rendered on the map.
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

#define CIV_NATION_NAME_MAX   64
#define CIV_NATION_ID_MAX     32
#define CIV_NATION_COUNT      8
#define CIV_SUBDIVISION_MAX   16

/* ── Territory region (lat/lon bounding box) ──────────────────────── */
typedef struct {
  float min_lon, max_lon;
  float min_lat, max_lat;
} civ_nation_region_t;

/* ── Nation ─────────────────────────────────────────────────────────── */
typedef struct {
  char                    id[CIV_NATION_ID_MAX];
  char                    name[CIV_NATION_NAME_MAX];
  uint32_t                color;
  uint32_t                color_accent;

  /* Territory regions that define this nation's borders */
  civ_nation_region_t  regions[8];
  int                     region_count;

  /* Capital location */
  float                   capital_lon, capital_lat;

  /* Government — fully dynamic, constitution-defined */
  civ_government_t       *government;

  /* Subdivisions (provinces/states/regions) */
  struct {
    char     name[64];
    char     id[32];
    uint32_t color;
    civ_nation_region_t region;
  } subdivisions[CIV_SUBDIVISION_MAX];
  int subdivision_count;

  /* Starting indices */
  int32_t tech_index;
  int32_t economic_index;
  int32_t military_index;
  int32_t cultural_index;

  /* Population */
  int64_t population;
} civ_nation_t;

/* ── Nation manager ────────────────────────────────────────────────── */
typedef struct {
  civ_nation_t  nations[CIV_NATION_COUNT];
  int           count;
  int           player_nation_index;
} civ_nation_manager_t;

civ_nation_manager_t *civ_nation_manager_create(void);
void civ_nation_manager_destroy(civ_nation_manager_t *mgr);

/* Create all starting nations with territory and government structures */
void civ_nation_manager_init_default(civ_nation_manager_t *mgr);

/* Claim territory tiles on the world map for a nation */
void civ_nation_claim_territory(civ_nation_t *nation, civ_map_t *map);
void civ_nation_claim_all_territories(civ_nation_manager_t *mgr, civ_map_t *map);

/* Get nation by ID or index */
civ_nation_t *civ_nation_get_by_id(civ_nation_manager_t *mgr, const char *id);
civ_nation_t *civ_nation_get_by_index(civ_nation_manager_t *mgr, int idx);

/* Check if a lon/lat point is inside a nation's territory */
bool civ_nation_contains_point(civ_nation_t *nation, float lon, float lat);
bool civ_nation_contains_tile(civ_nation_t *nation, int32_t tx, int32_t ty,
                              int32_t map_w, int32_t map_h);

#ifdef __cplusplus
}
#endif
#endif
