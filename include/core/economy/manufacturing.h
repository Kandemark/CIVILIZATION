/**
 * @file manufacturing.h
 * @brief Manufacturing — industry, production chains, factory output, automation
 */
#ifndef CIV_ECONOMY_MANUFACTURING_H
#define CIV_ECONOMY_MANUFACTURING_H

#include "../../common.h"
#include "../../types.h"

typedef struct {
  char        id[STRING_SHORT_LEN];
  char        product[STRING_SHORT_LEN];
  civ_float_t output_per_worker;
  civ_float_t capital_intensity;    /* 0.0-1.0, higher = more automated */
  civ_float_t raw_material_ratio;   /* units of raw material per unit output */
  int         workers;
  int         capacity;
  bool        active;
} civ_factory_t;

typedef struct {
  civ_factory_t *factories;
  int             factory_count;
  int             factory_capacity;
  civ_float_t     total_industrial_output;
  civ_float_t     capacity_utilization;   /* 0.0-1.0 */
  civ_float_t     automation_level;       /* 0.0-1.0 */
  civ_float_t     supply_chain_efficiency; /* 0.0-1.0 */
} civ_manufacturing_system_t;

civ_manufacturing_system_t *civ_manufacturing_create(void);
void civ_manufacturing_destroy(civ_manufacturing_system_t *m);
void civ_manufacturing_update(civ_manufacturing_system_t *m, civ_float_t time_delta,
                              civ_float_t tech_level, civ_float_t labor_available,
                              civ_float_t raw_materials_available, civ_float_t infrastructure_quality);

civ_factory_t *civ_manufacturing_add_factory(civ_manufacturing_system_t *m,
                                             const char *product, int capacity);
void civ_manufacturing_set_workers(civ_factory_t *f, int workers);
civ_float_t civ_manufacturing_productivity(const civ_manufacturing_system_t *m);

#endif
