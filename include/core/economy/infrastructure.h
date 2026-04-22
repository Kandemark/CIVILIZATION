/**
 * @file infrastructure.h
 * @brief Infrastructure — roads, ports, supply chains, transport cost
 */
#ifndef CIV_ECONOMY_INFRASTRUCTURE_H
#define CIV_ECONOMY_INFRASTRUCTURE_H

#include "../../common.h"
#include "../../types.h"

typedef enum { CIV_INFRA_ROAD, CIV_INFRA_RAIL, CIV_INFRA_PORT,
               CIV_INFRA_AIRPORT, CIV_INFRA_GRID, CIV_INFRA_TELECOM,
               CIV_INFRA_WATER, CIV_INFRA_TYPE_COUNT } civ_infrastructure_type_t;

typedef struct {
  civ_infrastructure_type_t type;
  civ_float_t               coverage;       /* 0.0-1.0 */
  civ_float_t               condition;      /* 0.0-1.0, degrades over time */
  civ_float_t               maintenance_cost;
  civ_float_t               construction_progress; /* for expanding */
} civ_infrastructure_network_t;

typedef struct {
  civ_infrastructure_network_t networks[CIV_INFRA_TYPE_COUNT];
  civ_float_t                  overall_quality;       /* composite 0.0-1.0 */
  civ_float_t                  transport_cost_modifier;
  civ_float_t                  supply_chain_efficiency;
  civ_float_t                  total_maintenance_burden;
} civ_infrastructure_system_t;

civ_infrastructure_system_t *civ_infrastructure_create(void);
void civ_infrastructure_destroy(civ_infrastructure_system_t *i);
void civ_infrastructure_update(civ_infrastructure_system_t *i, civ_float_t time_delta,
                               civ_float_t budget_allocation, civ_float_t population,
                               civ_float_t geography_size);

void civ_infrastructure_invest(civ_infrastructure_system_t *i,
                               civ_infrastructure_type_t type, civ_float_t amount);
civ_float_t civ_infrastructure_trade_efficiency(const civ_infrastructure_system_t *i);
civ_float_t civ_infrastructure_logistics_cost(const civ_infrastructure_system_t *i, civ_float_t distance);

#endif
