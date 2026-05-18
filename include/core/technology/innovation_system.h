/**
 * @file innovation_system.h
 * @brief Continuous technology indices — no discovery, no caps, no tree
 *
 * Technology is measured on continuous indices per domain that rise
 * indefinitely. There is no "discovered" state — every domain is always
 * advancing. Nations have relative positions (e.g., one at +536,
 * another at -100) displayed in global rankings.
 */
#ifndef CIVILIZATION_INNOVATION_SYSTEM_H
#define CIVILIZATION_INNOVATION_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_TECH_DOMAIN_COUNT 12

typedef enum {
  CIV_TECH_WRITING,
  CIV_TECH_ENGINEERING,
  CIV_TECH_AGRICULTURE,
  CIV_TECH_MILITARY_SCIENCE,
  CIV_TECH_MEDICINE,
  CIV_TECH_ASTRONOMY,
  CIV_TECH_METALLURGY,
  CIV_TECH_NAVIGATION,
  CIV_TECH_ARCHITECTURE,
  CIV_TECH_PHILOSOPHY,
  CIV_TECH_COMMERCE,
  CIV_TECH_GOVERNANCE_THEORY,
} civ_tech_domain_t;

typedef struct {
  char    name[64];
  int32_t index;           /* continuous, rises indefinitely, can be negative */
  float   research_alloc;  /* fraction of budget allocated */
  float   growth_rate;     /* current increase per turn */
  int32_t global_rank;     /* position among all nations (1 = leader) */
  int32_t global_average;  /* current global average index for this domain */
} civ_tech_domain_state_t;

typedef struct {
  float                   total_budget;    /* science output per turn */
  civ_tech_domain_state_t domains[CIV_TECH_DOMAIN_COUNT];
  int32_t                 aggregate_index; /* weighted average across domains */
  int32_t                 aggregate_rank;  /* composite global ranking */
  float                   allocation[CIV_TECH_DOMAIN_COUNT]; /* player-set allocation */
} civ_innovation_system_t;

civ_innovation_system_t *civ_innovation_system_create(void);
void civ_innovation_system_destroy(civ_innovation_system_t *is);

/* Process one turn of advancement */
void civ_innovation_system_update(civ_innovation_system_t *is, float dt);

/* Set total research budget (comes from population, buildings, trade) */
void civ_innovation_system_set_research_budget(civ_innovation_system_t *is,
                                               float budget);

/* Set allocation fraction for one domain */
void civ_innovation_system_set_allocation(civ_innovation_system_t *is,
                                          civ_tech_domain_t domain, float fraction);

/* Query */
int32_t civ_innovation_system_get_index(civ_innovation_system_t *is,
                                        civ_tech_domain_t domain);
const char *civ_tech_domain_name(civ_tech_domain_t domain);

/* Initialize domains with starting index values */
void civ_innovation_system_init_domains(civ_innovation_system_t *is,
                                        int32_t starting_indices[12]);

#ifdef __cplusplus
}
#endif
#endif
