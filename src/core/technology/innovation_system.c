/**
 * @file innovation_system.c
 * @brief Continuous technology indices — no discovery caps, infinite advancement
 */
#include "core/technology/innovation_system.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static const char *s_domain_names[CIV_TECH_DOMAIN_COUNT] = {
    "Writing & Recordkeeping",   "Engineering & Construction",
    "Agriculture & Husbandry",   "Military Science",
    "Medicine & Healing",        "Astronomy & Mathematics",
    "Metallurgy & Materials",    "Navigation & Cartography",
    "Architecture & Planning",   "Philosophy & Ethics",
    "Commerce & Currency",       "Governance Theory",
};

civ_innovation_system_t *civ_innovation_system_create(void) {
  civ_innovation_system_t *is = (civ_innovation_system_t *)CIV_MALLOC(
      sizeof(civ_innovation_system_t));
  if (!is) return NULL;
  memset(is, 0, sizeof(*is));

  for (int i = 0; i < CIV_TECH_DOMAIN_COUNT; i++) {
    strncpy(is->domains[i].name, s_domain_names[i],
            sizeof(is->domains[i].name) - 1);
    is->domains[i].index = 0;
    is->allocation[i] = 1.0f / (float)CIV_TECH_DOMAIN_COUNT;
  }
  return is;
}

void civ_innovation_system_destroy(civ_innovation_system_t *is) {
  CIV_FREE(is);
}

void civ_innovation_system_init_domains(civ_innovation_system_t *is,
                                        int32_t starting_indices[12]) {
  if (!is || !starting_indices) return;
  for (int i = 0; i < CIV_TECH_DOMAIN_COUNT; i++)
    is->domains[i].index = starting_indices[i];
}

void civ_innovation_system_set_research_budget(civ_innovation_system_t *is,
                                               float budget) {
  if (!is) return;
  is->total_budget = budget > 0.0f ? budget : 0.0f;
}

void civ_innovation_system_set_allocation(civ_innovation_system_t *is,
                                          civ_tech_domain_t domain,
                                          float fraction) {
  if (!is || domain >= CIV_TECH_DOMAIN_COUNT) return;
  if (fraction < 0.0f) fraction = 0.0f;
  if (fraction > 1.0f) fraction = 1.0f;
  is->allocation[domain] = fraction;
}

int32_t civ_innovation_system_get_index(civ_innovation_system_t *is,
                                        civ_tech_domain_t domain) {
  if (!is || domain >= CIV_TECH_DOMAIN_COUNT) return 0;
  return is->domains[domain].index;
}

const char *civ_tech_domain_name(civ_tech_domain_t domain) {
  if (domain >= CIV_TECH_DOMAIN_COUNT) return "Unknown";
  return s_domain_names[domain];
}

void civ_innovation_system_update(civ_innovation_system_t *is, float dt) {
  if (!is) return;

  for (int i = 0; i < CIV_TECH_DOMAIN_COUNT; i++) {
    float budget_share = is->total_budget * is->allocation[i];
    /* Diminishing returns: harder to advance at higher levels */
    float diminishing = 1.0f / (1.0f + fabsf((float)is->domains[i].index) / 500.0f);
    float growth = budget_share * diminishing * 0.1f * dt;
    is->domains[i].growth_rate = growth;
    is->domains[i].index += (int32_t)growth;
  }

  /* Aggregate index: weighted average across all domains */
  float weighted_sum = 0.0f, total_weight = 0.0f;
  for (int i = 0; i < CIV_TECH_DOMAIN_COUNT; i++) {
    weighted_sum += (float)is->domains[i].index * is->allocation[i];
    total_weight += is->allocation[i];
  }
  is->aggregate_index =
      (int32_t)(weighted_sum / (total_weight > 0.0f ? total_weight : 1.0f));

  /* Rankings — placeholder, real ranking compares against AI nations */
  is->aggregate_rank = 1;
  for (int i = 0; i < CIV_TECH_DOMAIN_COUNT; i++) {
    is->domains[i].global_average = 200;
    is->domains[i].global_rank = 1;
  }
}
