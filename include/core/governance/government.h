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
 *
 * This is the governance HUB — all governance subsystems are owned here
 * and ticked from civ_government_update.
 */
#ifndef CIVILIZATION_GOVERNMENT_H
#define CIVILIZATION_GOVERNMENT_H

#include "../../common.h"
#include "../../types.h"
#include "branches/council.h"
#include "branches/executive.h"
#include "branches/judiciary.h"
#include "branches/legislative.h"
#include "branches/religious_body.h"
#include "evolution/governance_evolution.h"
#include "institutions/civil_service.h"
#include "institutions/institution.h"
#include "institutions/ministry.h"
#include "interaction/conversation.h"
#include "interaction/interaction.h"
#include "interaction/notebook.h"
#include "legal/constitution.h"
#include "legal/legal_status.h"
#include "legal/rights.h"
#include "metrics/societal_metrics.h"
#include "political/corruption.h"
#include "political/elections.h"
#include "political/political_violence.h"
#include "territorial/subdivision.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_POSITION_TITLE_MAX  64
#define CIV_POSITION_ROLE_MAX   128
#define CIV_POSITION_SELECT_MAX 32
#define CIV_POSITION_TERM_MAX   32

/* Budget allocation categories matching civ_budget_category_t */
#define CIV_GOV_BUDGET_CATEGORIES 9

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

/* ── Government structure — the governance HUB ────────────────────── */
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

  /* ── Governance subsystems (all ticked from civ_government_update) ── */
  civ_constitution_t           *constitution;        /* dynamic rule-of-law foundation */

  /* ── Branches (all optional — NULL means "doesn't exist here") ── */
  civ_executive_t              *executive;           /* president, PM, monarch */
  civ_legislative_manager_t    *legislative_manager; /* parliament, senate, assembly */
  civ_judiciary_t              *judiciary;           /* courts, tribunals */
  civ_council_t                *council;             /* collective: junta, presidium, elders */
  civ_religious_body_t         *religious_body;      /* theocracy, guardian council */

  civ_institution_manager_t    *institution_manager;
  civ_subdivision_manager_t    *subdivision_manager;
  civ_corruption_engine_t      *corruption_engine;
  civ_election_system_t        *election_system;
  civ_political_violence_t     *political_violence;
  civ_ministry_manager_t       *ministry_manager;
  civ_civil_service_t          *civil_service;
  civ_rights_declaration_t     *rights;
  civ_notebook_t               *notebook;
  civ_governance_state_t        evolution_state;    /* embedded — traits + decisions */
  civ_societal_health_t         societal_health;    /* embedded — dashboard snapshot */

  /* Budget allocation across governance functions (from budget module) */
  float budget_allocations[CIV_GOV_BUDGET_CATEGORIES];
  float legislative_threshold;

  /* Legislative session cycle */
  int   turns_since_last_session;

  /* Cross-module cached values (updated each tick from game.c params) */
  float faction_support;
  int   faction_count;

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

/* Main tick — propagates to ALL governance subsystems.
 * Cross-module params enable dynamic behavior without hardcoding.
 * Pass 0/NULL for systems that don't exist yet. */
void  civ_government_update(civ_government_t *gov, float time_delta,
                            int total_population, float culture_level,
                            float total_budget, float education_level,
                            float economic_confidence, float literacy_rate,
                            float faction_support,     /* avg faction support 0-1 */
                            int   faction_count);       /* number of active factions */
float civ_government_get_stability(const civ_government_t *gov);

/* ── Corruption (single source of truth) ──────────────────────────── */
float civ_government_get_corruption(const civ_government_t *gov);

/* ── Budget integration ────────────────────────────────────────────── */
void  civ_government_set_budget(civ_government_t *gov, int category, float amount);

/* ── Legislative trigger ──────────────────────────────────────────── */
void  civ_government_hold_session(civ_government_t *gov);

#ifdef __cplusplus
}
#endif
#endif
