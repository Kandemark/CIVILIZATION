/**
 * @file constitution.c
 * @brief Constitution system — per-nation rules governing all actions
 */
#include "core/constitution.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *s_rule_names[CIV_RULE_COUNT] = {
  "Employment", "Housing", "Education", "Political Party",
  "Run for Office", "Business", "Currency Use", "Free Speech",
  "Assembly", "Religion", "Military Service", "Taxation",
  "Healthcare", "Banking", "Travel", "Voting",
};
static const char *s_perm_names[] = {"Prohibited","Restricted","Allowed","Protected Right"};

const char *civ_rule_category_name(civ_rule_category_t c) {
  return (c<CIV_RULE_COUNT)?s_rule_names[c]:"Unknown";
}
const char *civ_perm_level_name(civ_perm_level_t l) {
  return (l<4)?s_perm_names[l]:"Unknown";
}

civ_national_constitution_t *civ_national_constitution_create(const char *nation_id) {
  civ_national_constitution_t *c = (civ_national_constitution_t*)malloc(sizeof(civ_national_constitution_t));
  if (!c) return NULL;
  civ_national_constitution_init(c, nation_id);
  return c;
}
void civ_national_constitution_destroy(civ_national_constitution_t *c) { free(c); }

void civ_national_constitution_init(civ_national_constitution_t *c, const char *nid) {
  if (!c) return; memset(c, 0, sizeof(*c));
  snprintf(c->name, sizeof(c->name), "Constitution of %s", nid?nid:"the Nation");
  if (nid) strncpy(c->nation_id, nid, sizeof(c->nation_id)-1);
  c->ratified_year = 0; c->amend_threshold = 0.60f; c->can_suspend = false;

  /* Default: moderate liberal democracy — most things allowed */
  for (int i = 0; i < CIV_RULE_COUNT; i++) {
    c->rules[i].category = (civ_rule_category_t)i;
    c->rules[i].level = CIV_PERM_ALLOWED;
    c->rules[i].tax_rate = 0.15f;  /* 15% default tax */
    c->rules[i].minimum_age = 18.0f;
    c->rules[i].cost = 0.0f;
    snprintf(c->rules[i].description, sizeof(c->rules[i].description),
             "%s: %s", s_rule_names[i], s_perm_names[CIV_PERM_ALLOWED]);
  }
  /* Some defaults are restricted */
  c->rules[CIV_RULE_RUN_FOR_OFFICE].minimum_age = 25.0f;
  c->rules[CIV_RULE_BUSINESS].level = CIV_PERM_RESTRICTED;
  c->rules[CIV_RULE_BUSINESS].cost = 500.0f;
  c->rules[CIV_RULE_MILITARY_SERVICE].level = CIV_PERM_RESTRICTED;
  c->rules[CIV_RULE_CURRENCY_USE].level = CIV_PERM_ALLOWED;
  c->rules[CIV_RULE_VOTING].level = CIV_PERM_PROTECTED_RIGHT;
}

bool civ_national_constitution_check(const civ_national_constitution_t *c, civ_rule_category_t cat,
                            float actor_age, float actor_wealth) {
  if (!c || cat >= CIV_RULE_COUNT) return false;
  const civ_national_constitution_rule_t *r = &c->rules[cat];
  if (r->level == CIV_PERM_PROHIBITED) return false;
  if (r->level == CIV_PERM_PROTECTED_RIGHT) return true;
  if (actor_age < r->minimum_age) return false;
  if (r->level == CIV_PERM_RESTRICTED && r->cost > 0 && actor_wealth < r->cost)
    return false;
  return true;
}

civ_perm_level_t civ_national_constitution_get(const civ_national_constitution_t *c,
                                      civ_rule_category_t cat) {
  if (!c || cat >= CIV_RULE_COUNT) return CIV_PERM_PROHIBITED;
  return c->rules[cat].level;
}

void civ_national_constitution_amend(civ_national_constitution_t *c, civ_rule_category_t cat,
                            civ_perm_level_t new_level, const char *reason) {
  if (!c || cat >= CIV_RULE_COUNT) return;
  c->rules[cat].level = new_level;
  snprintf(c->rules[cat].description, sizeof(c->rules[cat].description),
           "%s: %s (%s)", s_rule_names[cat], s_perm_names[new_level],
           reason?reason:"amended");
}
