#include "core/economy/innovation_economy.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CIV_INNOVATION_INITIAL_SECTOR_CAP 8

civ_innovation_economy_t *civ_innovation_economy_create(void) {
  civ_innovation_economy_t *ie = CIV_MALLOC(sizeof(civ_innovation_economy_t));
  if (!ie) return NULL;
  memset(ie, 0, sizeof(*ie));
  ie->sector_capacity = CIV_INNOVATION_INITIAL_SECTOR_CAP;
  ie->sectors = CIV_MALLOC(sizeof(civ_innovation_sector_t) * ie->sector_capacity);
  ie->innovation_index = 0.35;
  ie->tech_commercialization = 0.25;
  ie->patent_productivity = 0.01;
  ie->startup_formation_rate = 0.5;
  return ie;
}

void civ_innovation_economy_destroy(civ_innovation_economy_t *ie) {
  if (!ie) return;
  free(ie->sectors);
  free(ie);
}

void civ_innovation_economy_update(civ_innovation_economy_t *ie, civ_float_t time_delta,
                                   civ_float_t gdp, civ_float_t education_level,
                                   civ_float_t business_confidence, civ_float_t regulation_level,
                                   civ_float_t capital_availability) {
  if (!ie) return;
  (void)time_delta;

  /* R&D intensity: fraction of GDP spent on research */
  ie->r_and_d_to_gdp = 0.01 + education_level * 0.03 + business_confidence * 0.01;
  ie->total_r_and_d = gdp * ie->r_and_d_to_gdp;

  /* Commercialization: education + confidence enable, regulation hinders */
  ie->tech_commercialization = education_level * 0.4 + business_confidence * 0.2
                               - regulation_level * 0.1 + 0.15;
  if (ie->tech_commercialization < 0.05) ie->tech_commercialization = 0.05;
  if (ie->tech_commercialization > 0.80) ie->tech_commercialization = 0.80;

  /* Patent productivity: R&D efficiency */
  ie->patent_productivity = 0.005 + ie->tech_commercialization * 0.02;

  /* Startup formation: capital availability + commercialization */
  ie->startup_formation_rate = capital_availability * 0.4 + ie->tech_commercialization * 0.3;

  /* Per-sector updates */
  for (int i = 0; i < ie->sector_count; i++) {
    civ_innovation_sector_t *s = &ie->sectors[i];

    /* R&D spending per sector */
    s->r_and_d_spending = ie->total_r_and_d / (ie->sector_count + 1);

    /* Innovation rate: R&D * patent productivity * education */
    s->innovation_rate = s->r_and_d_spending * ie->patent_productivity * (0.5 + education_level);
    s->active_patents += (int)(s->innovation_rate * 10.0);

    /* Tech diffusion: adoption rate affected by commercialization */
    s->tech_diffusion += (ie->tech_commercialization - s->tech_diffusion) * 0.2;

    /* Startups per sector */
    s->startups = (int)(ie->startup_formation_rate * 10.0 * (0.5 + s->tech_diffusion));
  }

  /* Composite innovation index */
  ie->innovation_index = ie->r_and_d_to_gdp * 10.0 * ie->tech_commercialization;
  if (ie->innovation_index > 1.0) ie->innovation_index = 1.0;
}

void civ_innovation_economy_add_sector(civ_innovation_economy_t *ie, const char *sector_name) {
  if (!ie || !sector_name) return;
  if (ie->sector_count >= ie->sector_capacity) {
    int nc = ie->sector_capacity * 2;
    civ_innovation_sector_t *tmp = CIV_REALLOC(ie->sectors, sizeof(civ_innovation_sector_t) * nc);
    if (!tmp) return;
    ie->sectors = tmp;
    ie->sector_capacity = nc;
  }
  civ_innovation_sector_t *s = &ie->sectors[ie->sector_count];
  memset(s, 0, sizeof(*s));
  strncpy(s->sector, sector_name, STRING_SHORT_LEN - 1);
  s->tech_diffusion = 0.1;
  ie->sector_count++;
}

civ_float_t civ_innovation_economy_productivity_gain(const civ_innovation_economy_t *ie) {
  if (!ie) return 0.0;
  return ie->innovation_index * 0.05; /* 0-5% productivity gain per cycle */
}

civ_float_t civ_innovation_economy_tech_diffusion_rate(const civ_innovation_economy_t *ie) {
  return ie ? ie->tech_commercialization : 0.0;
}
