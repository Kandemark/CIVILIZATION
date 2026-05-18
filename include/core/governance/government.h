/**
 * @file government.h
 * @brief Dynamic governance — no fixed types, structure defines the label
 *
 * Government types (Monarchy, Democracy, etc.) are NOT predefined.
 * Each nation has a list of named political positions from bottom to top
 * defined by its constitution. Proximity labels ("authoritarian",
 * "representative") are computed from the actual power distribution.
 *
 * No government structure is inherently "bad" — metrics like citizen
 * happiness, stability, and governance ranking are measured independently.
 */
#ifndef CIVILIZATION_GOVERNMENT_H
#define CIVILIZATION_GOVERNMENT_H

#include "../../common.h"
#include "../../types.h"
#include "institution.h"
#include "legislative_system.h"
#include "subdivision.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_POSITION_TITLE_MAX  64
#define CIV_POSITION_ROLE_MAX   128
#define CIV_POSITION_SELECT_MAX 32
#define CIV_POSITION_TERM_MAX   32

/* ── Political position — one role in the government hierarchy ────── */
typedef struct {
  char   title[CIV_POSITION_TITLE_MAX];
  char   role_description[CIV_POSITION_ROLE_MAX];
  int    hierarchy_level;      /* 0 = head of state, increasing = lower */
  float  executive_weight;     /* decision/implementation power weight */
  float  legislative_weight;   /* law-making power weight */
  float  judicial_weight;      /* review/adjudication power weight */
  char   selection_method[CIV_POSITION_SELECT_MAX];
  char   term[CIV_POSITION_TERM_MAX];
  int    position_count;       /* how many people hold this position */
  int    current_occupant;     /* entity ID occupying it (0 = vacant) */
} civ_political_position_t;

/* ── Computed governance descriptors ──────────────────────────────── */
typedef struct {
  float authority_concentration;   /* 0=fully distributed, 1=absolute single */
  float representation_index;      /* 0=no citizen input, 1=full input */
  float institutional_rigidity;    /* 0=easy to reform, 1=entrenched */
  float power_balance;             /* 0=one-branch-dominant, 1=balanced */
  float citizen_happiness;         /* 0.0–1.0 measured, not assumed */
  float governance_ranking;        /* relative global index */
} civ_governance_profile_t;

/* ── Global Stature Tiers (computed, not assigned) ────────────────── */
typedef enum {
  CIV_STATURE_FAILED_STATE = 0,
  CIV_STATURE_FRONTIER_NATION,
  CIV_STATURE_DEVELOPING_STATE,
  CIV_STATURE_STABLE_STATE,
  CIV_STATURE_REGIONAL_POWER,
  CIV_STATURE_GREAT_POWER,
  CIV_STATURE_HEGEMON,
} civ_stature_tier_t;

/* ── Government structure ─────────────────────────────────────────── */
typedef struct civ_government {
  char   id[STRING_SHORT_LEN];
  char   name[STRING_MEDIUM_LEN];

  /* Dynamic political positions from head-of-state down */
  civ_political_position_t *positions;
  size_t                    position_count;
  size_t                    position_capacity;

  /* Computed profile (updated each tick from structure) */
  civ_governance_profile_t profile;

  /* Core metrics */
  float stability;
  float legitimacy;
  float efficiency;

  /* Subsystems */
  civ_institution_manager_t  *institution_manager;
  civ_subdivision_manager_t  *subdivision_manager;
  civ_legislative_manager_t  *legislative_manager;
  float                       legislative_threshold;

  civ_stature_tier_t stature_tier;
} civ_government_t;

/* ── API ───────────────────────────────────────────────────────────── */
civ_government_t *civ_government_create(const char *name);
void              civ_government_destroy(civ_government_t *gov);

/* Add a political position to the government structure */
civ_political_position_t *civ_government_add_position(
    civ_government_t *gov, const char *title, int level,
    float exec_w, float leg_w, float jud_w,
    const char *selection, const char *term, int count);

/* Compute governance profile from actual political structure */
void civ_government_recompute_profile(civ_government_t *gov);

/* Get a descriptive proximity label (computed, not fixed) */
const char *civ_government_proximity_label(const civ_government_t *gov);

float civ_government_collect_taxes(civ_government_t *gov);
void  civ_government_update(civ_government_t *gov, float time_delta);
float civ_government_get_stability(const civ_government_t *gov);

#ifdef __cplusplus
}
#endif
#endif
