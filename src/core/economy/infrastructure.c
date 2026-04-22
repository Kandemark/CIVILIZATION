#include "core/economy/infrastructure.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_infrastructure_system_t *civ_infrastructure_create(void) {
  civ_infrastructure_system_t *inf = CIV_MALLOC(sizeof(civ_infrastructure_system_t));
  if (!inf) return NULL;
  memset(inf, 0, sizeof(*inf));
  for (int i = 0; i < CIV_INFRA_TYPE_COUNT; i++) {
    inf->networks[i].type = (civ_infrastructure_type_t)i;
    inf->networks[i].coverage = 0.35;
    inf->networks[i].condition = 0.70;
    inf->networks[i].maintenance_cost = 5000.0;
  }
  inf->networks[CIV_INFRA_TELECOM].coverage = 0.25;
  inf->networks[CIV_INFRA_AIRPORT].coverage = 0.10;
  inf->overall_quality = 0.50;
  inf->transport_cost_modifier = 1.0;
  inf->supply_chain_efficiency = 0.55;
  return inf;
}

void civ_infrastructure_destroy(civ_infrastructure_system_t *i) { free(i); }

void civ_infrastructure_update(civ_infrastructure_system_t *inf, civ_float_t time_delta,
                               civ_float_t budget_allocation, civ_float_t population,
                               civ_float_t geography_size) {
  if (!inf) return;
  (void)time_delta;
  (void)geography_size;

  /* Spread budget across networks */
  civ_float_t per_network = budget_allocation / CIV_INFRA_TYPE_COUNT;

  for (int i = 0; i < CIV_INFRA_TYPE_COUNT; i++) {
    civ_infrastructure_network_t *n = &inf->networks[i];

    /* Condition degrades from usage (population pressure) */
    civ_float_t wear = population * 0.00001;
    n->condition -= wear * 0.1;
    if (n->condition < 0.0) n->condition = 0.0;

    /* Maintenance from budget */
    if (per_network > 0) {
      n->maintenance_cost = per_network * 0.3;
      n->condition += per_network * 0.0001;
      if (n->condition > 1.0) n->condition = 1.0;
    }

    /* Coverage expands with investment */
    if (n->construction_progress > 0) {
      n->coverage += n->construction_progress * 0.01;
      n->construction_progress *= 0.9; /* decay */
      if (n->coverage > 1.0) n->coverage = 1.0;
    }
  }

  /* Composite quality: avg of coverage * condition */
  inf->overall_quality = 0.0;
  for (int i = 0; i < CIV_INFRA_TYPE_COUNT; i++) {
    inf->overall_quality += inf->networks[i].coverage * inf->networks[i].condition;
  }
  inf->overall_quality /= CIV_INFRA_TYPE_COUNT;

  /* Transport cost inversely proportional to quality */
  inf->transport_cost_modifier = 1.0 + (1.0 - inf->overall_quality) * 2.0;

  /* Supply chain efficiency from transport + telecom */
  inf->supply_chain_efficiency = (inf->networks[CIV_INFRA_ROAD].condition * 0.4
                                  + inf->networks[CIV_INFRA_RAIL].condition * 0.2
                                  + inf->networks[CIV_INFRA_PORT].condition * 0.2
                                  + inf->networks[CIV_INFRA_TELECOM].condition * 0.2);

  /* Total maintenance burden */
  inf->total_maintenance_burden = 0.0;
  for (int i = 0; i < CIV_INFRA_TYPE_COUNT; i++)
    inf->total_maintenance_burden += inf->networks[i].maintenance_cost;
}

void civ_infrastructure_invest(civ_infrastructure_system_t *i,
                               civ_infrastructure_type_t type, civ_float_t amount) {
  if (!i || type >= CIV_INFRA_TYPE_COUNT || amount <= 0) return;
  i->networks[type].construction_progress += amount * 0.001;
  i->networks[type].condition += amount * 0.0005;
  if (i->networks[type].condition > 1.0) i->networks[type].condition = 1.0;
}

civ_float_t civ_infrastructure_trade_efficiency(const civ_infrastructure_system_t *i) {
  return i ? i->supply_chain_efficiency : 0.5;
}

civ_float_t civ_infrastructure_logistics_cost(const civ_infrastructure_system_t *i, civ_float_t distance) {
  if (!i) return distance * 0.01;
  return distance * 0.01 * i->transport_cost_modifier;
}
