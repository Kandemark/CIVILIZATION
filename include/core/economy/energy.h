/**
 * @file energy.h
 * @brief Energy — fuel, electricity, power grid, energy independence
 */
#ifndef CIV_ECONOMY_ENERGY_H
#define CIV_ECONOMY_ENERGY_H

#include "../../common.h"
#include "../../types.h"

typedef enum { CIV_ENERGY_COAL, CIV_ENERGY_OIL, CIV_ENERGY_NATURAL_GAS,
               CIV_ENERGY_NUCLEAR, CIV_ENERGY_SOLAR, CIV_ENERGY_WIND,
               CIV_ENERGY_HYDRO, CIV_ENERGY_GEOTHERMAL, CIV_ENERGY_TYPE_COUNT } civ_energy_source_t;

typedef struct {
  civ_energy_source_t source;
  civ_float_t         capacity_mw;
  civ_float_t         current_output_mw;
  civ_float_t         efficiency;          /* 0.0-1.0 */
  civ_float_t         cost_per_mwh;
  civ_float_t         carbon_intensity;    /* emissions per MWh */
  bool                is_renewable;
} civ_power_plant_t;

typedef struct {
  civ_power_plant_t *plants;
  int                 plant_count;
  int                 plant_capacity;
  civ_float_t         total_capacity_mw;
  civ_float_t         total_demand_mw;
  civ_float_t         grid_reliability;     /* 0.0-1.0 */
  civ_float_t         energy_price;         /* per MWh */
  civ_float_t         energy_independence;  /* 0.0-1.0, fraction self-supplied */
  civ_float_t         renewable_fraction;
  bool                blackout;
} civ_energy_system_t;

civ_energy_system_t *civ_energy_create(void);
void civ_energy_destroy(civ_energy_system_t *e);
void civ_energy_update(civ_energy_system_t *e, civ_float_t time_delta,
                       civ_float_t population, civ_float_t industrial_output,
                       civ_float_t tech_level, civ_float_t fuel_availability);

civ_power_plant_t *civ_energy_add_plant(civ_energy_system_t *e, civ_energy_source_t source,
                                        civ_float_t capacity, civ_float_t efficiency);
civ_float_t civ_energy_surplus(const civ_energy_system_t *e);
civ_float_t civ_energy_cost_burden(const civ_energy_system_t *e, civ_float_t gdp);

#endif
