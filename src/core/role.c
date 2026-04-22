/**
 * @file role.c
 * @brief Role system — pre-defined roles, dynamic nav, action permissions
 */
#include "core/role.h"
#include <string.h>

const civ_role_def_t civ_role_private_citizen = {
  .title = "Private Citizen",
  .scope = "Personal affairs, work, and local community",
  .permitted_actions =
      CIV_ACT_VIEW_DASHBOARD | CIV_ACT_WORK_TRADE | CIV_ACT_NETWORK |
      CIV_ACT_JOIN_PARTY | CIV_ACT_APPLY_POSITION | CIV_ACT_START_BUSINESS |
      CIV_ACT_PROTEST | CIV_ACT_VOTE | CIV_ACT_RESIGN |
      CIV_ACT_VIEW_NEWS | CIV_ACT_VIEW_ECONOMY,
  .nav_screens = {CIV_NAV_DASHBOARD, CIV_NAV_MAP, CIV_NAV_WORK, CIV_NAV_FINANCE,
                  CIV_NAV_HOUSING, CIV_NAV_EDUCATION, CIV_NAV_NETWORK,
                  CIV_NAV_POLITICS, CIV_NAV_HEALTH, CIV_NAV_CONSTITUTION,
                  CIV_NAV_NEWS, CIV_NAV_ECONOMY},
  .nav_count = 12, .boss_title = "", .department = "Civil Society", .hierarchy_level = 99,
};
/* Keep other roles as-is for now */
const civ_role_def_t civ_role_cabinet_economics = {
  .title = "Cabinet Secretary of Economics",
  .scope = "National economic policy, budget, trade, and sector regulation",
  .permitted_actions = CIV_ACT_VIEW_DASHBOARD | CIV_ACT_VIEW_BUDGET | CIV_ACT_ALLOCATE_BUDGET |
      CIV_ACT_PROPOSE_POLICY | CIV_ACT_HIRE_STAFF | CIV_ACT_FIRE_STAFF |
      CIV_ACT_NETWORK | CIV_ACT_RESIGN | CIV_ACT_VIEW_NEWS |
      CIV_ACT_VIEW_ECONOMY | CIV_ACT_VIEW_GOVERNANCE,
  .nav_screens = {CIV_NAV_DASHBOARD, CIV_NAV_BUDGET, CIV_NAV_POLICY,
                  CIV_NAV_STAFF, CIV_NAV_REPORTS, CIV_NAV_NEWS, -1},
  .nav_count = 6, .boss_title = "Head of Government", .department = "Ministry of Economics", .hierarchy_level = 2,
};
const civ_role_def_t civ_role_cabinet_defense = {
  .title = "Minister of Defense", .scope = "National security and military operations",
  .permitted_actions = CIV_ACT_VIEW_DASHBOARD | CIV_ACT_DEPLOY_MILITARY |
      CIV_ACT_PROPOSE_POLICY | CIV_ACT_HIRE_STAFF | CIV_ACT_FIRE_STAFF |
      CIV_ACT_RESIGN | CIV_ACT_VIEW_NEWS | CIV_ACT_VIEW_INTELLIGENCE |
      CIV_ACT_VIEW_MILITARY | CIV_ACT_VIEW_BUDGET,
  .nav_screens = {CIV_NAV_DASHBOARD, CIV_NAV_FORCES, CIV_NAV_DEPLOYMENTS,
                  CIV_NAV_INTELLIGENCE, CIV_NAV_BUDGET, CIV_NAV_NEWS, -1},
  .nav_count = 6, .boss_title = "Head of Government", .department = "Ministry of Defense", .hierarchy_level = 2,
};
const civ_role_def_t civ_role_diplomat = {
  .title = "Ambassador", .scope = "Foreign relations and treaty negotiation",
  .permitted_actions = CIV_ACT_VIEW_DASHBOARD | CIV_ACT_NEGOTIATE_TREATY |
      CIV_ACT_NETWORK | CIV_ACT_RESIGN | CIV_ACT_VIEW_NEWS |
      CIV_ACT_VIEW_DIPLOMACY | CIV_ACT_PROPOSE_POLICY,
  .nav_screens = {CIV_NAV_DASHBOARD, CIV_NAV_DIPLOMACY, CIV_NAV_REPORTS,
                  CIV_NAV_NETWORK, CIV_NAV_NEWS, -1},
  .nav_count = 5, .boss_title = "Foreign Minister", .department = "Ministry of Foreign Affairs", .hierarchy_level = 3,
};
const civ_role_def_t civ_role_legislator = {
  .title = "Legislator", .scope = "Law-making and oversight",
  .permitted_actions = CIV_ACT_VIEW_DASHBOARD | CIV_ACT_PASS_LEGISLATION |
      CIV_ACT_VOTE | CIV_ACT_PROPOSE_POLICY | CIV_ACT_NETWORK |
      CIV_ACT_RESIGN | CIV_ACT_VIEW_NEWS | CIV_ACT_VIEW_GOVERNANCE |
      CIV_ACT_VIEW_BUDGET | CIV_ACT_IMPEACH,
  .nav_screens = {CIV_NAV_DASHBOARD, CIV_NAV_POLICY, CIV_NAV_GOVERNANCE,
                  CIV_NAV_POLITICS, CIV_NAV_BUDGET, CIV_NAV_NEWS, -1},
  .nav_count = 6, .boss_title = "Speaker", .department = "Legislature", .hierarchy_level = 2,
};
const civ_role_def_t civ_role_head_of_state = {
  .title = "Head of State", .scope = "Ultimate executive authority",
  .permitted_actions = CIV_ACT_VIEW_DASHBOARD | CIV_ACT_ALLOCATE_BUDGET |
      CIV_ACT_PROPOSE_POLICY | CIV_ACT_HIRE_STAFF | CIV_ACT_FIRE_STAFF |
      CIV_ACT_DEPLOY_MILITARY | CIV_ACT_NEGOTIATE_TREATY |
      CIV_ACT_DECLARE_WAR | CIV_ACT_PASS_LEGISLATION |
      CIV_ACT_APPOINT_OFFICIAL | CIV_ACT_IMPEACH | CIV_ACT_RESIGN |
      CIV_ACT_VIEW_INTELLIGENCE | CIV_ACT_VIEW_NEWS |
      CIV_ACT_VIEW_ECONOMY | CIV_ACT_VIEW_MILITARY | CIV_ACT_VIEW_GOVERNANCE | CIV_ACT_NETWORK,
  .nav_screens = {CIV_NAV_DASHBOARD, CIV_NAV_ECONOMY, CIV_NAV_MILITARY,
                  CIV_NAV_DIPLOMACY, CIV_NAV_GOVERNANCE, CIV_NAV_INTELLIGENCE,
                  CIV_NAV_STAFF, CIV_NAV_NEWS, -1},
  .nav_count = 8, .boss_title = "", .department = "Executive Office", .hierarchy_level = 0,
};

static const char *s_nav_labels[CIV_NAV_COUNT] = {
    "Dashboard", "Work", "Finance", "Housing",
    "Education", "Network", "Politics", "Health",
    "Budget", "Policy", "Reports", "Staff",
    "Forces", "Deployments", "Intelligence",
    "News", "Economy", "Governance", "Constitution",
    "Map", "Diplomacy", "Military",
};
const char *civ_nav_screen_label(civ_nav_screen_t s) {
  return (s < CIV_NAV_COUNT) ? s_nav_labels[s] : "Unknown";
}

void civ_role_init(civ_player_role_t *r) { memset(r, 0, sizeof(*r)); }
void civ_role_set(civ_player_role_t *r, const civ_role_def_t *def, const char *nid) {
  if (!r || !def) return;
  strncpy(r->role_title, def->title, CIV_ROLE_TITLE_MAX-1);
  strncpy(r->nation_id, nid?nid:"", sizeof(r->nation_id)-1);
  r->permitted_actions = def->permitted_actions;
  r->nav_count = def->nav_count;
  memcpy(r->nav_screens, def->nav_screens, sizeof(r->nav_screens));
  r->is_government_role = (def->hierarchy_level < 10);
  r->boss_trust = 0.5f; r->party_alignment = 0.5f; r->public_approval = 0.5f;
}
void civ_role_set_boss(civ_player_role_t *r, const char *name, float trust) {
  if (!r) return;
  strncpy(r->boss_name, name, sizeof(r->boss_name)-1); r->boss_trust = trust;
}
bool civ_role_can_act(const civ_player_role_t *r, civ_action_t act) {
  return r && (r->permitted_actions & (uint64_t)act);
}
