/**
 * @file role.h
 * @brief Role-based game system — what you can do depends on who you are
 *
 * Every citizen has a role. Roles define permitted actions, dashboard
 * views, boss/subordinate relationships, and trust metrics. The sidebar
 * navigation renders dynamically based on your current role.
 */
#ifndef CIV_CORE_ROLE_H
#define CIV_CORE_ROLE_H

#include "../common.h"
#include "../types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_ROLE_TITLE_MAX  64
#define CIV_ROLE_SCOPE_MAX  128
#define CIV_ACTION_NAME_MAX 32

/* ── Action permissions (bitmask) ───────────────────────────────── */
typedef enum {
  CIV_ACT_VIEW_DASHBOARD     = 1 << 0,
  CIV_ACT_WORK_TRADE         = 1 << 1,
  CIV_ACT_NETWORK            = 1 << 2,
  CIV_ACT_JOIN_PARTY         = 1 << 3,
  CIV_ACT_APPLY_POSITION     = 1 << 4,
  CIV_ACT_START_BUSINESS     = 1 << 5,
  CIV_ACT_PROTEST            = 1 << 6,
  CIV_ACT_VOTE               = 1 << 7,
  CIV_ACT_RESIGN             = 1 << 8,
  CIV_ACT_VIEW_BUDGET        = 1 << 9,
  CIV_ACT_ALLOCATE_BUDGET    = 1 << 10,
  CIV_ACT_PROPOSE_POLICY     = 1 << 11,
  CIV_ACT_HIRE_STAFF         = 1 << 12,
  CIV_ACT_FIRE_STAFF         = 1 << 13,
  CIV_ACT_DEPLOY_MILITARY    = 1 << 14,
  CIV_ACT_NEGOTIATE_TREATY   = 1 << 15,
  CIV_ACT_DECLARE_WAR        = 1 << 16,
  CIV_ACT_PASS_LEGISLATION   = 1 << 17,
  CIV_ACT_RULE_JUDGMENT      = 1 << 18,
  CIV_ACT_APPOINT_OFFICIAL   = 1 << 19,
  CIV_ACT_IMPEACH            = 1 << 20,
  CIV_ACT_VIEW_INTELLIGENCE  = 1 << 21,
  CIV_ACT_VIEW_NEWS          = 1 << 22,
  CIV_ACT_VIEW_ECONOMY       = 1 << 23,
  CIV_ACT_VIEW_MILITARY      = 1 << 24,
  CIV_ACT_VIEW_GOVERNANCE    = 1 << 25,
  CIV_ACT_VIEW_DIPLOMACY     = 1 << 26,
} civ_action_t;

/* ── Nav screen IDs (dynamic, role-dependent) ──────────────────── */
typedef enum {
  CIV_NAV_DASHBOARD,
  CIV_NAV_WORK,
  CIV_NAV_FINANCE,
  CIV_NAV_HOUSING,
  CIV_NAV_EDUCATION,
  CIV_NAV_NETWORK,
  CIV_NAV_POLITICS,
  CIV_NAV_HEALTH,
  CIV_NAV_BUDGET,
  CIV_NAV_POLICY,
  CIV_NAV_REPORTS,
  CIV_NAV_STAFF,
  CIV_NAV_FORCES,
  CIV_NAV_DEPLOYMENTS,
  CIV_NAV_INTELLIGENCE,
  CIV_NAV_NEWS,
  CIV_NAV_ECONOMY,
  CIV_NAV_GOVERNANCE,
  CIV_NAV_CONSTITUTION,
  CIV_NAV_MAP,
  CIV_NAV_DIPLOMACY,
  CIV_NAV_MILITARY,
  CIV_NAV_COUNT
} civ_nav_screen_t;

/* ── Role definition ────────────────────────────────────────────── */
typedef struct {
  char     title[CIV_ROLE_TITLE_MAX];
  char     scope[CIV_ROLE_SCOPE_MAX];     /* what this role oversees */
  uint64_t permitted_actions;             /* bitmask of civ_action_t */
  int      nav_screens[12];                /* which screens this role sees */
  int      nav_count;
  char     boss_title[CIV_ROLE_TITLE_MAX]; /* who you report to */
  char     department[48];                 /* which department */
  int      hierarchy_level;               /* 0=head of state, higher=lower */
} civ_role_def_t;

/* ── Player's current role state ────────────────────────────────── */
typedef struct {
  char     role_title[CIV_ROLE_TITLE_MAX];
  char     nation_id[32];
  char     boss_name[48];
  char     party_name[48];
  float    boss_trust;          /* 0-1, low = risk of shuffle/fire */
  float    party_alignment;     /* 0-1, how well you align with party */
  float    public_approval;     /* 0-1 */
  uint64_t permitted_actions;   /* current action set */
  int      nav_screens[12];      /* current nav */
  int      nav_count;
  bool     is_government_role;
} civ_player_role_t;

/* ── API ────────────────────────────────────────────────────────── */
void civ_role_init(civ_player_role_t *role);
void civ_role_set(civ_player_role_t *role, const civ_role_def_t *def,
                  const char *nation_id);
void civ_role_set_boss(civ_player_role_t *role, const char *boss_name,
                       float initial_trust);
bool civ_role_can_act(const civ_player_role_t *role, civ_action_t action);

/* Pre-defined role templates */
extern const civ_role_def_t civ_role_private_citizen;
extern const civ_role_def_t civ_role_cabinet_economics;
extern const civ_role_def_t civ_role_cabinet_defense;
extern const civ_role_def_t civ_role_diplomat;
extern const civ_role_def_t civ_role_legislator;
extern const civ_role_def_t civ_role_head_of_state;

const char *civ_nav_screen_label(civ_nav_screen_t screen);

#ifdef __cplusplus
}
#endif
#endif
