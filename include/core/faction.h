/**
 * @file faction.h
 * @brief Faction archetypes — starting index positions, never static bonuses
 *
 * Dominion is index-driven. Factions define where you start on
 * continuous measurement scales. Everything after turn 1 is emergent
 * from your decisions and the simulation systems.
 *
 * No faction has a permanent "+X%" modifier. Comparative terms
 * ("25% ahead of regional average") are computed dynamically from
 * global statistics and displayed in data panels, never hardcoded.
 */
#ifndef CIV_CORE_FACTION_H
#define CIV_CORE_FACTION_H

#include "../common.h"
#include "../ui/icon/icon_atlas.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_FACTION_NAME_MAX 32
#define CIV_FACTION_DESC_MAX 256
#define CIV_FACTION_ID_MAX   32
#define CIV_FACTION_COUNT    8

/* ── Economic sectors (each nation has a distribution across these) ── */
#define CIV_SECTOR_COUNT 8

typedef enum {
  CIV_SECTOR_AGRICULTURE,
  CIV_SECTOR_EXTRACTION,     /* mining, logging, fishing */
  CIV_SECTOR_MANUFACTURING,
  CIV_SECTOR_SERVICES,
  CIV_SECTOR_FINANCE,
  CIV_SECTOR_TRADE,
  CIV_SECTOR_RESEARCH,
  CIV_SECTOR_GOVERNANCE,
} civ_sector_t;

/* ── Starting conditions for a faction ─────────────────────────────── */
typedef struct {
  /* Starting position on continuous indices (0 = baseline, can be negative) */
  int32_t tech_index;         /* Scientific/innovation capability */
  int32_t military_index;     /* Military organization & readiness */
  int32_t economic_index;     /* Aggregate economic output */
  int32_t cultural_index;     /* Cultural influence & cohesion */
  int32_t stability_index;    /* Internal stability */
  int32_t population_index;   /* Population size & density */

  /* Initial economic sector distribution (fractions, must sum to ~1.0) */
  float sector_weights[CIV_SECTOR_COUNT];

  /* Power distribution in initial government (0.0–1.0 per branch concept) */
  float executive_power;      /* Concentration of decision authority */
  float legislative_power;    /* Collective decision-making weight */
  float judicial_power;       /* Rule enforcement independence */

  /* Geographic preference */
  const char *preferred_terrain;
  int32_t     preferred_latitude;  /* -90 (south pole) to +90 (north pole) */

  /* Initial settlement count and spread */
  int32_t starting_settlements;
  float   starting_territory_radius;
} civ_faction_start_t;

/* ── Faction definition ──────────────────────────────────────────────── */
typedef struct {
  char                id[CIV_FACTION_ID_MAX];
  char                name[CIV_FACTION_NAME_MAX];
  char                description[CIV_FACTION_DESC_MAX];
  uint32_t            color;
  uint32_t            color_accent;
  civ_icon_id_t       icon;
  civ_faction_start_t start;
} civ_faction_t;

const civ_faction_t *civ_faction_get_all(void);
const civ_faction_t *civ_faction_get(int index);
const civ_faction_t *civ_faction_get_by_id(const char *id);
int                  civ_faction_count(void);

#ifdef __cplusplus
}
#endif
#endif
