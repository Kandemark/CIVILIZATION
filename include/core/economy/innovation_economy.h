/**
 * @file innovation_economy.h
 * @brief Innovation economy — R&D investment, tech commercialization, patents, startups
 */
#ifndef CIV_ECONOMY_INNOVATION_ECONOMY_H
#define CIV_ECONOMY_INNOVATION_ECONOMY_H

#include "../../common.h"
#include "../../types.h"

typedef struct {
  char        sector[STRING_SHORT_LEN];
  civ_float_t r_and_d_spending;        /* total R&D investment */
  civ_float_t innovation_rate;          /* breakthroughs per cycle */
  civ_float_t tech_diffusion;           /* 0.0-1.0, how widely adopted */
  int         active_patents;
  int         startups;
} civ_innovation_sector_t;

typedef struct {
  civ_innovation_sector_t *sectors;
  int                      sector_count;
  int                      sector_capacity;
  civ_float_t              total_r_and_d;          /* aggregate R&D spend */
  civ_float_t              r_and_d_to_gdp;         /* R&D intensity */
  civ_float_t              innovation_index;       /* composite 0.0-1.0 */
  civ_float_t              tech_commercialization; /* fraction that reaches market */
  civ_float_t              patent_productivity;    /* patents per R&D unit */
  civ_float_t              startup_formation_rate;
} civ_innovation_economy_t;

civ_innovation_economy_t *civ_innovation_economy_create(void);
void civ_innovation_economy_destroy(civ_innovation_economy_t *ie);
void civ_innovation_economy_update(civ_innovation_economy_t *ie, civ_float_t time_delta,
                                   civ_float_t gdp, civ_float_t education_level,
                                   civ_float_t business_confidence, civ_float_t regulation_level,
                                   civ_float_t capital_availability);

void civ_innovation_economy_add_sector(civ_innovation_economy_t *ie, const char *sector_name);
civ_float_t civ_innovation_economy_productivity_gain(const civ_innovation_economy_t *ie);
civ_float_t civ_innovation_economy_tech_diffusion_rate(const civ_innovation_economy_t *ie);

#endif
